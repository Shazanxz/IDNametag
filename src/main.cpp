// IDNameTag - by Shazanxz (https://github.com/Shazanxz/IDNametag)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <cstdio>

static const char kOriginalFormat[] = "%s (%d)";
static const char kDefaultFormat[]  = "%s (ID: %d)";


static char g_format[64] = {};
static HMODULE g_self = nullptr;


static bool IsSafeFormat(const char* fmt) {
    int stage = 0; // 0 = waiting for %s, 1 = waiting for %d, 2 = done
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') continue;
        ++p;
        if (*p == '%') continue;
        if (*p == 's' && stage == 0) { stage = 1; continue; }
        if (*p == 'd' && stage == 1) { stage = 2; continue; }
        return false;
    }
    return stage == 2;
}

static void LoadConfig() {
    char path[MAX_PATH] = {};
    GetModuleFileNameA(g_self, path, MAX_PATH);
    char* dot = strrchr(path, '.');
    if (dot) strcpy_s(dot, path + MAX_PATH - dot, ".ini");

    char value[sizeof(g_format)] = {};
    GetPrivateProfileStringA("NameTag", "Format", kDefaultFormat, value, sizeof(value), path);

    // Length limit: SA-MP's nametag buffer is small (names are up to 24 chars).
    if (strlen(value) > 24 || !IsSafeFormat(value))
        strcpy_s(g_format, kDefaultFormat);
    else
        strcpy_s(g_format, value);

    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
        WritePrivateProfileStringA("NameTag", "Format", kDefaultFormat, path);
}

static IMAGE_NT_HEADERS* NtHeaders(HMODULE mod) {
    auto base = reinterpret_cast<BYTE*>(mod);
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    return reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
}

static const BYTE* FindFormatString(HMODULE mod) {
    auto base = reinterpret_cast<const BYTE*>(mod);
    DWORD size = NtHeaders(mod)->OptionalHeader.SizeOfImage;
    const size_t len = sizeof(kOriginalFormat); // includes the '\0'

    auto sec = IMAGE_FIRST_SECTION(NtHeaders(mod));
    for (WORD i = 0; i < NtHeaders(mod)->FileHeader.NumberOfSections; ++i, ++sec) {
        if (sec->Characteristics & IMAGE_SCN_MEM_EXECUTE) continue;
        const BYTE* start = base + sec->VirtualAddress;
        DWORD secSize = sec->Misc.VirtualSize;
        if (sec->VirtualAddress + secSize > size || secSize < len + 1) continue;
        for (DWORD off = 1; off + len <= secSize; ++off) {
            if (start[off - 1] == 0 && memcmp(start + off, kOriginalFormat, len) == 0)
                return start + off;
        }
    }
    return nullptr;
}

static int PatchReferences(HMODULE mod, const BYTE* target) {
    auto base = reinterpret_cast<BYTE*>(mod);
    DWORD targetAddr = reinterpret_cast<DWORD>(target);
    DWORD newAddr = reinterpret_cast<DWORD>(g_format);
    int patched = 0;

    auto sec = IMAGE_FIRST_SECTION(NtHeaders(mod));
    for (WORD i = 0; i < NtHeaders(mod)->FileHeader.NumberOfSections; ++i, ++sec) {
        if (!(sec->Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
        BYTE* start = base + sec->VirtualAddress;
        DWORD secSize = sec->Misc.VirtualSize;
        for (DWORD off = 0; off + 5 <= secSize; ++off) {
            if (start[off] != 0x68 || *reinterpret_cast<DWORD*>(start + off + 1) != targetAddr)
                continue;
            DWORD old;
            if (VirtualProtect(start + off + 1, 4, PAGE_EXECUTE_READWRITE, &old)) {
                *reinterpret_cast<DWORD*>(start + off + 1) = newAddr;
                VirtualProtect(start + off + 1, 4, old, &old);
                ++patched;
            }
            off += 4;
        }
    }
    return patched;
}

static DWORD WINAPI MainThread(LPVOID) {
    LoadConfig();

    HMODULE samp = nullptr;
    for (int i = 0; i < 600 && !samp; ++i) {
        samp = GetModuleHandleA("samp.dll");
        if (!samp) Sleep(100);
    }
    if (!samp) return 0;

    const BYTE* fmt = FindFormatString(samp);
    int patched = fmt ? PatchReferences(samp, fmt) : 0;

    char msg[128];
    sprintf_s(msg, "[IDNameTag] format string %s, %d reference(s) patched\n",
              fmt ? "found" : "NOT found", patched);
    OutputDebugStringA(msg);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = module;
        DisableThreadLibraryCalls(module);
        HANDLE t = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }
    return TRUE;
}
