#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <windows.h>
#include <mmsystem.h>
#include "zentix.h"        // الهيكل المركزي الموحد للترجمات
#include "HomeScreen.h"    // هيدر شاشة الهوم

#pragma comment(lib, "winmm.lib")

#define NeonRed "#FF2A4B"
#define NeonCyan "#00E5FF"
#define NeonGreen "#00E676"
#define NeonAmber "#FFD600"

// دالة تشغيل مسارات الـ MCI الصوتية
static void play_home_mci_audio(const char* alias, const char* filename) {
    char baseExeDir[MAX_PATH] = "";
    GetModuleFileNameA(NULL, baseExeDir, MAX_PATH);
    char* lastSlash = strrchr(baseExeDir, '\\');
    if (lastSlash) *lastSlash = '\0';
    for (int i = 0; baseExeDir[i] != '\0'; i++) {
        if (baseExeDir[i] == '\\') baseExeDir[i] = '/';
    }

    char fullPath[512];
    char cmd[1024];
    snprintf(fullPath, sizeof(fullPath), "%s/raw/%s", baseExeDir, filename);
    for (int i = 0; fullPath[i] != '\0'; i++) if (fullPath[i] == '/') fullPath[i] = '\\';

    snprintf(cmd, sizeof(cmd), "stop %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
    snprintf(cmd, sizeof(cmd), "close %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
    snprintf(cmd, sizeof(cmd), "open \"%s\" type mpegvideo alias %s", fullPath, alias);
    if (mciSendStringA(cmd, NULL, 0, NULL) == 0) {
        snprintf(cmd, sizeof(cmd), "play %s", alias);
        mciSendStringA(cmd, NULL, 0, NULL);
    }
}

static void load_zabet_css(void) {
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window, background, .background {"
        "  background-color: #090A0F;"
        "  color: #FFFFFF;"
        "}"
        "scrolledwindow, viewport {"
        "  background-color: #090A0F;"
        "}"
        ".desktop-main-container {"
        "  padding: 30px 40px;"
        "}"
        "button.zabet-style-card {"
        "  background-color: #12141D;"
        "  background-image: none;"
        "  border: 1px solid #1E2230;"
        "  border-radius: 20px;"
        "  color: #FFFFFF;"
        "  padding: 0px;"
        "  min-height: 200px;"
        "  box-shadow: 0 10px 25px rgba(0, 0, 0, 0.45);"
        "  transition: all 0.3s ease;"
        "}"
        "button.zabet-style-card:hover {"
        "  border-color: #FF2A4B;"
        "  background-color: #161923;"
        "  transform: translateY(-3px);"
        "}"
        "button.game-mode-button-card {"
        "  background: linear-gradient(135deg, #FF2A4B 0%, #99001C 100%);"
        "  background-image: none;"
        "  background-color: #FF2A4B;"
        "  border-radius: 18px;"
        "  color: #FFFFFF;"
        "  border: 1.5px solid rgba(255, 255, 255, 0.2);"
        "  min-height: 105px;"
        "  box-shadow: 0 8px 25px rgba(255, 42, 75, 0.4);"
        "  transition: all 0.25s ease;"
        "}"
        "button.game-mode-button-card:hover {"
        "  background-color: #FF4D6A;"
        "  box-shadow: 0 12px 35px rgba(255, 42, 75, 0.6);"
        "}"
        ".app-title-label {"
        "  font-weight: 900;"
        "  font-size: 28px;"
        "  color: #FFFFFF;"
        "}"
        ".app-subtitle-label {"
        "  font-size: 12.5px;"
        "  color: #808080;"
        "  font-weight: bold;"
        "}"
        ".card-title-label {"
        "  font-weight: bold;"
        "  font-size: 18px;"
        "  color: #FFFFFF;"
        "}"
        ".card-desc-label {"
        "  font-size: 13px;"
        "  color: #AAAAAA;"
        "}"
        ".game-btn-label {"
        "  font-weight: 900;"
        "  font-size: 24px;"
        "  letter-spacing: 4px;"
        "  color: #FFFFFF;"
        "}"
        ".bolt-icon-box {"
        "  background-color: rgba(255, 42, 75, 0.18);"
        "  border: 1.5px solid #FF2A4B;"
        "  border-radius: 14px;"
        "  font-size: 24px;"
        "  padding: 8px 14px;"
        "}"
        "button.settings-icon-btn {"
        "  background-color: #12141D;"
        "  background-image: none;"
        "  border: 1px solid #1E2230;"
        "  border-radius: 14px;"
        "  color: #FFFFFF;"
        "  font-size: 20px;"
        "  box-shadow: 0 4px 12px rgba(0,0,0,0.3);"
        "}"
        "button.settings-icon-btn:hover {"
        "  border-color: #FF2A4B;"
        "  background-color: #181b26;"
        "}"
        ".accent-icon-box {"
        "  background-color: rgba(255, 255, 255, 0.06);"
        "  border: 1px solid rgba(255, 255, 255, 0.1);"
        "  border-radius: 14px;"
        "  font-size: 20px;"
        "  padding: 10px 14px;"
        "}"
        ".toast-notification {"
        "  background-color: rgba(18, 20, 29, 0.96);"
        "  border: 1.5px solid #FF2A4B;"
        "  color: #FFFFFF;"
        "  border-radius: 14px;"
        "  padding: 14px 26px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  box-shadow: 0 12px 35px rgba(0,0,0,0.7);"
        "}"
    );

    GdkDisplay* display = gdk_display_get_default();
    if (display != NULL) {
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }
}

static void on_card_clicked_cb(GtkButton* btn, gpointer user_data) {
    void (*callback)(void) = (void(*)(void))user_data;
    if (callback) {
        callback();
    }
    (void)btn;
}

typedef struct {
    void (*callback)(void);
} GameModeTransitionContext;

static gboolean execute_gamemode_transition_cb(gpointer user_data) {
    GameModeTransitionContext* ctx = (GameModeTransitionContext*)user_data;
    if (ctx) {
        if (ctx->callback) {
            ctx->callback();
        }
        g_free(ctx);
    }
    return G_SOURCE_REMOVE;
}

static void on_gamemode_clicked_cb(GtkButton* btn, gpointer user_data) {
    void (*callback)(void) = (void(*)(void))user_data;
    play_home_mci_audio("powerclickalias", "ps3_power_click.mp3");

    if (callback) {
        GameModeTransitionContext* ctx = g_new0(GameModeTransitionContext, 1);
        ctx->callback = callback;
        g_timeout_add(750, execute_gamemode_transition_cb, ctx);
    }
    (void)btn;
}

typedef struct {
    GtkWidget* revealer;
    guint timer_id;
} ToastContext;

static gboolean hide_toast_cb(gpointer user_data) {
    ToastContext* ctx = (ToastContext*)user_data;
    if (ctx && ctx->revealer) {
        gtk_revealer_set_reveal_child(GTK_REVEALER(ctx->revealer), FALSE);
    }
    g_free(ctx);
    return G_SOURCE_REMOVE;
}

static gboolean on_key_pressed_cb(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    GtkWidget* window = GTK_WIDGET(user_data);
    if ((state & GDK_ALT_MASK) && (keyval == GDK_KEY_Return || keyval == GDK_KEY_ISO_Enter || keyval == GDK_KEY_KP_Enter)) {
        if (window) {
            if (gtk_window_is_maximized(GTK_WINDOW(window))) {
                gtk_window_unmaximize(GTK_WINDOW(window));
            }
            else {
                gtk_window_maximize(GTK_WINDOW(window));
            }
        }
        return TRUE;
    }
    (void)controller;
    (void)keycode;
    return FALSE;
}

static GtkWidget* TopAppBarSection(void (*onSettingsClick)(void)) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();

    GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_valign(row, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(row, 10);
    gtk_widget_set_margin_bottom(row, 10);

    GtkWidget* left_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_valign(left_box, GTK_ALIGN_CENTER);

    GtkWidget* icon_lbl = gtk_label_new("⚡");
    gtk_widget_add_css_class(icon_lbl, "bolt-icon-box");
    gtk_box_append(GTK_BOX(left_box), icon_lbl);

    GtkWidget* text_col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    GtkWidget* app_name_label = gtk_label_new(t->home_app_name);
    gtk_widget_set_halign(app_name_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(app_name_label, "app-title-label");

    GtkWidget* app_subtitle_label = gtk_label_new(t->home_subtitle);
    gtk_widget_set_halign(app_subtitle_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(app_subtitle_label, "app-subtitle-label");

    gtk_box_append(GTK_BOX(text_col), app_name_label);
    gtk_box_append(GTK_BOX(text_col), app_subtitle_label);
    gtk_box_append(GTK_BOX(left_box), text_col);

    gtk_box_append(GTK_BOX(row), left_box);

    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(row), spacer);

    GtkWidget* settings_btn = gtk_button_new_with_label("⚙");
    gtk_widget_set_size_request(settings_btn, 48, 48);
    gtk_widget_add_css_class(settings_btn, "settings-icon-btn");

    if (onSettingsClick) {
        g_signal_connect(settings_btn, "clicked", G_CALLBACK(on_card_clicked_cb), onSettingsClick);
    }
    gtk_box_append(GTK_BOX(row), settings_btn);

    return row;
}

static GtkWidget* ZabetStyleCard(
    const char* title,
    const char* description,
    const char* accentColor,
    const char* iconEmoji,
    void (*onClick)(void)
) {
    GtkWidget* card = gtk_button_new();
    gtk_widget_add_css_class(card, "zabet-style-card");
    gtk_widget_set_hexpand(card, TRUE);
    gtk_widget_set_vexpand(card, FALSE);

    GtkWidget* content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_margin_start(content_box, 24);
    gtk_widget_set_margin_end(content_box, 24);
    gtk_widget_set_margin_top(content_box, 22);
    gtk_widget_set_margin_bottom(content_box, 22);
    gtk_widget_set_vexpand(content_box, FALSE);
    gtk_widget_set_hexpand(content_box, TRUE);

    GtkWidget* header_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_hexpand(header_row, TRUE);

    GtkWidget* icon_lbl = gtk_label_new(iconEmoji);
    gtk_widget_add_css_class(icon_lbl, "accent-icon-box");
    gtk_box_append(GTK_BOX(header_row), icon_lbl);

    GtkWidget* title_label = gtk_label_new(title);
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(title_label, TRUE);
    gtk_widget_add_css_class(title_label, "card-title-label");
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(header_row), title_label);

    GtkWidget* chevron = gtk_label_new("➔");
    gtk_box_append(GTK_BOX(header_row), chevron);

    gtk_box_append(GTK_BOX(content_box), header_row);

    GtkWidget* desc_label = gtk_label_new(description);
    gtk_widget_set_halign(desc_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(desc_label, "card-desc-label");
    gtk_label_set_wrap(GTK_LABEL(desc_label), TRUE);
    gtk_label_set_lines(GTK_LABEL(desc_label), 3);
    gtk_label_set_ellipsize(GTK_LABEL(desc_label), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(content_box), desc_label);

    gtk_button_set_child(GTK_BUTTON(card), content_box);

    if (onClick) {
        g_signal_connect(card, "clicked", G_CALLBACK(on_card_clicked_cb), onClick);
    }

    (void)accentColor;
    return card;
}

static GtkWidget* GameModeButton(void (*onClick)(void)) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();

    GtkWidget* card = gtk_button_new();
    gtk_widget_add_css_class(card, "game-mode-button-card");
    gtk_widget_set_hexpand(card, TRUE);

    GtkWidget* center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_vexpand(center_box, FALSE);
    gtk_widget_set_hexpand(center_box, TRUE);

    GtkWidget* power_lbl = gtk_label_new("⚡");
    gtk_box_append(GTK_BOX(center_box), power_lbl);

    GtkWidget* btn_label = gtk_label_new(t->home_game_mode_btn);
    gtk_widget_add_css_class(btn_label, "game-btn-label");
    gtk_box_append(GTK_BOX(center_box), btn_label);

    gtk_button_set_child(GTK_BUTTON(card), center_box);

    if (onClick) {
        g_signal_connect(card, "clicked", G_CALLBACK(on_gamemode_clicked_cb), onClick);
    }

    return card;
}

GtkWidget* HomeScreen(HomeScreenCallbacks callbacks) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();
    load_zabet_css();

    GtkWidget* overlay = gtk_overlay_new();
    gtk_widget_set_vexpand(overlay, TRUE);
    gtk_widget_set_hexpand(overlay, TRUE);

    GtkWidget* scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);

    GtkWidget* desktop_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(desktop_wrapper, "desktop-main-container");
    gtk_widget_set_hexpand(desktop_wrapper, TRUE);
    gtk_widget_set_vexpand(desktop_wrapper, FALSE);
    gtk_widget_set_halign(desktop_wrapper, GTK_ALIGN_FILL);
    gtk_widget_set_valign(desktop_wrapper, GTK_ALIGN_START);

    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 24);
    gtk_widget_set_hexpand(main_box, TRUE);

    GtkWidget* top_bar = TopAppBarSection(callbacks.onNavigateToSettings);
    gtk_box_append(GTK_BOX(main_box), top_bar);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator, 4);
    gtk_widget_set_margin_bottom(separator, 4);
    gtk_box_append(GTK_BOX(main_box), separator);

    GtkWidget* row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_hexpand(row1, TRUE);

    GtkWidget* card_msi = ZabetStyleCard(
        t->home_card_fps_title,
        t->home_card_fps_desc,
        NeonRed,
        "📊",
        callbacks.onNavigateToMSISuite
    );
    gtk_widget_set_hexpand(card_msi, TRUE);

    GtkWidget* card_bench = ZabetStyleCard(
        t->home_card_sys_title,
        t->home_card_sys_desc,
        NeonCyan,
        "📋",
        callbacks.onNavigateToBenchmark
    );
    gtk_widget_set_hexpand(card_bench, TRUE);

    gtk_box_append(GTK_BOX(row1), card_msi);
    gtk_box_append(GTK_BOX(row1), card_bench);
    gtk_box_append(GTK_BOX(main_box), row1);

    GtkWidget* game_mode_btn = GameModeButton(callbacks.onNavigateToGameMode);
    gtk_box_append(GTK_BOX(main_box), game_mode_btn);

    gtk_box_append(GTK_BOX(desktop_wrapper), main_box);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), desktop_wrapper);

    gtk_overlay_set_child(GTK_OVERLAY(overlay), scrolled_window);

    GtkWidget* toast_revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(toast_revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
    gtk_revealer_set_transition_duration(toast_revealer, 500);
    gtk_widget_set_halign(toast_revealer, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(toast_revealer, GTK_ALIGN_END);
    gtk_widget_set_margin_bottom(toast_revealer, 35);

    GtkWidget* toast_label = gtk_label_new(t->home_toast_tip);
    gtk_widget_add_css_class(toast_label, "toast-notification");
    gtk_revealer_set_child(GTK_REVEALER(toast_revealer), toast_label);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), toast_revealer);

    gtk_revealer_set_reveal_child(GTK_REVEALER(toast_revealer), TRUE);
    ToastContext* t_ctx = g_new0(ToastContext, 1);
    t_ctx->revealer = toast_revealer;
    g_timeout_add(4000, hide_toast_cb, t_ctx);

    GtkWidget* native_window = gtk_widget_get_ancestor(overlay, GTK_TYPE_WINDOW);
    if (native_window) {
        gtk_window_maximize(GTK_WINDOW(native_window));
        GtkEventController* key_controller = gtk_event_controller_key_new();
        g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed_cb), native_window);
        gtk_widget_add_controller(native_window, key_controller);
    }

    return overlay;
}