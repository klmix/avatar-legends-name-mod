// Proxy steam_api64.dll for Avatar Legends: The Fighting Game.
// Forwards every export to the original DLL (renamed to steam_api64_o.dll),
// except SteamInternal_FindOrCreateUserInterface: when the game asks for
// ISteamFriends ("SteamFriends017"), the interface's vtable is patched so that
// GetPersonaName() and GetFriendPersonaName(<own SteamID>) return the name
// from playername.txt instead of the Steam profile name.

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "exports.h"

typedef int32_t HSteamUser;
typedef void*       (__cdecl *FindOrCreate_t)(HSteamUser, const char*);
typedef const char* (*GetPersonaName_t)(void* self);
typedef const char* (*GetFriendPersonaName_t)(void* self, uint64_t steamId);
typedef void*       (__cdecl *SteamUserAccessor_t)();
typedef uint64_t    (__cdecl *GetSteamID_t)(void* user);

// ISteamFriends017 vtable slots
enum { SLOT_GetPersonaName = 0, SLOT_GetFriendPersonaName = 7 };

// install.bat looks for this string to tell the mod apart from Valve's original DLL.
extern "C" __declspec(dllexport) const char NameModMarker[] = "AVATAR-LEGENDS-NAME-MOD 1.0";

static wchar_t g_dir[MAX_PATH];
static HMODULE g_real;
static FindOrCreate_t g_realFindOrCreate;
static GetPersonaName_t g_origGetPersonaName;
static GetFriendPersonaName_t g_origGetFriendPersonaName;
static char g_name[256];
static uint64_t g_selfId;
static volatile LONG g_patched;

static void Log(const char* fmt, ...)
{
    wchar_t path[MAX_PATH];
    swprintf(path, MAX_PATH, L"%snamemod.log", g_dir);
    FILE* f = _wfopen(path, L"a");
    if (!f) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(f, "%04d-%02d-%02d %02d:%02d:%02d  ", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

// Reads playername.txt (UTF-8, first line). Empty or missing file = mod inactive.
static void LoadName()
{
    wchar_t path[MAX_PATH];
    swprintf(path, MAX_PATH, L"%splayername.txt", g_dir);
    FILE* f = _wfopen(path, L"rb");
    if (!f) return;
    size_t n = fread(g_name, 1, sizeof(g_name) - 1, f);
    fclose(f);
    g_name[n] = 0;
    char* s = g_name;
    if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) s += 3;
    s[strcspn(s, "\r\n")] = 0;
    while (*s == ' ' || *s == '\t') ++s;
    size_t len = strlen(s);
    while (len && (s[len - 1] == ' ' || s[len - 1] == '\t')) s[--len] = 0;
    memmove(g_name, s, len + 1);
}

static uint64_t SelfId()
{
    if (!g_selfId) {
        auto accessor = (SteamUserAccessor_t)GetProcAddress(g_real, "SteamAPI_SteamUser_v023");
        auto getId = (GetSteamID_t)GetProcAddress(g_real, "SteamAPI_ISteamUser_GetSteamID");
        void* user = accessor ? accessor() : nullptr;
        if (user && getId) g_selfId = getId(user);
    }
    return g_selfId;
}

static const char* Hook_GetPersonaName(void* self)
{
    return g_name;
}

static const char* Hook_GetFriendPersonaName(void* self, uint64_t steamId)
{
    uint64_t me = SelfId();
    if (me && steamId == me) return g_name;
    return g_origGetFriendPersonaName(self, steamId);
}

static void PatchFriends(void* friends)
{
    void** vtbl = *(void***)friends;
    DWORD old;
    if (!VirtualProtect(vtbl, sizeof(void*) * 8, PAGE_READWRITE, &old)) {
        Log("VirtualProtect failed (%lu)", GetLastError());
        return;
    }
    g_origGetPersonaName = (GetPersonaName_t)vtbl[SLOT_GetPersonaName];
    g_origGetFriendPersonaName = (GetFriendPersonaName_t)vtbl[SLOT_GetFriendPersonaName];
    vtbl[SLOT_GetPersonaName] = (void*)&Hook_GetPersonaName;
    vtbl[SLOT_GetFriendPersonaName] = (void*)&Hook_GetFriendPersonaName;
    VirtualProtect(vtbl, sizeof(void*) * 8, old, &old);
    Log("Steam name '%s' replaced with '%s'", g_origGetPersonaName(friends), g_name);
}

extern "C" void* __cdecl Hook_FindOrCreateUserInterface(HSteamUser user, const char* version)
{
    if (!g_realFindOrCreate) {
        g_real = LoadLibraryW(L"steam_api64_o.dll");
        if (g_real) g_realFindOrCreate = (FindOrCreate_t)GetProcAddress(g_real, "SteamInternal_FindOrCreateUserInterface");
        if (!g_realFindOrCreate) {
            Log("steam_api64_o.dll not found");
            return nullptr;
        }
    }
    void* iface = g_realFindOrCreate(user, version);
    if (iface && version && strcmp(version, "SteamFriends017") == 0 && g_name[0]
        && InterlockedCompareExchange(&g_patched, 1, 0) == 0)
        PatchFriends(iface);
    return iface;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);
        DWORD n = GetModuleFileNameW(inst, g_dir, MAX_PATH);
        while (n && g_dir[n - 1] != L'\\') --n;
        g_dir[n] = 0;
        LoadName();
    }
    return TRUE;
}
