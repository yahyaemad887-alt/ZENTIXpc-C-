#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <gtk/gtk.h>
#include "zentix.h"

// مؤشر عام للـ GtkStack للتحكم في التنقل بين الشاشات
static GtkStack* global_stack = NULL;

// ============================================================================
// دالة قراءة ملف config.ini وتطبيق اللغة (مطابقة 100% لما يكتبه برنامج التثبيت)
// ============================================================================
static void apply_saved_language(void) {
    FILE* file = fopen("config.ini", "r");
    int lang_idx = 0; // القيمة الافتراضية (0 = English)

    if (file != NULL) {
        if (fscanf(file, "%d", &lang_idx) != 1) {
            lang_idx = 0;
        }
        fclose(file);
    }

    // تطبيق اللغة مباشرة باستخدام دالة المشروع المعتمدة
    ZentixSetLanguage((ZentixAppLanguage)lang_idx);

    // ضبط بيئة العمل (Locale) بناءً على رقم الـ Index المقروء
    switch (lang_idx) {
    case 1: // Arabic
        g_setenv("LANG", "ar_EG.UTF-8", TRUE);
        g_setenv("LANGUAGE", "ar", TRUE);
        break;
    case 2: // German
        g_setenv("LANG", "de_DE.UTF-8", TRUE);
        g_setenv("LANGUAGE", "de", TRUE);
        break;
    case 3: // French
        g_setenv("LANG", "fr_FR.UTF-8", TRUE);
        g_setenv("LANGUAGE", "fr", TRUE);
        break;
    case 4: // Japanese
        g_setenv("LANG", "ja_JP.UTF-8", TRUE);
        g_setenv("LANGUAGE", "ja", TRUE);
        break;
    case 5: // Korean
        g_setenv("LANG", "ko_KR.UTF-8", TRUE);
        g_setenv("LANGUAGE", "ko", TRUE);
        break;
    case 6: // Russian
        g_setenv("LANG", "ru_RU.UTF-8", TRUE);
        g_setenv("LANGUAGE", "ru", TRUE);
        break;
    case 7: // Turkish
        g_setenv("LANG", "tr_TR.UTF-8", TRUE);
        g_setenv("LANGUAGE", "tr", TRUE);
        break;
    case 8: // Chinese
        g_setenv("LANG", "zh_CN.UTF-8", TRUE);
        g_setenv("LANGUAGE", "zh", TRUE);
        break;
    default: // English (0)
        g_setenv("LANG", "en_US.UTF-8", TRUE);
        g_setenv("LANGUAGE", "en", TRUE);
        break;
    }
}

// ============================================================================
// دوال التنقل بين الشاشات
// ============================================================================
static void navigate_to_home(void) {
    if (global_stack && GTK_IS_STACK(global_stack)) {
        gtk_stack_set_visible_child_name(global_stack, "home_screen");
    }
}

static void navigate_to_settings(void) {
    if (global_stack && GTK_IS_STACK(global_stack)) {
        gtk_stack_set_visible_child_name(global_stack, "settings_screen");
    }
}

static void navigate_to_fps_suite(void) {
    if (global_stack && GTK_IS_STACK(global_stack)) {
        gtk_stack_set_visible_child_name(global_stack, "msi_suite_screen");
    }
}

static void navigate_to_benchmark(void) {
    if (global_stack && GTK_IS_STACK(global_stack)) {
        gtk_stack_set_visible_child_name(global_stack, "logger_screen");
    }
}

static void navigate_to_overclock(void) {
    if (global_stack && GTK_IS_STACK(global_stack)) {
        gtk_stack_set_visible_child_name(global_stack, "kernel_screen");
    }
}

static void navigate_to_gamemode(void) {
    if (global_stack && GTK_IS_STACK(global_stack)) {
        gtk_stack_set_visible_child_name(global_stack, "gamemode_screen");
    }
}

// ============================================================================
// دالة تهيئة التطبيق الرئيسية
// ============================================================================
static void on_activate(GtkApplication* app, gpointer user_data) {
    if (!app) return;
    (void)user_data;

    // 1. إنشاء النافذة الرئيسية للديسك توب
    GtkWidget* window = gtk_application_window_new(app);
    if (!window) return;

    gtk_window_set_title(GTK_WINDOW(window), "ZENTIX — High Performance Center (Desktop Edition)");
    gtk_window_set_default_size(GTK_WINDOW(window), 1150, 720);

    // 2. إنشاء نظام التنقل GtkStack مباشرة
    GtkWidget* stack_widget = gtk_stack_new();
    global_stack = GTK_STACK(stack_widget);
    gtk_stack_set_transition_type(global_stack, GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(global_stack, 250);

    // 3. إعداد الكولباكس الخاصة بالشاشة الرئيسية
    HomeScreenCallbacks home_callbacks = {
        .onNavigateToSettings = navigate_to_settings,
        .onNavigateToMSISuite = navigate_to_fps_suite,
        .onNavigateToBenchmark = navigate_to_benchmark,
        .onNavigateToProfiles = NULL,
        .onNavigateToOverclock = navigate_to_overclock,
        .onNavigateToGameMode = navigate_to_gamemode
    };

    // 4. بناء وإضافة جميع الشاشات للـ Stack
    GtkWidget* home_widget = HomeScreen(home_callbacks);
    if (home_widget) {
        gtk_stack_add_named(global_stack, home_widget, "home_screen");
    }

    GtkWidget* settings_widget = SettingsScreen(navigate_to_home);
    if (settings_widget) {
        gtk_stack_add_named(global_stack, settings_widget, "settings_screen");
    }

    SystemInfoCallbacks sys_callbacks = {
        .onBackToHome = navigate_to_home
    };
    GtkWidget* logger_widget = SystemInfoScreen(sys_callbacks);
    if (logger_widget) {
        gtk_stack_add_named(global_stack, logger_widget, "logger_screen");
    }

    GtkWidget* msi_widget = FpsAndOtherScreen(navigate_to_home);
    if (msi_widget) {
        gtk_stack_add_named(global_stack, msi_widget, "msi_suite_screen");
    }

    GtkWidget* gamemode_widget = GameModeScreen(navigate_to_home);
    if (gamemode_widget) {
        gtk_stack_add_named(global_stack, gamemode_widget, "gamemode_screen");
    }

    // 5. البدء بشاشة الـ Home أولاً وتعريفها كطفل مباشر للنافذة
    gtk_stack_set_visible_child_name(global_stack, "home_screen");

    gtk_window_set_child(GTK_WINDOW(window), stack_widget);
    gtk_window_present(GTK_WINDOW(window));
}

// ============================================================================
// نقطة الدخول الرئيسية للبرنامج (main)
// ============================================================================
int main(int argc, char** argv) {
    // 0. قراءة لغة البرنامج من ملف config.ini وتطبيقها فوراً قبل أي شيء
    apply_saved_language();

    // منع GStreamer نهائياً من محاولة تحميل بلاجنز الـ D3D12/D3D11 وتجنب أي كراش
    g_setenv("GST_PLUGINS_NO_LOAD", "libgstd3d12:libgstd3d11:d3d12:d3d11:d3d:dxva", TRUE);
    g_setenv("GST_DEBUG", "0", TRUE);

    GtkApplication* app = gtk_application_new("com.example.zentix", G_APPLICATION_DEFAULT_FLAGS);
    if (!app) return 1;

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return status;
}