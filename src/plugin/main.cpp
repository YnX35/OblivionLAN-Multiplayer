#include "obse/PluginAPI.h"

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
    // Stage 1: prove that xOBSE can load the DLL.
    // No commands, hooks, networking, save access, or game-memory writes yet.
    return obse != nullptr && !obse->isEditor;
}

}
