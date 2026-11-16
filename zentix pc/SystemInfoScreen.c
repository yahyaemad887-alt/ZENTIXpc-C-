#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <windows.h>
#include <sysinfoapi.h>
#include "zentix.h"            // الهيكل المركزي الموحد للترجمات
#include "SystemInfoScreen.h"  // هيدر شاشة معلومات النظام

// 1. جلب إجمالي الرام الفعلي ديناميكياً
static void get_dynamic_ram(char* buffer, size_t size) {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        double total_gb = (double)status.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
        snprintf(buffer, size, "%.1f GB", total_gb);
    }
    else {
        snprintf(buffer, size, "Unknown RAM");
    }
}

// 2. جلب اسم المعالج (CPU) الحقيقي لأي جهاز ديناميكياً من الريجستري
static void get_dynamic_cpu(char* buffer, size_t size) {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD buf_size = (DWORD)size;
        if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)buffer, &buf_size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return;
        }
        RegCloseKey(hKey);
    }
    snprintf(buffer, size, "Standard x64 Processor");
}

// 3. جلب اسم كارت الشاشة (GPU) الحقيقي حتماً وبدون أي قيم فارغة عبر فحص مفاتيح الريجستري النشطة
static void get_dynamic_gpu(char* buffer, size_t size) {
    HKEY hParentKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}", 0, KEY_READ, &hParentKey) == ERROR_SUCCESS) {
        char subkey_name[32];
        DWORD index = 0;
        DWORD name_size = sizeof(subkey_name);

        while (RegEnumKeyExA(hParentKey, index, subkey_name, &name_size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            HKEY hSubKey;
            if (RegOpenKeyExA(hParentKey, subkey_name, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                char desc_buf[256];
                DWORD desc_size = sizeof(desc_buf);
                DWORD type = 0;
                if (RegQueryValueExA(hSubKey, "DriverDesc", NULL, &type, (LPBYTE)desc_buf, &desc_size) == ERROR_SUCCESS) {
                    if (strlen(desc_buf) > 0 && strstr(desc_buf, "Monitor") == NULL) {
                        strncpy_s(buffer, size, desc_buf, _TRUNCATE);
                        RegCloseKey(hSubKey);
                        RegCloseKey(hParentKey);
                        return;
                    }
                }
                RegCloseKey(hSubKey);
            }
            index++;
            name_size = sizeof(subkey_name);
        }
        RegCloseKey(hParentKey);
    }
    snprintf(buffer, size, "Standard Display Adapter");
}

// 4. جلب اسم جهاز المستخدم ديناميكياً
static void get_dynamic_computer_name(char* buffer, size_t size) {
    DWORD size_dw = (DWORD)size;
    if (!GetComputerNameA(buffer, &size_dw)) {
        snprintf(buffer, size, "DESKTOP-USER");
    }
}

// 5. جلب مساحة الهارد الإجمالية للكمبيوتر بالكامل
static void get_dynamic_storage(char* buffer, size_t size) {
    ULARGE_INTEGER total_all_bytes = { 0 };
    char drive_strings[256];
    DWORD len = GetLogicalDriveStringsA(sizeof(drive_strings) - 1, drive_strings);

    if (len > 0) {
        char* p = drive_strings;
        while (*p) {
            if (GetDriveTypeA(p) == DRIVE_FIXED) {
                ULARGE_INTEGER total_bytes, free_bytes, available_bytes;
                if (GetDiskFreeSpaceExA(p, &available_bytes, &total_bytes, &free_bytes)) {
                    total_all_bytes.QuadPart += total_bytes.QuadPart;
                }
            }
            p += strlen(p) + 1;
        }
    }

    if (total_all_bytes.QuadPart > 0) {
        double total_tb = (double)total_all_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0 * 1024.0);
        if (total_tb >= 0.95) {
            snprintf(buffer, size, "%.1f TB Total Storage", total_tb);
        }
        else {
            double total_gb = (double)total_all_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
            snprintf(buffer, size, "%.0f GB Total Storage", total_gb);
        }
    }
    else {
        snprintf(buffer, size, "Local Storage Drive");
    }
}

static void on_back_clicked_cb(GtkButton* btn, gpointer user_data) {
    void (*callback)(void) = (void(*)(void))user_data;
    if (callback) {
        callback();
    }
    (void)btn;
}

static GtkWidget* CreateTopMiniCard(const char* title, const char* value, const char* subvalue) {
    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(card, "sys-mini-card");
    gtk_widget_set_hexpand(card, TRUE);

    GtkWidget* title_lbl = gtk_label_new(title);
    gtk_widget_add_css_class(title_lbl, "sys-mini-title");
    gtk_widget_set_halign(title_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card), title_lbl);

    GtkWidget* val_lbl = gtk_label_new(value);
    gtk_widget_add_css_class(val_lbl, "sys-mini-value");
    gtk_widget_set_halign(val_lbl, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(val_lbl), TRUE);
    gtk_box_append(GTK_BOX(card), val_lbl);

    if (subvalue && subvalue[0] != '\0') {
        GtkWidget* sub_lbl = gtk_label_new(subvalue);
        gtk_widget_add_css_class(sub_lbl, "sys-mini-sub");
        gtk_widget_set_halign(sub_lbl, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(card), sub_lbl);
    }

    return card;
}

static GtkWidget* CreateDetailRow(const char* label_text, const char* val_text) {
    GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_add_css_class(row, "detail-row");
    gtk_widget_set_hexpand(row, TRUE);

    GtkWidget* lbl = gtk_label_new(label_text);
    gtk_widget_add_css_class(lbl, "detail-label");
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_widget_set_size_request(lbl, 180, -1);
    gtk_box_append(GTK_BOX(row), lbl);

    GtkWidget* val = gtk_label_new(val_text);
    gtk_widget_add_css_class(val, "detail-value");
    gtk_widget_set_halign(val, GTK_ALIGN_START);
    gtk_widget_set_hexpand(val, TRUE);
    gtk_label_set_wrap(GTK_LABEL(val), TRUE);
    gtk_box_append(GTK_BOX(row), val);

    return row;
}

__declspec(dllexport) GtkWidget* SystemInfoScreen(SystemInfoCallbacks callbacks) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();

    GtkWidget* scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);

    GtkWidget* main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 24);
    gtk_widget_set_margin_start(main_container, 40);
    gtk_widget_set_margin_end(main_container, 40);
    gtk_widget_set_margin_top(main_container, 30);
    gtk_widget_set_margin_bottom(main_container, 40);

    GtkWidget* top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_valign(top_bar, GTK_ALIGN_CENTER);

    GtkWidget* back_btn = gtk_button_new_with_label(t->back_btn);
    gtk_widget_add_css_class(back_btn, "back-nav-button");
    if (callbacks.onBackToHome) {
        g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked_cb), callbacks.onBackToHome);
    }
    gtk_box_append(GTK_BOX(top_bar), back_btn);

    GtkWidget* header_lbl = gtk_label_new(t->sys_header_title ? t->sys_header_title : "System Information & Benchmarks");
    gtk_widget_add_css_class(header_lbl, "screen-header-title");
    gtk_box_append(GTK_BOX(top_bar), header_lbl);

    gtk_box_append(GTK_BOX(main_container), top_bar);

    GtkWidget* sub_info = gtk_label_new(t->sys_header_subtitle ? t->sys_header_subtitle : "Inspect full hardware specifications and system architecture.");
    gtk_widget_add_css_class(sub_info, "screen-header-subtitle");
    gtk_widget_set_halign(sub_info, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_container), sub_info);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(main_container), separator);

    // --- قراءة المواصفات الحقيقية بالكامل لحظياً ---
    char cpu_buf[256];
    get_dynamic_cpu(cpu_buf, sizeof(cpu_buf));

    char gpu_buf[256];
    get_dynamic_gpu(gpu_buf, sizeof(gpu_buf));

    char pc_name[128];
    get_dynamic_computer_name(pc_name, sizeof(pc_name));

    char ram_str[64];
    get_dynamic_ram(ram_str, sizeof(ram_str));

    char disk_str[64];
    get_dynamic_storage(disk_str, sizeof(disk_str));

    // --- بناء الكروت العلوية ---
    GtkWidget* cards_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_hexpand(cards_row, TRUE);

    gtk_box_append(GTK_BOX(cards_row), CreateTopMiniCard(t->sys_card_ram_title ? t->sys_card_ram_title : "System RAM", ram_str, t->sys_card_ram_sub ? t->sys_card_ram_sub : "Physical Memory"));
    gtk_box_append(GTK_BOX(cards_row), CreateTopMiniCard(t->sys_card_cpu_title ? t->sys_card_cpu_title : "Processor (CPU)", cpu_buf, t->sys_card_cpu_sub ? t->sys_card_cpu_sub : "Architecture"));
    gtk_box_append(GTK_BOX(cards_row), CreateTopMiniCard(t->sys_card_gpu_title ? t->sys_card_gpu_title : "Graphics (GPU)", gpu_buf, t->sys_card_gpu_sub ? t->sys_card_gpu_sub : "Display Adapter"));
    gtk_box_append(GTK_BOX(cards_row), CreateTopMiniCard(t->sys_card_disk_title ? t->sys_card_disk_title : "Storage Drive", disk_str, t->sys_card_disk_sub ? t->sys_card_disk_sub : "Local Fixed Drives"));

    gtk_box_append(GTK_BOX(main_container), cards_row);

    // --- بناء الجدول التفصيلي لمواصفات الجهاز ---
    GtkWidget* spec_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(spec_section, "specs-section-box");

    GtkWidget* spec_title = gtk_label_new(t->sys_specs_title ? t->sys_specs_title : "Hardware Specifications Details");
    gtk_widget_add_css_class(spec_title, "specs-section-title");
    gtk_widget_set_halign(spec_title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(spec_section), spec_title);

    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_device_name ? t->sys_row_device_name : "Device Name", pc_name));
    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_processor ? t->sys_row_processor : "Processor", cpu_buf));
    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_ram ? t->sys_row_ram : "Installed RAM", ram_str));
    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_gpu ? t->sys_row_gpu : "Graphics Card", gpu_buf));
    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_storage ? t->sys_row_storage : "Total Storage", disk_str));
    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_sys_type ? t->sys_row_sys_type : "System Type", t->sys_val_sys_type ? t->sys_val_sys_type : "64-bit Operating System, x64-based processor"));
    gtk_box_append(GTK_BOX(spec_section), CreateDetailRow(t->sys_row_os ? t->sys_row_os : "Operating System", t->sys_val_os ? t->sys_val_os : "Microsoft Windows 10/11 Pro (x64)"));

    gtk_box_append(GTK_BOX(main_container), spec_section);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), main_container);

    return scrolled_window;
}