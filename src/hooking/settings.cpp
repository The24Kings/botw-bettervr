#include "cemu_hooks.h"
#include "instance.h"
#include "hooking/entity_debugger.h"

std::mutex g_settingsMutex;
data_VRSettingsIn g_settings = {};

uint64_t CemuHooks::s_memoryBaseAddress = 0;
std::atomic_uint32_t CemuHooks::s_framesSinceLastCameraUpdate = 0;


#include <Tlhelp32.h>
#include <winbase.h>
#include <windows.h>

struct FindCtx {
    std::wstring needle;
    HWND found{ nullptr };
};

static BOOL CALLBACK EnumCb(HWND h, LPARAM lp) {
    FindCtx* ctx = reinterpret_cast<FindCtx*>(lp);
    if (!IsWindowVisible(h)) return TRUE;
    int len = GetWindowTextLengthW(h);
    if (len <= 0) return TRUE;
    std::wstring t(len, L'\0');
    GetWindowTextW(h, t.data(), len + 1);
    if (t.find(ctx->needle) != std::wstring::npos) {
        ctx->found = h;
        return FALSE;
    }
    return TRUE;
}

static HWND FindByTitleSubstring(const std::wstring& needle) {
    FindCtx ctx{ needle, nullptr };
    EnumWindows(EnumCb, reinterpret_cast<LPARAM>(&ctx));
    return ctx.found;
}

static bool SetWindowClientSize(HWND h, int clientW, int clientH, int x, int y) {
    if (!IsWindow(h)) return false;

    RECT rc{ 0, 0, clientW, clientH };
    DWORD style = (DWORD)GetWindowLongPtrW(h, GWL_STYLE);
    DWORD ex = (DWORD)GetWindowLongPtrW(h, GWL_EXSTYLE);
    BOOL hasMenu = GetMenu(h) != nullptr;

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    using PFN_AdjustForDpi = BOOL(WINAPI*)(LPRECT, DWORD, BOOL, DWORD, UINT);
    using PFN_GetDpiForWindow = UINT(WINAPI*)(HWND);
    auto pAdj = (PFN_AdjustForDpi)GetProcAddress(user32, "AdjustWindowRectExForDpi");
    auto pGetDpi = (PFN_GetDpiForWindow)GetProcAddress(user32, "GetDpiForWindow");

    if (pAdj && pGetDpi) {
        UINT dpi = pGetDpi(h);
        if (!pAdj(&rc, style, hasMenu, ex, dpi)) return false;
    }
    else {
        if (!AdjustWindowRectEx(&rc, style, hasMenu, ex)) return false;
    }

    int outerW = rc.right - rc.left;
    int outerH = rc.bottom - rc.top;

    // If minimized or maximized, restore first
    if (IsIconic(h) || IsZoomed(h)) ShowWindow(h, SW_RESTORE);

    return SetWindowPos(h, nullptr, x, y, outerW, outerH, SWP_NOZORDER | SWP_NOACTIVATE) != 0;
}

void CemuHooks::hook_UpdateSettings(PPCInterpreter_t* hCPU) {
    // Log::print("Updated settings!");
    hCPU->instructionPointer = hCPU->sprNew.LR;

    uint32_t ppc_settingsOffset = hCPU->gpr[5];
    uint32_t ppc_tableOfCutsceneEventSettings = hCPU->gpr[6];
    data_VRSettingsIn settings = {};

    if (auto& debugger = VRManager::instance().Hooks->m_entityDebugger) {
        debugger->UpdateEntityMemory();
    }
#ifndef NDEBUG
    static bool debug_restoredWindowPositions = false;
    if (!debug_restoredWindowPositions) {
        if (HWND h = FindByTitleSubstring(L"Meta XR Simulator")) {
            SetWindowClientSize(h, 960 + 400, 860, 2050, 2);
            //SetWindowClientSize(h, (960 + 400) / 1.5f, (860.0f) / 1.5f, 2050 + 560, 2);
        }

        if (HWND h2 = FindByTitleSubstring(L"BetterVR Debugging Console")) {
            SetWindowClientSize(h2, 700, 450, 2700, 900);
            //SetWindowClientSize(h2, 600, 450/1.3f, 2700, 630);
        }

        // find HWND that starts with Cemu in its title
        struct EnumWindowsData {
            DWORD cemuPid;
            HWND outHwnd;
        } enumData = { GetCurrentProcessId(), NULL };

        EnumWindows([](HWND iteratedHwnd, LPARAM data) -> BOOL {
            EnumWindowsData* enumData = (EnumWindowsData*)data;
            DWORD currPid;
            GetWindowThreadProcessId(iteratedHwnd, &currPid);
            if (currPid == enumData->cemuPid) {
                constexpr size_t bufSize = 256;
                wchar_t buf[bufSize];
                GetWindowTextW(iteratedHwnd, buf, bufSize);
                if (wcsstr(buf, L"Cemu") != nullptr) {
                    enumData->outHwnd = iteratedHwnd;
                    return FALSE;
                }
            }
            return TRUE;
        },
                    (LPARAM)&enumData);
        //SetWindowClientSize(enumData.outHwnd, 1280 / 1.75, 720 / 1.75, 1950-175, 910);
        SetWindowClientSize(enumData.outHwnd, 1280 / 1.75, 720 / 1.75, 1950, 910);

        debug_restoredWindowPositions = true;
    }
#endif

    readMemory(ppc_settingsOffset, &settings);

    std::lock_guard lock(g_settingsMutex);
    g_settings = settings;
    ++s_framesSinceLastCameraUpdate;

    static bool logSettings = true;
    if (logSettings) {
        Log::print<INFO>("VR Settings:\n{}", g_settings.ToString());
        logSettings = false;
    }

    initCutsceneDefaultSettings(ppc_tableOfCutsceneEventSettings);
}

data_VRSettingsIn CemuHooks::GetSettings() {
    std::lock_guard lock(g_settingsMutex);
    return g_settings;
}


void CemuHooks::hook_OSReportToConsole(PPCInterpreter_t* hCPU) {
    hCPU->instructionPointer = hCPU->sprNew.LR;

    uint32_t strPtr = hCPU->gpr[3];
    if (strPtr == 0) {
        return;
    }
    char* str = (char*)(s_memoryBaseAddress + strPtr);
    if (str == nullptr) {
        return;
    }
    if (str[0] != '\0') {
        Log::print<PPC>(str);
    }
}

constexpr uint32_t playerVtable = 0x101E5FFC;
void CemuHooks::hook_RouteActorJob(PPCInterpreter_t* hCPU) {
    hCPU->instructionPointer = hCPU->sprNew.LR;

    uint32_t actorPtr = hCPU->gpr[3];
    uint32_t jobName = hCPU->gpr[4];
    uint32_t side = hCPU->gpr[5]; // 0 = left, 1 = right

    std::string jobNameStr = std::string((char*)(s_memoryBaseAddress + jobName));

    ActorWiiU actor;
    readMemory(actorPtr, &actor);
    std::string actorName = actor.name.getLE();

#define SKIP_ON_LEFT_SIDE if (side == 0) { hCPU->gpr[3] = 1; }
#define SKIP_ON_RIGHT_SIDE if (side == 1) { hCPU->gpr[3] = 1; }
#define USE_ALTERED_PATH_ON_LEFT_SIDE if (side == 0) { hCPU->gpr[3] = 2; }
#define USE_ALTERED_PATH_ON_RIGHT_SIDE if (side == 1) { hCPU->gpr[3] = 2; }

    hCPU->gpr[3] = 0;
    if (actorName == "GameROMPlayer") {
        if (jobNameStr == "job0_1") {
            // this only runs the climbing portion of this actor job on the left eye's side
            // so that later jobs on the left side can use the state set by this portion of code
            USE_ALTERED_PATH_ON_LEFT_SIDE
        }
        else if (jobNameStr == "job0_2") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job1_1") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job1_2") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job2_1_ragdoll_related") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job2_2") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job4") {
            SKIP_ON_RIGHT_SIDE
        }
    }
    else {
        if (jobNameStr == "job0_1") {
            SKIP_ON_LEFT_SIDE
        }
        else if (jobNameStr == "job0_2") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job1_1") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job1_2") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job2_1_ragdoll_related") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job2_2") {
            SKIP_ON_RIGHT_SIDE
        }
        else if (jobNameStr == "job4") {
            SKIP_ON_RIGHT_SIDE
        }
    }

    if (hCPU->gpr[3] == 0) {
        //Log::print<INFO>("[{}] Ran {}", actorName, jobNameStr);
    }
    else if (hCPU->gpr[3] == 2) {
        //Log::print<INFO>("[{}] Ran ALTERED VERSION of {}", actorName, jobNameStr);
    }



    // exit r3:
    // 1 = skip job
    // 0 = perform job
    // 2 = altered job
}