#include "obse/PluginAPI.h"
#include <windows.h>
#include <cstring>

namespace
{
void WriteLoadLog()
{
    HMODULE module = nullptr;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&WriteLoadLog),
            &module))
        return;

    char path[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(module, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return;

    char* separator = std::strrchr(path, '\\');
    if (separator == nullptr)
        return;

    *(separator + 1) = '\0';
    const char* filename = "OblivionLAN.log";
    if (std::strlen(path) + std::strlen(filename) >= MAX_PATH)
        return;
    std::strcat(path, filename);

    HANDLE file = CreateFileA(
        path, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    static const char message[] =
        "OblivionLAN: OBSEPlugin_Load reached successfully.\\r\\n";
    DWORD written = 0;
    WriteFile(file, message, static_cast<DWORD>(sizeof(message) - 1), &written, nullptr);
    CloseHandle(file);
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
    info->version = 1;

    // This first build intentionally targets only the classic 1.2.0.416 runtime.
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

    // Diagnostic only: no commands, hooks, networking, save access, or game-memory writes.
    WriteLoadLog();
    return true;
}

}
