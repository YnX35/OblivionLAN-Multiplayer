#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "obse/PluginAPI.h"
#include "obse/GameObjects.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <new>
#include <string>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

namespace
{
constexpr std::uint16_t kProtocolVersion = 1;
constexpr std::uint16_t kHello = 1;
constexpr std::uint16_t kWelcome = 2;
constexpr std::uint16_t kPlayerSnapshot = 3;
constexpr std::uint32_t kMaxPayload = 256;
constexpr unsigned short kDefaultPort = 47621;
constexpr LONG kMaxRemotePlayers = 3; // Up to four players including the host.
volatile LONG g_connectedClients = 0;
volatile LONG g_nextPlayerId = 1;
volatile LONG g_localNetworkPlayerId = 0;
CRITICAL_SECTION g_snapshotLock{};
bool g_snapshotLockReady = false;
struct PlayerSnapshot
{
    std::uint16_t playerId = 0;
    std::uint32_t sequence = 0;
    std::uint32_t cellId = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};
PlayerSnapshot g_latestSnapshot{};
bool g_hasSnapshot = false;
DWORD g_lastSnapshotSequence = 0;
constexpr char kMagic[4] = {'O', 'L', 'A', 'N'};

std::string g_pluginDirectory;
std::string g_logPath;
OBSETasks2Interface* g_tasks2 = nullptr;
Task<bool>* g_gameThreadTask = nullptr;
DWORD g_lastPlayerSnapshotLog = 0;

void Log(const char* message)
{
    if (g_logPath.empty())
        return;

    HANDLE file = CreateFileA(g_logPath.c_str(), FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    SYSTEMTIME now{};
    GetLocalTime(&now);
    char line[512] = {};
    const int length = std::snprintf(line, sizeof(line),
        "[%02u:%02u:%02u] %s\r\n",
        static_cast<unsigned>(now.wHour),
        static_cast<unsigned>(now.wMinute),
        static_cast<unsigned>(now.wSecond), message);
    if (length > 0 && static_cast<std::size_t>(length) < sizeof(line))
    {
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(length), &written, nullptr);
    }
    CloseHandle(file);
}

bool GameThreadPump()
{
    // xOBSE invokes this removable task from its main game loop.
    // Returning false keeps it scheduled for the next loop iteration.
    PlayerCharacter* player = (g_thePlayer != nullptr) ? *g_thePlayer : nullptr;
    if (player == nullptr || player->parentCell == nullptr)
        return false;

    if (g_snapshotLockReady)
    {
        PlayerSnapshot snapshot{};
        snapshot.playerId = static_cast<std::uint16_t>(
            InterlockedCompareExchange(&g_localNetworkPlayerId, 0, 0));
        snapshot.sequence = ++g_lastSnapshotSequence;
        snapshot.cellId = player->parentCell->refID;
        snapshot.x = player->posX;
        snapshot.y = player->posY;
        snapshot.z = player->posZ;
        EnterCriticalSection(&g_snapshotLock);
        g_latestSnapshot = snapshot;
        g_hasSnapshot = true;
        LeaveCriticalSection(&g_snapshotLock);
    }

    const DWORD now = GetTickCount();
    if (now - g_lastPlayerSnapshotLog >= 5000)
    {
        g_lastPlayerSnapshotLog = now;
        char message[256] = {};
        const unsigned long baseFormId = player->baseForm != nullptr
            ? static_cast<unsigned long>(player->baseForm->refID) : 0UL;
        std::snprintf(message, sizeof(message),
            "Game-thread snapshot available: cell=%08lX base=%08lX pos=(%.1f, %.1f, %.1f).",
            static_cast<unsigned long>(player->parentCell->refID), baseFormId,
            player->posX, player->posY, player->posZ);
        Log(message);
    }
    return false;
}

bool GetModulePaths()
{
    HMODULE module = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&GetModulePaths), &module))
        return false;

    char path[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(module, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return false;

    char* separator = std::strrchr(path, '\\');
    if (separator == nullptr)
        return false;
    *(separator + 1) = '\0';

    g_pluginDirectory = path;
    g_logPath = g_pluginDirectory + "OblivionLAN.log";
    return true;
}

void put_u16(std::vector<std::uint8_t>& out, std::uint16_t value)
{
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
    out.push_back(static_cast<std::uint8_t>(value & 0xff));
}

void put_u32(std::vector<std::uint8_t>& out, std::uint32_t value)
{
    out.push_back(static_cast<std::uint8_t>((value >> 24) & 0xff));
    out.push_back(static_cast<std::uint8_t>((value >> 16) & 0xff));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
    out.push_back(static_cast<std::uint8_t>(value & 0xff));
}

std::uint16_t read_u16(const std::uint8_t* p)
{
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(p[0]) << 8) | p[1]);
}

std::uint32_t read_u32(const std::uint8_t* p)
{
    return (static_cast<std::uint32_t>(p[0]) << 24) |
           (static_cast<std::uint32_t>(p[1]) << 16) |
           (static_cast<std::uint32_t>(p[2]) << 8) |
           static_cast<std::uint32_t>(p[3]);
}

bool send_all(SOCKET socket, const std::uint8_t* data, std::size_t size)
{
    while (size > 0)
    {
        const int chunk = static_cast<int>(
            size > 0x7fffffffU ? 0x7fffffffU : size);
        const int sent = send(socket, reinterpret_cast<const char*>(data), chunk, 0);
        if (sent == SOCKET_ERROR || sent == 0)
            return false;
        data += sent;
        size -= static_cast<std::size_t>(sent);
    }
    return true;
}

bool recv_all(SOCKET socket, std::uint8_t* data, std::size_t size)
{
    while (size > 0)
    {
        const int chunk = static_cast<int>(
            size > 0x7fffffffU ? 0x7fffffffU : size);
        const int received = recv(socket, reinterpret_cast<char*>(data), chunk, 0);
        if (received == SOCKET_ERROR || received == 0)
            return false;
        data += received;
        size -= static_cast<std::size_t>(received);
    }
    return true;
}

bool send_frame(SOCKET socket, std::uint16_t type,
                const std::vector<std::uint8_t>& payload)
{
    if (payload.size() > kMaxPayload)
        return false;

    std::vector<std::uint8_t> frame;
    frame.reserve(12 + payload.size());
    frame.insert(frame.end(), kMagic, kMagic + 4);
    put_u16(frame, kProtocolVersion);
    put_u16(frame, type);
    put_u32(frame, static_cast<std::uint32_t>(payload.size()));
    frame.insert(frame.end(), payload.begin(), payload.end());
    return send_all(socket, frame.data(), frame.size());
}

bool recv_frame(SOCKET socket, std::uint16_t expectedType,
                std::vector<std::uint8_t>& payload)
{
    std::array<std::uint8_t, 12> header{};
    if (!recv_all(socket, header.data(), header.size()))
        return false;
    if (std::memcmp(header.data(), kMagic, 4) != 0 ||
        read_u16(header.data() + 4) != kProtocolVersion ||
        read_u16(header.data() + 6) != expectedType)
        return false;

    const std::uint32_t length = read_u32(header.data() + 8);
    if (length > kMaxPayload)
        return false;
    payload.resize(length);
    return length == 0 || recv_all(socket, payload.data(), payload.size());
}

bool sendHello(SOCKET socket)
{
    const char name[] = "Oblivion";
    std::vector<std::uint8_t> payload;
    payload.push_back(static_cast<std::uint8_t>(sizeof(name) - 1));
    payload.insert(payload.end(), name, name + sizeof(name) - 1);
    return send_frame(socket, kHello, payload);
}

bool acceptHello(SOCKET socket, std::uint16_t& assignedPlayerId)
{
    std::vector<std::uint8_t> payload;
    if (!recv_frame(socket, kHello, payload) ||
        payload.empty() || payload[0] == 0 || payload[0] > 32 ||
        payload.size() != static_cast<std::size_t>(payload[0]) + 1)
        return false;

    for (std::size_t i = 1; i < payload.size(); ++i)
        if (payload[i] < 0x20 || payload[i] == 0x7f)
            return false;

    std::array<std::uint8_t, 16> session{};
    const DWORD seed = GetTickCount() ^ GetCurrentProcessId();
    for (std::size_t i = 0; i < session.size(); ++i)
        session[i] = static_cast<std::uint8_t>((seed >> ((i % 4) * 8)) ^ (i * 37));
    assignedPlayerId = static_cast<std::uint16_t>(
        InterlockedIncrement(&g_nextPlayerId));
    if (assignedPlayerId == 0)
        return false;
    std::vector<std::uint8_t> welcome(session.begin(), session.end());
    put_u16(welcome, assignedPlayerId);
    return send_frame(socket, kWelcome, welcome);
}

struct PeerContext
{
    SOCKET socket;
    std::uint16_t playerId;
};

DWORD WINAPI PeerWorker(void* parameter)
{
    PeerContext* context = static_cast<PeerContext*>(parameter);
    if (context == nullptr)
        return 0;

    const SOCKET peer = context->socket;
    delete context;

    // Receive client position snapshots on this worker only. Engine calls stay
    // on the xOBSE game thread; this worker only validates wire data.
    for (;;)
    {
        std::vector<std::uint8_t> payload;
        if (!recv_frame(peer, kPlayerSnapshot, payload) || payload.size() != 22)
            break;

        const std::uint16_t playerId = read_u16(payload.data());
        const std::uint32_t sequence = read_u32(payload.data() + 2);
        const std::uint32_t cellId = read_u32(payload.data() + 6);
        std::uint32_t xb = read_u32(payload.data() + 10);
        std::uint32_t yb = read_u32(payload.data() + 14);
        std::uint32_t zb = read_u32(payload.data() + 18);
        float x = 0.0f, y = 0.0f, z = 0.0f;
        std::memcpy(&x, &xb, sizeof(x));
        std::memcpy(&y, &yb, sizeof(y));
        std::memcpy(&z, &zb, sizeof(z));
        if (playerId != context->playerId || sequence == 0 ||
            !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
            std::fabs(x) > 10000000.0f || std::fabs(y) > 10000000.0f ||
            std::fabs(z) > 10000000.0f)
        {
            Log("Rejected invalid player snapshot.");
            break;
        }
        char message[160] = {};
        std::snprintf(message, sizeof(message),
            "Received player snapshot: player=%u seq=%lu cell=%08lX pos=(%.1f, %.1f, %.1f).",
            static_cast<unsigned>(playerId), static_cast<unsigned long>(sequence),
            static_cast<unsigned long>(cellId), x, y, z);
        Log(message);
    }

    shutdown(peer, SD_BOTH);
    closesocket(peer);
    InterlockedDecrement(&g_connectedClients);
    Log("Peer disconnected.");
    return 0;
}

void KeepConnectionUntilClosed(SOCKET peer)
{
    // Client-side transport loop. It sends the newest game-thread snapshot;
    // stale positions are coalesced instead of building an unbounded backlog.
    DWORD lastSentAt = 0;
    std::uint32_t lastSentSequence = 0;
    for (;;)
    {
        const DWORD now = GetTickCount();
        if (now - lastSentAt >= 100 && g_snapshotLockReady)
        {
            PlayerSnapshot snapshot{};
            bool available = false;
            EnterCriticalSection(&g_snapshotLock);
            if (g_hasSnapshot)
            {
                snapshot = g_latestSnapshot;
                available = true;
            }
            LeaveCriticalSection(&g_snapshotLock);

            if (available && snapshot.playerId != 0 &&
                snapshot.sequence != lastSentSequence)
            {
                std::vector<std::uint8_t> payload;
                payload.reserve(22);
                put_u16(payload, snapshot.playerId);
                put_u32(payload, snapshot.sequence);
                put_u32(payload, snapshot.cellId);
                std::uint32_t bits = 0;
                std::memcpy(&bits, &snapshot.x, sizeof(bits)); put_u32(payload, bits);
                std::memcpy(&bits, &snapshot.y, sizeof(bits)); put_u32(payload, bits);
                std::memcpy(&bits, &snapshot.z, sizeof(bits)); put_u32(payload, bits);
                if (!send_frame(peer, kPlayerSnapshot, payload))
                    break;
                lastSentSequence = snapshot.sequence;
                lastSentAt = now;
            }
        }

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(peer, &readSet);
        timeval wait{0, 50000};
        const int ready = select(0, &readSet, nullptr, nullptr, &wait);
        if (ready == SOCKET_ERROR)
            break;
        if (ready > 0)
        {
            char probe = 0;
            const int received = recv(peer, &probe, 1, MSG_PEEK);
            if (received <= 0)
                break;
        }
    }
    shutdown(peer, SD_BOTH);
    closesocket(peer);
    Log("Host connection closed.");
}

DWORD WINAPI NetworkWorker(void*)
{
    char mode[16] = {};
    char host[64] = {};
    char portText[16] = {};
    const std::string iniPath = g_pluginDirectory + "OblivionLAN.ini";

    GetPrivateProfileStringA("Network", "Mode", "disabled",
        mode, static_cast<DWORD>(sizeof(mode)), iniPath.c_str());
    GetPrivateProfileStringA("Network", "Host", "127.0.0.1",
        host, static_cast<DWORD>(sizeof(host)), iniPath.c_str());
    GetPrivateProfileStringA("Network", "Port", "47621",
        portText, static_cast<DWORD>(sizeof(portText)), iniPath.c_str());

    char* end = nullptr;
    const unsigned long parsedPort = std::strtoul(portText, &end, 10);
    if (end == portText || *end != '\0' || parsedPort == 0 || parsedPort > 65535)
    {
        Log("Network disabled: invalid Port in OblivionLAN.ini.");
        return 0;
    }
    const unsigned short port = static_cast<unsigned short>(parsedPort);

    if (_stricmp(mode, "disabled") == 0)
    {
        Log("Plugin loaded. Network mode is disabled in OblivionLAN.ini.");
        return 0;
    }

    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        Log("Network error: WSAStartup failed.");
        return 0;
    }

    if (_stricmp(mode, "host") == 0)
    {
        SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener == INVALID_SOCKET)
        {
            Log("Network error: could not create host socket.");
            WSACleanup();
            return 0;
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);

        if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
            listen(listener, kMaxRemotePlayers) == SOCKET_ERROR)
        {
            Log("Network error: host bind/listen failed. Check port and firewall.");
            closesocket(listener);
            WSACleanup();
            return 0;
        }

        char message[128] = {};
        std::snprintf(message, sizeof(message),
            "Network host listening on TCP port %u.", static_cast<unsigned>(port));
        Log(message);

        Log("Host is ready for up to three remote players.");
        for (;;)
        {
            SOCKET peer = accept(listener, nullptr, nullptr);
            if (peer == INVALID_SOCKET)
            {
                Log("Network warning: accept failed; host listener remains active.");
                Sleep(250);
                continue;
            }

            if (InterlockedCompareExchange(&g_connectedClients, 0, 0) >= kMaxRemotePlayers)
            {
                Log("Connection refused: host already has three remote players.");
                shutdown(peer, SD_BOTH);
                closesocket(peer);
                continue;
            }

            DWORD handshakeTimeoutMs = 5000;
            setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
                reinterpret_cast<const char*>(&handshakeTimeoutMs), sizeof(handshakeTimeoutMs));
            setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO,
                reinterpret_cast<const char*>(&handshakeTimeoutMs), sizeof(handshakeTimeoutMs));
            std::uint16_t assignedPlayerId = 0;
            if (!acceptHello(peer, assignedPlayerId))
            {
                Log("Network warning: rejected client with invalid or timed-out handshake.");
                shutdown(peer, SD_BOTH);
                closesocket(peer);
                continue;
            }

            handshakeTimeoutMs = 0;
            setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
                reinterpret_cast<const char*>(&handshakeTimeoutMs), sizeof(handshakeTimeoutMs));
            setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO,
                reinterpret_cast<const char*>(&handshakeTimeoutMs), sizeof(handshakeTimeoutMs));

            PeerContext* context = new (std::nothrow) PeerContext{peer, assignedPlayerId};
            if (context == nullptr)
            {
                Log("Network error: not enough memory for peer context.");
                shutdown(peer, SD_BOTH);
                closesocket(peer);
                continue;
            }

            InterlockedIncrement(&g_connectedClients);
            HANDLE peerThread = CreateThread(nullptr, 0, PeerWorker, context, 0, nullptr);
            if (peerThread == nullptr)
            {
                InterlockedDecrement(&g_connectedClients);
                delete context;
                shutdown(peer, SD_BOTH);
                closesocket(peer);
                Log("Network error: could not start peer thread.");
                continue;
            }
            CloseHandle(peerThread);
            Log("LAN handshake successful: client connected to Oblivion host.");
        }
    }
    else if (_stricmp(mode, "client") == 0)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        if (InetPtonA(AF_INET, host, &address.sin_addr) != 1)
        {
            Log("Network error: Host must be an IPv4 address in OblivionLAN.ini.");
            WSACleanup();
            return 0;
        }

        SOCKET peer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (peer == INVALID_SOCKET)
        {
            Log("Network error: could not create client socket.");
            WSACleanup();
            return 0;
        }

        // A short timeout prevents a bad/missing host from hanging game startup.
        DWORD timeoutMs = 5000;
        setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
        setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

        if (connect(peer, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
        {
            Log("Network error: could not connect to host. Check Host, port and firewall.");
            closesocket(peer);
            WSACleanup();
            return 0;
        }

        std::vector<std::uint8_t> welcome;
        if (!sendHello(peer) || !recv_frame(peer, kWelcome, welcome) || welcome.size() != 18)
        {
            Log("Network error: host handshake failed or protocol version differs.");
            shutdown(peer, SD_BOTH);
            closesocket(peer);
            WSACleanup();
            return 0;
        }
        const std::uint16_t assignedPlayerId = read_u16(welcome.data() + 16);
        if (assignedPlayerId == 0)
        {
            Log("Network error: host assigned an invalid player ID.");
            shutdown(peer, SD_BOTH);
            closesocket(peer);
            WSACleanup();
            return 0;
        }
        InterlockedExchange(&g_localNetworkPlayerId, assignedPlayerId);
        timeoutMs = 0;
        setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
        setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));
        Log("LAN handshake successful: Oblivion client connected to host.");
        KeepConnectionUntilClosed(peer);
    }
    else
    {
        Log("Network disabled: Mode must be disabled, host, or client.");
    }

    WSACleanup();
    return 0;
}

void WriteLoadLog()
{
    if (g_logPath.empty())
        return;
    Log("OBSEPlugin_Load reached successfully.");
}
}

extern "C"
{
bool OBSEPlugin_Query(const OBSEInterface* obse, PluginInfo* info)
{
    if (obse == nullptr || info == nullptr)
        return false;

    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "OblivionLAN";
    info->version = 2;

    if (obse->isEditor)
        return false;
    if (obse->obseVersion < OBSE_VERSION_INTEGER)
        return false;
    if (obse->oblivionVersion != OBLIVION_VERSION)
        return false;

    return true;
}

bool OBSEPlugin_Load(const OBSEInterface* obse)
{
    if (obse == nullptr || obse->isEditor)
        return false;

    if (!GetModulePaths())
        return false;

    WriteLoadLog();
    InitializeCriticalSection(&g_snapshotLock);
    g_snapshotLockReady = true;

    // Use the SDK's supported main-loop task interface. Game state must only
    // be sampled/applied here, never from the socket worker threads.
    g_tasks2 = static_cast<OBSETasks2Interface*>(
        obse->QueryInterface(kInterface_Tasks2));
    if (g_tasks2 != nullptr &&
        g_tasks2->version >= OBSETasks2Interface::kVersion &&
        g_tasks2->EnqueueTaskRemovable != nullptr)
    {
        g_gameThreadTask = g_tasks2->EnqueueTaskRemovable(GameThreadPump);
        Log(g_gameThreadTask != nullptr
            ? "xOBSE main-loop gameplay pump registered."
            : "Could not register xOBSE main-loop gameplay pump.");
    }
    else
    {
        Log("Gameplay pump unavailable: xOBSE Tasks2 interface is missing or too old.");
    }

    HANDLE thread = CreateThread(nullptr, 0, NetworkWorker, nullptr, 0, nullptr);
    if (thread == nullptr)
    {
        Log("Network error: could not start worker thread.");
        return false;
    }
    CloseHandle(thread);
    return true;
}
}
