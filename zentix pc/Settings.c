#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <windows.h>
#include "zentix.h"
#include "Settings.h" // بدلاً من SettingsScreen.h

// دالة لحفظ اللغة المختارة في ملف على الجهاز
static void save_language_to_file(int lang) {
    FILE* f = fopen(ZENTIX_CONFIG_FILE, "w");
    if (f) {
        fprintf(f, "%d", lang);
        fclose(f);
    }
}

static void on_back_clicked_cb(GtkButton* btn, gpointer user_data) {
    (void)btn;
    void (*callback)(void) = (void(*)(void))user_data;
    if (callback) {
        callback();
    } 
}

// دالة تفاعلية لمعالجة اختيار اللغة وتحديث التطبيق لحظياً
static void on_language_selected(GtkDropDown* dropdown, GParamSpec* pspec, gpointer user_data) {
    guint selected = gtk_drop_down_get_selected(dropdown);

    // تحديث اللغة مركزياً عبر دالة الهيدر
    ZentixSetLanguage((ZentixAppLanguage)selected);

    // حفظ الاختيار في ملف على الجهاز ليفضل ثابت دائماً
    save_language_to_file((int)selected);

    (void)pspec;
    (void)user_data;
}

__declspec(dllexport) GtkWidget* SettingsScreen(void (*onNavigateBack)(void)) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();
    int current_lang = (int)ZentixGetLanguage();

    GtkWidget* scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);

    GtkWidget* main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 24);
    gtk_widget_set_margin_start(main_container, 40);
    gtk_widget_set_margin_end(main_container, 40);
    gtk_widget_set_margin_top(main_container, 30);
    gtk_widget_set_margin_bottom(main_container, 40);

    // شريط التنقل العلوي وزر الرجوع
    GtkWidget* top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_valign(top_bar, GTK_ALIGN_CENTER);

    GtkWidget* back_btn = gtk_button_new_with_label(t->back_btn);
    gtk_widget_add_css_class(back_btn, "back-nav-button");
    if (onNavigateBack) {
        g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked_cb), onNavigateBack);
    }
    gtk_box_append(GTK_BOX(top_bar), back_btn);

    GtkWidget* header_lbl = gtk_label_new(t->settings_title ? t->settings_title : "Settings & Preferences");
    gtk_widget_add_css_class(header_lbl, "screen-header-title");
    gtk_box_append(GTK_BOX(top_bar), header_lbl);

    gtk_box_append(GTK_BOX(main_container), top_bar);

    GtkWidget* sub_info = gtk_label_new(t->settings_subtitle ? t->settings_subtitle : "Configure application preferences, language, and system parameters.");
    gtk_widget_add_css_class(sub_info, "screen-header-subtitle");
    gtk_widget_set_halign(sub_info, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(main_container), sub_info);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(main_container), separator);

    // --- صندوق قسم الإعدادات العامة ---
    GtkWidget* settings_section = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_add_css_class(settings_section, "specs-section-box");

    GtkWidget* sec_title = gtk_label_new(t->settings_general_title ? t->settings_general_title : "General Preferences");
    gtk_widget_add_css_class(sec_title, "specs-section-title");
    gtk_widget_set_halign(sec_title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(settings_section), sec_title);

    // 1. قائمة اختيار اللغات التفاعلية
    GtkWidget* lang_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(lang_row, "detail-row");
    gtk_widget_set_hexpand(lang_row, TRUE);

    GtkWidget* lang_lbl = gtk_label_new(t->settings_lang_label ? t->settings_lang_label : "Application Language");
    gtk_widget_add_css_class(lang_lbl, "detail-label");
    gtk_widget_set_halign(lang_lbl, GTK_ALIGN_START);
    gtk_widget_set_hexpand(lang_lbl, TRUE);
    gtk_box_append(GTK_BOX(lang_row), lang_lbl);

    const char* languages[] = {
        "English (EN)",
        "العربية (AR)",
        "Deutsch (DE)",
        "Français (FR)",
        "日本語 (JA)",
        "한국어 (KO)",
        "Русский (RU)",
        "Türkçe (TR)",
        "中文 (ZH)",
        NULL
    };
    GtkWidget* lang_dropdown = gtk_drop_down_new_from_strings(languages);
    gtk_widget_set_valign(lang_dropdown, GTK_ALIGN_CENTER);

    // ضبط القائمة على اللغة المحفوظة حالياً
    gtk_drop_down_set_selected(GTK_DROP_DOWN(lang_dropdown), (guint)current_lang);

    g_signal_connect(lang_dropdown, "notify::selected", G_CALLBACK(on_language_selected), NULL);
    gtk_box_append(GTK_BOX(lang_row), lang_dropdown);

    gtk_box_append(GTK_BOX(settings_section), lang_row);

    // 2. عرض معلومة الإصدار
    GtkWidget* ver_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_add_css_class(ver_row, "detail-row");
    gtk_widget_set_hexpand(ver_row, TRUE);

    GtkWidget* ver_lbl = gtk_label_new(t->settings_version_label ? t->settings_version_label : "Software Version");
    gtk_widget_add_css_class(ver_lbl, "detail-label");
    gtk_widget_set_halign(ver_lbl, GTK_ALIGN_START);
    gtk_widget_set_hexpand(ver_lbl, TRUE);
    gtk_box_append(GTK_BOX(ver_row), ver_lbl);

    GtkWidget* ver_val = gtk_label_new("Zentix v1.0.0 (Release x64)");
    gtk_widget_add_css_class(ver_val, "detail-value");
    gtk_widget_set_valign(ver_val, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(ver_row), ver_val);

    gtk_box_append(GTK_BOX(settings_section), ver_row);

    gtk_box_append(GTK_BOX(main_container), settings_section);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), main_container);

    return scrolled_window;
}