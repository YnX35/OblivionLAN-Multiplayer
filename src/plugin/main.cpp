#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "obse/PluginAPI.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
constexpr std::uint32_t kMaxPayload = 256;
constexpr unsigned short kDefaultPort = 47621;
constexpr LONG kMaxRemotePlayers = 3; // Up to four players including the host.
volatile LONG g_connectedClients = 0;
constexpr char kMagic[4] = {'O', 'L', 'A', 'N'};

std::string g_pluginDirectory;
std::string g_logPath;

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

bool acceptHello(SOCKET socket)
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
    std::vector<std::uint8_t> welcome(session.begin(), session.end());
    return send_frame(socket, kWelcome, welcome);
}

struct PeerContext
{
    SOCKET socket;
};

DWORD WINAPI PeerWorker(void* parameter)
{
    PeerContext* context = static_cast<PeerContext*>(parameter);
    if (context == nullptr)
        return 0;

    const SOCKET peer = context->socket;
    delete context;

    // Gameplay packets are not implemented yet. Wait for a clean peer shutdown.
    char ignored = 0;
    for (;;)
    {
        const int result = recv(peer, &ignored, 1, 0);
        if (result <= 0)
            break;
    }

    shutdown(peer, SD_BOTH);
    closesocket(peer);
    InterlockedDecrement(&g_connectedClients);
    Log("Peer disconnected.");
    return 0;
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
            listen(listener, 1) == SOCKET_ERROR)
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
            if (!acceptHello(peer))
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

            PeerContext* context = new (std::nothrow) PeerContext{peer};
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
        if (!sendHello(peer) || !recv_frame(peer, kWelcome, welcome) || welcome.size() != 16)
        {
            Log("Network error: host handshake failed or protocol version differs.");
            shutdown(peer, SD_BOTH);
            closesocket(peer);
            WSACleanup();
            return 0;
        }
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
