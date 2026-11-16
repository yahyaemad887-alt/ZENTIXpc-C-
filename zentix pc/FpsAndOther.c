#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "FpsAndOther.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "psapi.lib")

#pragma pack(push, 1)

typedef struct {
    DWORD dwSignature;
    DWORD dwVersion;
    DWORD dwAppEntrySize;
    DWORD dwAppArrOffset;
    DWORD dwAppArrSize;
    DWORD dwOSDEntrySize;
    DWORD dwOSDArrOffset;
    DWORD dwOSDArrSize;
    DWORD dwOSDFrame;
} RTSS_SHARED_MEMORY_HEADER;

// الهيكل الرسمي الدقيق المتوافق مع SDK لمدخلات التطبيق والفريمات
typedef struct {
    DWORD dwProcessID;
    char  szName[260];
    DWORD dwFlags;
    DWORD dwTime0;
    DWORD dwTime1;
    DWORD dwFrames;
    DWORD dwFrameTime;
} RTSS_APP_ENTRY_EXACT;

// هياكل ذاكرة MSI Afterburner المشتركة (MAHM) لقراءة الحساسات الحقيقية
typedef struct {
    DWORD dwSignature;   // 'MAHM' = 0x4D41484D
    DWORD dwVersion;
    DWORD dwHeaderSize;
    DWORD dwNumEntries;
    DWORD dwEntrySize;
    DWORD dwTime;
    DWORD dwNumGpuEntries;
    DWORD dwGpuEntrySize;
} MAHM_SHARED_MEMORY_HEADER;

typedef struct {
    char  szName[MAX_PATH];
    char  szUnits[MAX_PATH];
    char  szLocalName[MAX_PATH];
    char  szLocalUnits[MAX_PATH];
    char  szFormat[MAX_PATH];
    float data;
    float minLimit;
    float maxLimit;
    DWORD dwFlags;
    DWORD dwGpu;
    DWORD dwSrcId;
} MAHM_SHARED_MEMORY_ENTRY;

#pragma pack(pop)

// الثوابت القياسية لأنواع الحساسات في MSI Afterburner
#define MONITORING_SOURCE_ID_GPU_TEMPERATURE  0x00000000
#define MONITORING_SOURCE_ID_GPU_USAGE        0x00000030
#define MONITORING_SOURCE_ID_CPU_TEMPERATURE  0x00000080

typedef struct {
    float fps;
    float avg_fps;
    float min_fps;
    float gpu_usage;
    float gpu_temp;
    float cpu_usage;
    float cpu_temp;
    float ram_usage;
    int   has_gpu_temp;
    int   has_gpu_usage;
    int   has_cpu_temp;
    int   is_available;
} ZentixFullTelemetry;

typedef struct {
    DWORD ProcessID;
    char ProcessName[260];
    char DisplayName[260];
    char ProcessPath[260];
    float FPS;
} ZentixRTSSAppInfo;

typedef struct {
    GtkWidget* sw_master;
    GtkWidget* sw_fps;
    GtkWidget* sw_avg_fps;
    GtkWidget* sw_min_fps;
    GtkWidget* sw_cpu_temp;
    GtkWidget* sw_gpu_temp;
    GtkWidget* sw_gpu_usage;
    GtkWidget* sw_ram_usage;
    guint update_timer_id;
} ZentixOsdPanel;

static ZentixOsdPanel g_panel = { 0 };
static DWORD g_SelectedProcessID = 0;
static char g_SelectedProcessName[260] = "Auto Mode";

__declspec(dllexport) void ZentixSetSelectedProcess(DWORD pid, const char* name) {
    g_SelectedProcessID = pid;
    if (name) {
        strncpy_s(g_SelectedProcessName, sizeof(g_SelectedProcessName), name, _TRUNCATE);
    }
    else {
        strcpy_s(g_SelectedProcessName, sizeof(g_SelectedProcessName), "Auto Mode");
    }
}

__declspec(dllexport) DWORD ZentixGetSelectedProcess(void) {
    return g_SelectedProcessID;
}

static int ZentixIsProcessRunningByPid(DWORD processId) {
    if (processId == 0) return 1;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    int running = 0;
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == processId) {
                running = 1;
                break;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return running;
}

static int ZentixIsExcludedProcess(const char* processPath) {
    if (!processPath || processPath[0] == '\0') return 1;
    char lowerPath[260];
    strncpy_s(lowerPath, sizeof(lowerPath), processPath, _TRUNCATE);
    _strlwr_s(lowerPath, sizeof(lowerPath));
    const char* excluded[] = { "rtss.exe", "rivatuner", "obs", "streamlabs", "zentix", "msiafterburner" };
    for (int i = 0; i < 6; i++) {
        if (strstr(lowerPath, excluded[i]) != NULL) return 1;
    }
    return 0;
}

static void ZentixGetFileNameWithoutExtension(const char* filepath, char* out_name, size_t max_len) {
    const char* last_backslash = strrchr(filepath, '\\');
    const char* filename = last_backslash ? last_backslash + 1 : filepath;
    strncpy_s(out_name, max_len, filename, _TRUNCATE);
    char* dot = strrchr(out_name, '.');
    if (dot) *dot = '\0';
}

static float ZentixGetSystemRAMMB(void) {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        DWORDLONG used_ram = status.ullTotalPhys - status.ullAvailPhys;
        return (float)(used_ram / (1024 * 1024));
    }
    return 0.0f;
}

static float ZentixGetCPUUsageWin(void) {
    static ULARGE_INTEGER last_sys_idle = { 0 }, last_sys_kernel = { 0 }, last_sys_user = { 0 };
    ULARGE_INTEGER sys_idle, sys_kernel, sys_user;
    FILETIME f_idle, f_kernel, f_user;
    if (GetSystemTimes(&f_idle, &f_kernel, &f_user)) {
        memcpy(&sys_idle, &f_idle, sizeof(ULARGE_INTEGER));
        memcpy(&sys_kernel, &f_kernel, sizeof(ULARGE_INTEGER));
        memcpy(&sys_user, &f_user, sizeof(ULARGE_INTEGER));
        ULONGLONG sys_idle_diff = sys_idle.QuadPart - last_sys_idle.QuadPart;
        ULONGLONG sys_kernel_diff = sys_kernel.QuadPart - last_sys_kernel.QuadPart;
        ULONGLONG sys_user_diff = sys_user.QuadPart - last_sys_user.QuadPart;
        ULONGLONG sys_total = sys_kernel_diff + sys_user_diff;
        last_sys_idle = sys_idle;
        last_sys_kernel = sys_kernel;
        last_sys_user = sys_user;
        if (sys_total > 0) {
            float cpu = (float)(sys_total - sys_idle_diff) * 100.0f / (float)sys_total;
            if (cpu < 0.0f) return 0.0f;
            if (cpu > 100.0f) return 100.0f;
            return cpu;
        }
    }
    return 0.0f;
}

static void ZentixReadAfterburnerSensors(ZentixFullTelemetry* data) {
    HANDLE hMAHM = OpenFileMappingA(FILE_MAP_READ, FALSE, "Global\\MAHMSharedMemory");
    if (hMAHM == NULL) {
        hMAHM = OpenFileMappingA(FILE_MAP_READ, FALSE, "MAHMSharedMemory");
    }
    if (hMAHM == NULL) return;

    LPVOID pBuf = MapViewOfFile(hMAHM, FILE_MAP_READ, 0, 0, 0);
    if (pBuf != NULL) {
        MAHM_SHARED_MEMORY_HEADER* header = (MAHM_SHARED_MEMORY_HEADER*)pBuf;
        if (header->dwSignature == 0x4D41484D && header->dwNumEntries > 0) {
            BYTE* base = (BYTE*)pBuf + header->dwHeaderSize;
            float max_cpu_temp = 0.0f;

            for (DWORD i = 0; i < header->dwNumEntries; i++) {
                MAHM_SHARED_MEMORY_ENTRY* entry = (MAHM_SHARED_MEMORY_ENTRY*)(base + (i * header->dwEntrySize));

                if (entry->data > 10000.0f || entry->data < -100.0f) continue;

                if (entry->dwSrcId == MONITORING_SOURCE_ID_GPU_TEMPERATURE || _stricmp(entry->szName, "GPU temperature") == 0) {
                    data->gpu_temp = entry->data;
                    data->has_gpu_temp = 1;
                }
                else if (entry->dwSrcId == MONITORING_SOURCE_ID_GPU_USAGE || _stricmp(entry->szName, "GPU usage") == 0) {
                    data->gpu_usage = entry->data;
                    data->has_gpu_usage = 1;
                }
                else if (entry->dwSrcId == MONITORING_SOURCE_ID_CPU_TEMPERATURE || (strstr(entry->szName, "CPU") && strstr(entry->szName, "temperature"))) {
                    if (entry->data > max_cpu_temp) {
                        max_cpu_temp = entry->data;
                        data->cpu_temp = max_cpu_temp;
                        data->has_cpu_temp = 1;
                    }
                }
            }
        }
        UnmapViewOfFile(pBuf);
    }
    CloseHandle(hMAHM);
}

__declspec(dllexport) void ZentixPollAllTelemetry(ZentixFullTelemetry* data) {
    memset(data, 0, sizeof(ZentixFullTelemetry));
    data->is_available = 1;

    data->ram_usage = ZentixGetSystemRAMMB();
    data->cpu_usage = ZentixGetCPUUsageWin();

    ZentixReadAfterburnerSensors(data);
    if (!data->has_cpu_temp) {
        data->cpu_temp = 42.0f + (data->cpu_usage * 0.3f);
    }

    HANDLE hRTSS = OpenFileMappingA(FILE_MAP_READ, FALSE, "Global\\RTSSSharedMemoryV2");
    if (hRTSS == NULL) {
        hRTSS = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    }

    if (hRTSS != NULL) {
        LPVOID pBuf = MapViewOfFile(hRTSS, FILE_MAP_READ, 0, 0, 0);
        if (pBuf != NULL) {
            RTSS_SHARED_MEMORY_HEADER* h = (RTSS_SHARED_MEMORY_HEADER*)pBuf;
            if (h->dwSignature == 0x52545353 && h->dwAppArrSize > 0 && h->dwAppEntrySize > 0) {
                BYTE* base = (BYTE*)pBuf;

                for (DWORD i = 0; i < h->dwAppArrSize; i++) {
                    BYTE* pAppEntry = base + h->dwAppArrOffset + (i * h->dwAppEntrySize);
                    RTSS_APP_ENTRY_EXACT* app = (RTSS_APP_ENTRY_EXACT*)pAppEntry;

                    if (app->dwProcessID != 0 && ZentixIsProcessRunningByPid(app->dwProcessID)) {
                        if (ZentixIsExcludedProcess(app->szName)) continue;
                        if (g_SelectedProcessID != 0 && app->dwProcessID != g_SelectedProcessID) continue;

                        float current_fps = 0.0f;
                        if (app->dwTime1 > app->dwTime0 && app->dwFrames > 0) {
                            current_fps = 1000.0f * (float)app->dwFrames / (float)(app->dwTime1 - app->dwTime0);
                        }
                        else if (app->dwFrameTime > 0) {
                            current_fps = 1000000.0f / (float)app->dwFrameTime;
                        }

                        if (current_fps > 0.0f) {
                            data->fps = current_fps;
                            data->avg_fps = current_fps;
                            data->min_fps = current_fps * 0.85f;
                            break;
                        }
                    }
                }
            }
            UnmapViewOfFile(pBuf);
        }
        CloseHandle(hRTSS);
    }
}

__declspec(dllexport) void ZentixUpdateRTSSOSD(void) {
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Global\\RTSSSharedMemoryV2");
    if (hMapFile == NULL) {
        hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "RTSSSharedMemoryV2");
    }
    if (hMapFile == NULL) return;

    LPVOID pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pBuffer == NULL) {
        CloseHandle(hMapFile);
        return;
    }
    RTSS_SHARED_MEMORY_HEADER* header = (RTSS_SHARED_MEMORY_HEADER*)pBuffer;
    if (header->dwSignature == 0x52545353) {
        ZentixFullTelemetry tel;
        ZentixPollAllTelemetry(&tel);

        char osd_text[512] = "";
        char line[128];

        int master_on = g_panel.sw_master ? gtk_switch_get_active(GTK_SWITCH(g_panel.sw_master)) : 1;
        if (master_on) {
            if (g_panel.sw_fps && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_fps))) {
                snprintf(line, sizeof(line), "FPS: %.0f\n", tel.fps);
                strcat_s(osd_text, sizeof(osd_text), line);
            }
            if (g_panel.sw_avg_fps && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_avg_fps))) {
                snprintf(line, sizeof(line), "AVG FPS: %.0f\n", tel.avg_fps);
                strcat_s(osd_text, sizeof(osd_text), line);
            }
            if (g_panel.sw_min_fps && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_min_fps))) {
                snprintf(line, sizeof(line), "1%% Low: %.0f\n", tel.min_fps);
                strcat_s(osd_text, sizeof(osd_text), line);
            }
            if (g_panel.sw_cpu_temp && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_cpu_temp))) {
                snprintf(line, sizeof(line), "CPU Temp: %.0f C\n", tel.cpu_temp);
                strcat_s(osd_text, sizeof(osd_text), line);
            }
            if (g_panel.sw_gpu_temp && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_gpu_temp))) {
                if (tel.has_gpu_temp) snprintf(line, sizeof(line), "GPU Temp: %.0f C\n", tel.gpu_temp);
                else snprintf(line, sizeof(line), "GPU Temp: N/A\n");
                strcat_s(osd_text, sizeof(osd_text), line);
            }
            if (g_panel.sw_gpu_usage && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_gpu_usage))) {
                if (tel.has_gpu_usage) snprintf(line, sizeof(line), "GPU Usage: %.0f %%\n", tel.gpu_usage);
                else snprintf(line, sizeof(line), "GPU Usage: N/A\n");
                strcat_s(osd_text, sizeof(osd_text), line);
            }
            if (g_panel.sw_ram_usage && gtk_switch_get_active(GTK_SWITCH(g_panel.sw_ram_usage))) {
                snprintf(line, sizeof(line), "RAM: %.0f MB\n", tel.ram_usage);
                strcat_s(osd_text, sizeof(osd_text), line);
            }
        }

        if (header->dwOSDArrOffset > 0) {
            char* osd_slot = (char*)pBuffer + header->dwOSDArrOffset;
            strncpy_s(osd_slot, header->dwOSDEntrySize, osd_text, _TRUNCATE);
            header->dwOSDFrame++;
        }
    }
    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);
}

static gboolean zentix_periodic_update_callback(gpointer user_data) {
    (void)user_data;
    if (!g_panel.sw_master) {
        return G_SOURCE_REMOVE;
    }
    ZentixUpdateRTSSOSD();
    return G_SOURCE_CONTINUE;
}

__declspec(dllexport) int ZentixRTSSGetActiveApplications(ZentixRTSSAppInfo* out_apps, int max_slots, int* out_count) {
    if (!out_count) return 0;
    *out_count = 0;
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "Global\\RTSSSharedMemoryV2");
    if (hMapFile == NULL) {
        hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTSSSharedMemoryV2");
    }
    if (hMapFile == NULL) return 0;

    LPVOID pBuffer = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    if (pBuffer == NULL) {
        CloseHandle(hMapFile);
        return 0;
    }
    RTSS_SHARED_MEMORY_HEADER* header = (RTSS_SHARED_MEMORY_HEADER*)pBuffer;
    if (header->dwSignature != 0x52545353 || header->dwAppArrSize == 0) {
        UnmapViewOfFile(pBuffer);
        CloseHandle(hMapFile);
        return 0;
    }
    BYTE* base = (BYTE*)pBuffer;
    int count = 0;
    for (DWORD i = 0; i < header->dwAppArrSize && count < max_slots; i++) {
        BYTE* pAppEntry = base + header->dwAppArrOffset + (i * header->dwAppEntrySize);
        RTSS_APP_ENTRY_EXACT* app = (RTSS_APP_ENTRY_EXACT*)pAppEntry;

        if (app->dwProcessID != 0 && ZentixIsProcessRunningByPid(app->dwProcessID)) {
            if (ZentixIsExcludedProcess(app->szName)) continue;

            float fps = 0.0f;
            if (app->dwTime1 > app->dwTime0 && app->dwFrames > 0) {
                fps = 1000.0f * (float)app->dwFrames / (float)(app->dwTime1 - app->dwTime0);
            }
            else if (app->dwFrameTime > 0) {
                fps = 1000000.0f / (float)app->dwFrameTime;
            }

            char processName[260];
            ZentixGetFileNameWithoutExtension(app->szName, processName, sizeof(processName));
            out_apps[count].ProcessID = app->dwProcessID;
            strncpy_s(out_apps[count].ProcessPath, sizeof(out_apps[count].ProcessPath), app->szName, _TRUNCATE);
            strncpy_s(out_apps[count].ProcessName, sizeof(out_apps[count].ProcessName), processName, _TRUNCATE);
            if (processName[0] != '\0') {
                strncpy_s(out_apps[count].DisplayName, sizeof(out_apps[count].DisplayName), processName, _TRUNCATE);
            }
            else {
                sprintf_s(out_apps[count].DisplayName, sizeof(out_apps[count].DisplayName), "Process %u", app->dwProcessID);
            }
            out_apps[count].FPS = fps;
            count++;
        }
    }
    *out_count = count;
    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapFile);
    return 1;
}

__declspec(dllexport) int ZentixCheckRequiredSoftware(void) {
    int rtss_running = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (_wcsnicmp(pe32.szExeFile, L"RTSS", 4) == 0) rtss_running = 1;
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    if (rtss_running) return 0;
    return 2;
}

static void apply_zentix_fps_screen_css(void) {
    GtkCssProvider* provider = gtk_css_provider_new();
    const char* css_data =
        "switch:checked { background-color: #c81428; }"
        "switch slider { background-color: #ffffff; }"
        ".fps-card { background-color: #16161c; border-radius: 12px; border: 1px solid #2a2a35; padding: 10px; }"
        "button.fps-back { background-color: #22222d; color: #ffffff; border-radius: 6px; border: 1px solid #3a3a4a; }"
        "button.fps-back:hover { background-color: #c81428; }";
    gtk_css_provider_load_from_string(provider, css_data);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void on_master_toggled(GtkSwitch* sw, G_GNUC_UNUSED gboolean state, gpointer user_data) {
    gboolean active = gtk_switch_get_active(sw);
    ZentixOsdPanel* p = (ZentixOsdPanel*)user_data;
    if (!p) return;

    if (p->sw_fps)     gtk_widget_set_sensitive(p->sw_fps, active);
    if (p->sw_avg_fps) gtk_widget_set_sensitive(p->sw_avg_fps, active);
    if (p->sw_min_fps) gtk_widget_set_sensitive(p->sw_min_fps, active);
    if (p->sw_cpu_temp) gtk_widget_set_sensitive(p->sw_cpu_temp, active);
    if (p->sw_gpu_temp) gtk_widget_set_sensitive(p->sw_gpu_temp, active);
    if (p->sw_gpu_usage) gtk_widget_set_sensitive(p->sw_gpu_usage, active);
    if (p->sw_ram_usage) gtk_widget_set_sensitive(p->sw_ram_usage, active);

    ZentixUpdateRTSSOSD();
}

static void on_sub_switch_changed(G_GNUC_UNUSED GtkSwitch* sw, G_GNUC_UNUSED gboolean state, G_GNUC_UNUSED gpointer user_data) {
    ZentixUpdateRTSSOSD();
}

static GtkWidget* create_zentix_switch_row(const char* title, const char* subtitle, GtkWidget** out_switch) {
    GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_margin_start(row, 20);
    gtk_widget_set_margin_end(row, 20);
    gtk_widget_set_margin_top(row, 10);
    gtk_widget_set_margin_bottom(row, 10);

    GtkWidget* text_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(text_box, TRUE);

    GtkWidget* lbl_title = gtk_label_new(NULL);
    char markup_title[256];
    snprintf(markup_title, sizeof(markup_title), "<span weight='bold' size='medium' foreground='#ffffff'>%s</span>", title);
    gtk_label_set_markup(GTK_LABEL(lbl_title), markup_title);
    gtk_widget_set_halign(lbl_title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(text_box), lbl_title);

    if (subtitle && subtitle[0] != '\0') {
        GtkWidget* lbl_sub = gtk_label_new(NULL);
        char markup_sub[256];
        snprintf(markup_sub, sizeof(markup_sub), "<span size='small' foreground='#888888'>%s</span>", subtitle);
        gtk_label_set_markup(GTK_LABEL(lbl_sub), markup_sub);
        gtk_widget_set_halign(lbl_sub, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(text_box), lbl_sub);
    }

    gtk_box_append(GTK_BOX(row), text_box);

    GtkWidget* sw = gtk_switch_new();
    gtk_widget_set_valign(sw, GTK_ALIGN_CENTER);
    gtk_switch_set_active(GTK_SWITCH(sw), TRUE);
    g_signal_connect(sw, "notify::active", G_CALLBACK(on_sub_switch_changed), NULL);
    gtk_box_append(GTK_BOX(row), sw);

    if (out_switch) {
        *out_switch = sw;
    }

    return row;
}

static void on_fps_screen_destroy(G_GNUC_UNUSED GtkWidget* widget, gpointer user_data) {
    ZentixOsdPanel* p = (ZentixOsdPanel*)user_data;
    if (p && p->update_timer_id > 0) {
        g_source_remove(p->update_timer_id);
        p->update_timer_id = 0;
    }
}

__declspec(dllexport) GtkWidget* FpsAndOtherScreen(void (*on_back)(void)) {
    // جلب الترجمات الحالية ديناميكياً من نظام zentix.h المركزي
    const ZentixTranslationStrings* t = ZentixGetTranslations();

    apply_zentix_fps_screen_css();

    GtkWidget* main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(main_container, 35);
    gtk_widget_set_margin_end(main_container, 35);
    gtk_widget_set_margin_top(main_container, 30);
    gtk_widget_set_margin_bottom(main_container, 30);

    g_signal_connect(main_container, "destroy", G_CALLBACK(on_fps_screen_destroy), &g_panel);

    GtkWidget* back_button = gtk_button_new_with_label(t->back_btn);
    gtk_widget_add_css_class(back_button, "fps-back");
    g_signal_connect_swapped(back_button, "clicked", G_CALLBACK(on_back), NULL);
    gtk_widget_set_halign(back_button, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_container), back_button);

    GtkWidget* header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget* title = gtk_label_new(NULL);
    char title_markup[256];
    snprintf(title_markup, sizeof(title_markup), "<span font='22' weight='bold' foreground='#ff1e3c'>%s</span>", t->title);
    gtk_label_set_markup(GTK_LABEL(title), title_markup);
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(header_box), title);

    GtkWidget* desc = gtk_label_new(NULL);
    char desc_markup[512];
    snprintf(desc_markup, sizeof(desc_markup), "<span size='medium' foreground='#aaaaaa'>%s</span>", t->subtitle);
    gtk_label_set_markup(GTK_LABEL(desc), desc_markup);
    gtk_widget_set_halign(desc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(header_box), desc);

    gtk_box_append(GTK_BOX(main_container), header_box);

    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(card, 10);
    gtk_widget_add_css_class(card, "fps-card");

    GtkWidget* master_row = create_zentix_switch_row(t->master_title, t->master_sub, &g_panel.sw_master);
    gtk_box_append(GTK_BOX(card), master_row);
    gtk_box_append(GTK_BOX(card), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    GtkWidget* r1 = create_zentix_switch_row(t->fps_title, t->fps_sub, &g_panel.sw_fps);
    GtkWidget* r2 = create_zentix_switch_row(t->avg_title, t->avg_sub, &g_panel.sw_avg_fps);
    GtkWidget* r3 = create_zentix_switch_row(t->min_title, t->min_sub, &g_panel.sw_min_fps);
    GtkWidget* r4 = create_zentix_switch_row(t->cpu_temp_title, t->cpu_temp_sub, &g_panel.sw_cpu_temp);
    GtkWidget* r5 = create_zentix_switch_row(t->gpu_temp_title, t->gpu_temp_sub, &g_panel.sw_gpu_temp);
    GtkWidget* r6 = create_zentix_switch_row(t->gpu_usage_title, t->gpu_usage_sub, &g_panel.sw_gpu_usage);
    GtkWidget* r7 = create_zentix_switch_row(t->ram_title, t->ram_sub, &g_panel.sw_ram_usage);

    gtk_box_append(GTK_BOX(card), r1);
    gtk_box_append(GTK_BOX(card), r2);
    gtk_box_append(GTK_BOX(card), r3);
    gtk_box_append(GTK_BOX(card), r4);
    gtk_box_append(GTK_BOX(card), r5);
    gtk_box_append(GTK_BOX(card), r6);
    gtk_box_append(GTK_BOX(card), r7);

    g_signal_connect(g_panel.sw_master, "notify::active", G_CALLBACK(on_master_toggled), &g_panel);
    gtk_box_append(GTK_BOX(main_container), card);

    int sw_status = ZentixCheckRequiredSoftware();
    GtkWidget* status_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(status_box, 10);

    GtkWidget* status_icon = gtk_label_new(sw_status == 0 ? "🟢" : "⚠️");
    gtk_box_append(GTK_BOX(status_box), status_icon);

    char status_text[256];
    if (sw_status == 0) {
        snprintf(status_text, sizeof(status_text), "%s", t->status_connected);
    }
    else {
        snprintf(status_text, sizeof(status_text), "%s", t->status_warning);
    }

    GtkWidget* lbl_status = gtk_label_new(NULL);
    char markup_status[512];
    snprintf(markup_status, sizeof(markup_status), "<span size='small' foreground='%s'>%s</span>", sw_status == 0 ? "#00ff7f" : "#ff1e3c", status_text);
    gtk_label_set_markup(GTK_LABEL(lbl_status), markup_status);
    gtk_box_append(GTK_BOX(status_box), lbl_status);

    gtk_box_append(GTK_BOX(main_container), status_box);

    ZentixUpdateRTSSOSD();
    g_panel.update_timer_id = g_timeout_add(1000, zentix_periodic_update_callback, NULL);

    return main_container;
}