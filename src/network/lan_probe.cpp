#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace
{
constexpr std::uint16_t kProtocolVersion = 1;
constexpr std::uint16_t kHello = 1;
constexpr std::uint16_t kWelcome = 2;
constexpr std::uint32_t kMaxPayload = 256;
constexpr unsigned short kDefaultPort = 47621;
constexpr char kMagic[4] = {'O', 'L', 'A', 'N'};

class Winsock
{
public:
    Winsock()
    {
        WSADATA data{};
        ok_ = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }
    ~Winsock() { if (ok_) WSACleanup(); }
    bool ok() const { return ok_; }
private:
    bool ok_ = false;
};

class Socket
{
public:
    explicit Socket(SOCKET value = INVALID_SOCKET) : value_(value) {}
    ~Socket() { close(); }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : value_(other.value_) { other.value_ = INVALID_SOCKET; }
    Socket& operator=(Socket&& other) noexcept
    {
        if (this != &other) { close(); value_ = other.value_; other.value_ = INVALID_SOCKET; }
        return *this;
    }
    SOCKET get() const { return value_; }
    bool valid() const { return value_ != INVALID_SOCKET; }
    void close()
    {
        if (valid()) { shutdown(value_, SD_BOTH); closesocket(value_); value_ = INVALID_SOCKET; }
    }
private:
    SOCKET value_;
};

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
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(p[0]) << 8) | p[1]);
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
        const int chunk = static_cast<int>((size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            ? std::numeric_limits<int>::max() : size);
        const int sent = send(socket, reinterpret_cast<const char*>(data), chunk, 0);
        if (sent == SOCKET_ERROR || sent == 0) return false;
        data += sent;
        size -= static_cast<std::size_t>(sent);
    }
    return true;
}

bool recv_all(SOCKET socket, std::uint8_t* data, std::size_t size)
{
    while (size > 0)
    {
        const int chunk = static_cast<int>((size > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            ? std::numeric_limits<int>::max() : size);
        const int received = recv(socket, reinterpret_cast<char*>(data), chunk, 0);
        if (received == SOCKET_ERROR || received == 0) return false;
        data += received;
        size -= static_cast<std::size_t>(received);
    }
    return true;
}

bool send_frame(SOCKET socket, std::uint16_t type, const std::vector<std::uint8_t>& payload)
{
    if (payload.size() > kMaxPayload) return false;
    std::vector<std::uint8_t> frame;
    frame.reserve(12 + payload.size());
    frame.insert(frame.end(), kMagic, kMagic + 4);
    put_u16(frame, kProtocolVersion);
    put_u16(frame, type);
    put_u32(frame, static_cast<std::uint32_t>(payload.size()));
    frame.insert(frame.end(), payload.begin(), payload.end());
    return send_all(socket, frame.data(), frame.size());
}

bool recv_frame(SOCKET socket, std::uint16_t expected_type, std::vector<std::uint8_t>& payload)
{
    std::array<std::uint8_t, 12> header{};
    if (!recv_all(socket, header.data(), header.size())) return false;
    if (std::memcmp(header.data(), kMagic, 4) != 0) return false;
    if (read_u16(header.data() + 4) != kProtocolVersion) return false;
    if (read_u16(header.data() + 6) != expected_type) return false;
    const std::uint32_t length = read_u32(header.data() + 8);
    if (length > kMaxPayload) return false;
    payload.resize(length);
    return length == 0 || recv_all(socket, payload.data(), payload.size());
}

std::string trim_name(std::string name)
{
    if (name.size() > 32) name.resize(32);
    for (char& ch : name)
    {
        const unsigned char c = static_cast<unsigned char>(ch);
        if (c < 0x20 || c == 0x7f) ch = '_';
    }
    if (name.empty()) name = "Player";
    return name;
}

bool make_hello(const std::string& name, std::vector<std::uint8_t>& payload)
{
    const std::string safe_name = trim_name(name);
    payload.clear();
    payload.push_back(static_cast<std::uint8_t>(safe_name.size()));
    payload.insert(payload.end(), safe_name.begin(), safe_name.end());
    return true;
}

bool parse_hello(const std::vector<std::uint8_t>& payload, std::string& name)
{
    if (payload.empty()) return false;
    const std::size_t length = payload[0];
    if (length == 0 || length > 32 || payload.size() != length + 1) return false;
    name.assign(payload.begin() + 1, payload.end());
    for (unsigned char ch : name)
        if (ch < 0x20 || ch == 0x7f) return false;
    return true;
}

std::array<std::uint8_t, 16> make_session_id()
{
    std::array<std::uint8_t, 16> id{};
    std::random_device random;
    for (auto& byte : id) byte = static_cast<std::uint8_t>(random());
    return id;
}

int run_host(unsigned short port)
{
    Socket listener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!listener.valid()) { std::cerr << "socket() failed: " << WSAGetLastError() << "\n"; return 1; }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(listener.get(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed: " << WSAGetLastError() << "\n";
        return 1;
    }
    if (listen(listener.get(), 1) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed: " << WSAGetLastError() << "\n";
        return 1;
    }

    std::cout << "Host ready on TCP port " << port << ". Waiting for one player...\n";
    Socket peer(accept(listener.get(), nullptr, nullptr));
    if (!peer.valid()) { std::cerr << "accept() failed: " << WSAGetLastError() << "\n"; return 1; }

    std::vector<std::uint8_t> payload;
    std::string player_name;
    if (!recv_frame(peer.get(), kHello, payload) || !parse_hello(payload, player_name))
    {
        std::cerr << "Rejected invalid or incompatible handshake.\n";
        return 1;
    }

    const auto session_id = make_session_id();
    std::vector<std::uint8_t> welcome(session_id.begin(), session_id.end());
    if (!send_frame(peer.get(), kWelcome, welcome))
    {
        std::cerr << "Could not send welcome packet.\n";
        return 1;
    }

    std::cout << "Player connected: " << player_name << "\n";
    std::cout << "Handshake successful. Press Enter to close the test session.\n";
    std::string ignored;
    std::getline(std::cin, ignored);
    return 0;
}

int run_client(const std::string& ip, unsigned short port, const std::string& name)
{
    Socket peer(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!peer.valid()) { std::cerr << "socket() failed: " << WSAGetLastError() << "\n"; return 1; }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (InetPtonA(AF_INET, ip.c_str(), &address.sin_addr) != 1)
    {
        std::cerr << "Invalid IPv4 address. Example: 192.168.1.20\n";
        return 1;
    }
    if (connect(peer.get(), reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        std::cerr << "connect() failed: " << WSAGetLastError() << "\n";
        return 1;
    }

    std::vector<std::uint8_t> hello;
    make_hello(name, hello);
    if (!send_frame(peer.get(), kHello, hello))
    {
        std::cerr << "Could not send handshake.\n";
        return 1;
    }

    std::vector<std::uint8_t> welcome;
    if (!recv_frame(peer.get(), kWelcome, welcome) || welcome.size() != 16)
    {
        std::cerr << "Host rejected the handshake or sent an invalid response.\n";
        return 1;
    }

    std::cout << "Connected to host " << ip << ":" << port << ".\n";
    std::cout << "LAN handshake successful. Press Enter to disconnect.\n";
    std::string ignored;
    std::getline(std::cin, ignored);
    return 0;
}
}

int main(int argc, char** argv)
{
    Winsock winsock;
    if (!winsock.ok()) { std::cerr << "Winsock initialization failed.\n"; return 1; }

    if (argc < 2)
    {
        std::cout << "OblivionLAN network prototype\n"
                  << "  host [port]\n"
                  << "  join <IPv4> [port] [player-name]\n";
        return 0;
    }

    const std::string mode = argv[1];
    if (mode == "host")
    {
        const unsigned long port = argc >= 3 ? std::stoul(argv[2]) : kDefaultPort;
        if (port == 0 || port > 65535) { std::cerr << "Invalid port.\n"; return 1; }
        return run_host(static_cast<unsigned short>(port));
    }
    if (mode == "join")
    {
        if (argc < 3) { std::cerr << "Usage: OblivionLANProbe join <IPv4> [port] [player-name]\n"; return 1; }
        const unsigned long port = argc >= 4 ? std::stoul(argv[3]) : kDefaultPort;
        if (port == 0 || port > 65535) { std::cerr << "Invalid port.\n"; return 1; }
        const std::string name = argc >= 5 ? argv[4] : "Player";
        return run_client(argv[2], static_cast<unsigned short>(port), name);
    }

    std::cerr << "Unknown mode. Use host or join.\n";
    return 1;
}
