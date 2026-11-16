#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <windows.h>
#include <stdio.h>
#include "zentix.h"

static GtkWidget* wizard_win = NULL;
static GtkWidget* stack = NULL;
static GtkWidget* btn_next = NULL;
static GtkWidget* btn_back = NULL;
static GtkWidget* btn_cancel = NULL;

// عناصر الصفحات للتحديث اللحظي
static GtkWidget* lbl_lang_title, * lbl_lang_desc;
static GtkWidget* lbl_comp_title, * chk_core, * lbl_rtss_warn, * btn_rtss_link;
static GtkWidget* lbl_loc_title, * entry_path, * lbl_space_info;
static GtkWidget* lbl_ready_title, * lbl_ready_desc, * prog_bar;

static void (*on_install_complete_cb)(void) = NULL;
static int selected_lang_idx = 0;

// حفظ اللغة في ملف الإعدادات الخاص بالتطبيق
static void sync_language_with_app(int lang_idx) {
    selected_lang_idx = lang_idx;
    ZentixSetLanguage((ZentixAppLanguage)lang_idx);

    FILE* f = fopen(ZENTIX_CONFIG_FILE, "w");
    if (f) {
        fprintf(f, "%d", lang_idx);
        fclose(f);
    }
}

// حساب المساحة المتاحة الحقيقية على القرص
static void update_disk_space(const char* path) {
    char root[4] = "C:\\";
    if (path && strlen(path) >= 2 && path[1] == ':') {
        root[0] = path[0];
    }

    ULARGE_INTEGER free_bytes;
    double free_gb = 0.0;
    if (GetDiskFreeSpaceExA(root, &free_bytes, NULL, NULL)) {
        free_gb = (double)free_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
    }

    char buf[256];
    if (selected_lang_idx == 1) {
        snprintf(buf, sizeof(buf), "المساحة المطلوبة: 55.0 MB\nالمساحة المتاحة على القرص (%s): %.1f GB", root, free_gb);
    }
    else {
        snprintf(buf, sizeof(buf), "Space required: 55.0 MB\nSpace available on (%s): %.1f GB", root, free_gb);
    }
    gtk_label_set_text(GTK_LABEL(lbl_space_info), buf);
}

// ترجمة نصوص الـ Wizard حسب اللغة المختارة
static void refresh_wizard_texts(void) {
    const char* visible = gtk_stack_get_visible_child_name(GTK_STACK(stack));
    gboolean is_last = (g_strcmp0(visible, "page_install") == 0);

    // 0:EN, 1:AR, 2:DE, 3:FR, 4:JA, 5:KO, 6:RU, 7:TR, 8:ZH
    const char* titles[] = {
        "Zentix PC Setup", "تثبيت Zentix PC", "Zentix PC Setup", "Installation de Zentix PC",
        "Zentix PC セットアップ", "Zentix PC 설치", "Установка Zentix PC", "Zentix PC Kurulumu", "Zentix PC 安装"
    };
    const char* next_txt[] = { "Next >", "التالي >", "Weiter >", "Suivant >", "次へ >", "다음 >", "Далее >", "İleri >", "下一步 >" };
    const char* inst_txt[] = { "Install", "تثبيت", "Installieren", "Installer", "インストール", "설치", "Установить", "Kur", "安装" };
    const char* back_txt[] = { "< Back", "< السابق", "< Zurück", "< Précédent", "< 戻る", "< 이전", "< Назад", "< Geri", "< 上一步" };
    const char* canc_txt[] = { "Cancel", "إلغاء", "Abbrechen", "Annuler", "キャンセル", "취소", "Отмена", "İptal", "取消" };

    gtk_window_set_title(GTK_WINDOW(wizard_win), titles[selected_lang_idx]);
    gtk_button_set_label(GTK_BUTTON(btn_next), is_last ? inst_txt[selected_lang_idx] : next_txt[selected_lang_idx]);
    gtk_button_set_label(GTK_BUTTON(btn_back), back_txt[selected_lang_idx]);
    gtk_button_set_label(GTK_BUTTON(btn_cancel), canc_txt[selected_lang_idx]);

    if (selected_lang_idx == 1) { // العربية
        gtk_label_set_text(GTK_LABEL(lbl_lang_title), "لغة التثبيت والتطبيق");
        gtk_label_set_text(GTK_LABEL(lbl_lang_desc), "اختر اللغة التي ترغب في استخدامها داخل معالج التثبيت والتطبيق:");
        gtk_label_set_text(GTK_LABEL(lbl_comp_title), "اختيار المكونات والمتطلبات");
        gtk_check_button_set_label(GTK_CHECK_BUTTON(chk_core), "تطبيق Zentix PC الأساسي (مطلوب)");
        gtk_label_set_text(GTK_LABEL(lbl_rtss_warn), "تنبيه هام: لكي يعمل قسم (FPS & Telemetry) ويعرض الفريمات والحرارة بدقة، يجب تحميل وتثبيت برنامج RivaTuner Statistics Server (RTSS):");
        gtk_button_set_label(GTK_BUTTON(btn_rtss_link), "تحميل RTSS 7.3.7 من الموقع الرسمي");
        gtk_label_set_text(GTK_LABEL(lbl_loc_title), "اختر مسار التثبيت");
        gtk_label_set_text(GTK_LABEL(lbl_ready_title), "جاهز للتثبيت");
        gtk_label_set_text(GTK_LABEL(lbl_ready_desc), "اضغط على زر (تثبيت) لإنشاء المجلدات وتهيئة Zentix PC للعمل فوراً.");
    }
    else if (selected_lang_idx == 2) { // Deutsch
        gtk_label_set_text(GTK_LABEL(lbl_lang_title), "Installationssprache");
        gtk_label_set_text(GTK_LABEL(lbl_lang_desc), "Wählen Sie die Sprache für das Setup und die Anwendung:");
        gtk_label_set_text(GTK_LABEL(lbl_comp_title), "Komponenten auswählen");
        gtk_check_button_set_label(GTK_CHECK_BUTTON(chk_core), "Zentix PC Hauptanwendung (Erforderlich)");
        gtk_label_set_text(GTK_LABEL(lbl_rtss_warn), "Hinweis: Damit FPS & Telemetrie funktioniert, laden Sie bitte RivaTuner Statistics Server (RTSS) herunter:");
        gtk_button_set_label(GTK_BUTTON(btn_rtss_link), "RTSS 7.3.7 herunterladen");
        gtk_label_set_text(GTK_LABEL(lbl_loc_title), "Zielordner auswählen");
        gtk_label_set_text(GTK_LABEL(lbl_ready_title), "Bereit zur Installation");
        gtk_label_set_text(GTK_LABEL(lbl_ready_desc), "Klicken Sie auf Installieren, um fortzufahren.");
    }
    else if (selected_lang_idx == 3) { // Français
        gtk_label_set_text(GTK_LABEL(lbl_lang_title), "Langue d'installation");
        gtk_label_set_text(GTK_LABEL(lbl_lang_desc), "Sélectionnez la langue de l'assistant et de l'application :");
        gtk_label_set_text(GTK_LABEL(lbl_comp_title), "Choisir les composants");
        gtk_check_button_set_label(GTK_CHECK_BUTTON(chk_core), "Application Zentix PC (Requis)");
        gtk_label_set_text(GTK_LABEL(lbl_rtss_warn), "Remarque : Pour que FPS & Télémétrie fonctionne, veuillez télécharger RivaTuner Statistics Server (RTSS) :");
        gtk_button_set_label(GTK_BUTTON(btn_rtss_link), "Télécharger RTSS 7.3.7");
        gtk_label_set_text(GTK_LABEL(lbl_loc_title), "Dossier de destination");
        gtk_label_set_text(GTK_LABEL(lbl_ready_title), "Prêt à installer");
        gtk_label_set_text(GTK_LABEL(lbl_ready_desc), "Cliquez sur Installer pour lancer l'installation.");
    }
    else { // English & باقي اللغات
        gtk_label_set_text(GTK_LABEL(lbl_lang_title), "Installer & Application Language");
        gtk_label_set_text(GTK_LABEL(lbl_lang_desc), "Please select the language for setup and Zentix PC:");
        gtk_label_set_text(GTK_LABEL(lbl_comp_title), "Choose Components & Requirements");
        gtk_check_button_set_label(GTK_CHECK_BUTTON(chk_core), "Zentix PC Application (Required)");
        gtk_label_set_text(GTK_LABEL(lbl_rtss_warn), "Notice: For FPS & Telemetry to work properly, please download and install RivaTuner Statistics Server (RTSS):");
        gtk_button_set_label(GTK_BUTTON(btn_rtss_link), "Download RTSS 7.3.7");
        gtk_label_set_text(GTK_LABEL(lbl_loc_title), "Choose Install Location");
        gtk_label_set_text(GTK_LABEL(lbl_ready_title), "Ready to Install");
        gtk_label_set_text(GTK_LABEL(lbl_ready_desc), "Click Install to configure Zentix PC and launch the suite.");
    }

    update_disk_space(gtk_editable_get_text(GTK_EDITABLE(entry_path)));
}

static void on_lang_dropdown_changed(GtkDropDown* dropdown, GParamSpec* pspec, gpointer user_data) {
    (void)pspec; (void)user_data;
    guint sel = gtk_drop_down_get_selected(dropdown);
    sync_language_with_app((int)sel);
    refresh_wizard_texts();
}

static void on_rtss_download_clicked(GtkButton* btn, gpointer user_data) {
    (void)btn; (void)user_data;
    ShellExecuteA(NULL, "open", "https://share.google/hqQrJYwFQ7WsZdY1f", NULL, NULL, SW_SHOWNORMAL);
}

static void on_path_changed(GtkEditable* editable, gpointer user_data) {
    (void)user_data;
    update_disk_space(gtk_editable_get_text(editable));
}

// تنفيذ التثبيت الفعلي والانتقال للتطبيق
static gboolean finish_install_timeout(gpointer user_data) {
    (void)user_data;
    gtk_window_destroy(GTK_WINDOW(wizard_win));
    if (on_install_complete_cb) {
        on_install_complete_cb(); // فتح التطبيق الرئيسي باللغة الجديدة
    }
    return G_SOURCE_REMOVE;
}

static void on_next_or_install_clicked(GtkButton* btn, gpointer user_data) {
    (void)btn; (void)user_data;
    const char* visible = gtk_stack_get_visible_child_name(GTK_STACK(stack));

    if (g_strcmp0(visible, "page_lang") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "page_comp");
        gtk_widget_set_sensitive(btn_back, TRUE);
    }
    else if (g_strcmp0(visible, "page_comp") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "page_loc");
    }
    else if (g_strcmp0(visible, "page_loc") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "page_install");
    }
    else if (g_strcmp0(visible, "page_install") == 0) {
        // إنشاء مجلد التثبيت وحفظ الإعدادات
        const char* dest = gtk_editable_get_text(GTK_EDITABLE(entry_path));
        CreateDirectoryA(dest, NULL);
        sync_language_with_app(selected_lang_idx);

        gtk_widget_set_sensitive(btn_next, FALSE);
        gtk_widget_set_sensitive(btn_back, FALSE);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(prog_bar), 1.0);
        g_timeout_add(600, finish_install_timeout, NULL);
        return;
    }
    refresh_wizard_texts();
}

static void on_back_clicked(GtkButton* btn, gpointer user_data) {
    (void)btn; (void)user_data;
    const char* visible = gtk_stack_get_visible_child_name(GTK_STACK(stack));

    if (g_strcmp0(visible, "page_install") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "page_loc");
    }
    else if (g_strcmp0(visible, "page_loc") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "page_comp");
    }
    else if (g_strcmp0(visible, "page_comp") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "page_lang");
        gtk_widget_set_sensitive(btn_back, FALSE);
    }
    refresh_wizard_texts();
}

static void on_cancel_clicked(GtkButton* btn, gpointer user_data) {
    (void)btn; (void)user_data;
    gtk_window_destroy(GTK_WINDOW(wizard_win));
}

// الدالة الرئيسية لتشغيل الـ Wizard
void ShowZentixSetupWizard(GtkApplication* app, void (*onComplete)(void)) {
    on_install_complete_cb = onComplete;
    selected_lang_idx = (int)ZentixGetLanguage();

    wizard_win = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(wizard_win), 540, 390);
    gtk_window_set_resizable(GTK_WINDOW(wizard_win), FALSE);

    GtkWidget* root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(wizard_win), root_box);

    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_widget_set_vexpand(stack, TRUE);
    gtk_box_append(GTK_BOX(root_box), stack);

    // --- الصفحة 1: اختيار اللغة ---
    GtkWidget* p1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(p1, 35); gtk_widget_set_margin_end(p1, 35); gtk_widget_set_margin_top(p1, 35);
    lbl_lang_title = gtk_label_new("");
    gtk_widget_add_css_class(lbl_lang_title, "screen-header-title");
    gtk_widget_set_halign(lbl_lang_title, GTK_ALIGN_START);

    lbl_lang_desc = gtk_label_new("");
    gtk_widget_set_halign(lbl_lang_desc, GTK_ALIGN_START);

    const char* languages[] = {
        "English (EN)", "العربية (AR)", "Deutsch (DE)", "Français (FR)",
        "日本語 (JA)", "한국어 (KO)", "Русский (RU)", "Türkçe (TR)", "中文 (ZH)", NULL
    };
    GtkWidget* dropdown = gtk_drop_down_new_from_strings(languages);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), (guint)selected_lang_idx);
    g_signal_connect(dropdown, "notify::selected", G_CALLBACK(on_lang_dropdown_changed), NULL);

    gtk_box_append(GTK_BOX(p1), lbl_lang_title);
    gtk_box_append(GTK_BOX(p1), lbl_lang_desc);
    gtk_box_append(GTK_BOX(p1), dropdown);
    gtk_stack_add_named(GTK_STACK(stack), p1, "page_lang");

    // --- الصفحة 2: المكونات ورابط RTSS ---
    GtkWidget* p2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_margin_start(p2, 35); gtk_widget_set_margin_end(p2, 35); gtk_widget_set_margin_top(p2, 30);
    lbl_comp_title = gtk_label_new("");
    gtk_widget_add_css_class(lbl_comp_title, "screen-header-title");
    gtk_widget_set_halign(lbl_comp_title, GTK_ALIGN_START);

    chk_core = gtk_check_button_new_with_label("Zentix PC");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(chk_core), TRUE);
    gtk_widget_set_sensitive(chk_core, FALSE);

    lbl_rtss_warn = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(lbl_rtss_warn), TRUE);
    gtk_widget_set_halign(lbl_rtss_warn, GTK_ALIGN_START);

    btn_rtss_link = gtk_button_new_with_label("Download RTSS 7.3.7");
    gtk_widget_set_halign(btn_rtss_link, GTK_ALIGN_START);
    g_signal_connect(btn_rtss_link, "clicked", G_CALLBACK(on_rtss_download_clicked), NULL);

    gtk_box_append(GTK_BOX(p2), lbl_comp_title);
    gtk_box_append(GTK_BOX(p2), chk_core);
    gtk_box_append(GTK_BOX(p2), lbl_rtss_warn);
    gtk_box_append(GTK_BOX(p2), btn_rtss_link);
    gtk_stack_add_named(GTK_STACK(stack), p2, "page_comp");

    // --- الصفحة 3: مكان التثبيت والمساحة ---
    GtkWidget* p3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_margin_start(p3, 35); gtk_widget_set_margin_end(p3, 35); gtk_widget_set_margin_top(p3, 30);
    lbl_loc_title = gtk_label_new("");
    gtk_widget_add_css_class(lbl_loc_title, "screen-header-title");
    gtk_widget_set_halign(lbl_loc_title, GTK_ALIGN_START);

    entry_path = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry_path), "C:\\Program Files\\ZentixPC");
    g_signal_connect(entry_path, "changed", G_CALLBACK(on_path_changed), NULL);

    lbl_space_info = gtk_label_new("");
    gtk_widget_set_halign(lbl_space_info, GTK_ALIGN_START);

    gtk_box_append(GTK_BOX(p3), lbl_loc_title);
    gtk_box_append(GTK_BOX(p3), entry_path);
    gtk_box_append(GTK_BOX(p3), lbl_space_info);
    gtk_stack_add_named(GTK_STACK(stack), p3, "page_loc");

    // --- الصفحة 4: زر Install ---
    GtkWidget* p4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    gtk_widget_set_margin_start(p4, 35); gtk_widget_set_margin_end(p4, 35); gtk_widget_set_margin_top(p4, 45);
    lbl_ready_title = gtk_label_new("");
    gtk_widget_add_css_class(lbl_ready_title, "screen-header-title");
    gtk_widget_set_halign(lbl_ready_title, GTK_ALIGN_START);

    lbl_ready_desc = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(lbl_ready_desc), TRUE);
    gtk_widget_set_halign(lbl_ready_desc, GTK_ALIGN_START);

    prog_bar = gtk_progress_bar_new();

    gtk_box_append(GTK_BOX(p4), lbl_ready_title);
    gtk_box_append(GTK_BOX(p4), lbl_ready_desc);
    gtk_box_append(GTK_BOX(p4), prog_bar);
    gtk_stack_add_named(GTK_STACK(stack), p4, "page_install");

    // --- شريط الأزرار السفلي ---
    gtk_box_append(GTK_BOX(root_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    GtkWidget* nav_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(nav_bar, GTK_ALIGN_END);
    gtk_widget_set_margin_end(nav_bar, 20);
    gtk_widget_set_margin_top(nav_bar, 12);
    gtk_widget_set_margin_bottom(nav_bar, 12);

    btn_back = gtk_button_new_with_label("< Back");
    gtk_widget_set_sensitive(btn_back, FALSE);
    btn_next = gtk_button_new_with_label("Next >");
    btn_cancel = gtk_button_new_with_label("Cancel");

    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), NULL);
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_next_or_install_clicked), NULL);
    g_signal_connect(btn_cancel, "clicked", G_CALLBACK(on_cancel_clicked), NULL);

    gtk_box_append(GTK_BOX(nav_bar), btn_back);
    gtk_box_append(GTK_BOX(nav_bar), btn_next);
    gtk_box_append(GTK_BOX(nav_bar), btn_cancel);
    gtk_box_append(GTK_BOX(root_box), nav_bar);

    refresh_wizard_texts();
    gtk_window_present(GTK_WINDOW(wizard_win));
}