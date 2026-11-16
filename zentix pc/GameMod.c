#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <windows.h>
#include <gdk/win32/gdkwin32.h>
#include <shellapi.h>
#include <shlobj.h>
#include <xinput.h>
#include <mmsystem.h>
#include <urlmon.h>
#include <wininet.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include "zentix.h"
#include "GameMod.h" 
#include "zentix.h"
#include "GameMod.h"
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

typedef struct {
    char title[256];
    char exePath[512];
    char posterPath[MAX_PATH];
    gboolean is_loading;
} ConsoleAppGame;

static ConsoleAppGame customGamesList[100];
static ConsoleAppGame filteredGamesList[100];
static int customGamesCount = 0;
static int filteredGamesCount = 0;
static int selectedGameIndex = 0;
static char baseExeDir[MAX_PATH] = "";

static GtkWidget* global_carousel_box = NULL;
static GtkWidget* global_active_title_lbl = NULL;
static GtkWidget* global_counter_lbl = NULL;
static GtkWidget* global_clock_lbl = NULL;
static GtkWidget* global_root_widget = NULL;
static GtkWidget* global_search_entry = NULL;
static GtkWidget* global_bg_picture = NULL;
static char current_bg_path[512] = "";
static gboolean is_search_visible = FALSE;
static void (*global_on_navigate_back)(void) = NULL;

// دالة لتحديد المسار المطلق بجوار الـ .exe مباشرة
static void init_base_directory(void) {
    if (baseExeDir[0] == '\0') {
        GetModuleFileNameA(NULL, baseExeDir, MAX_PATH);
        char* lastSlash = strrchr(baseExeDir, '\\');
        if (lastSlash) *lastSlash = '\0';
        for (int i = 0; baseExeDir[i] != '\0'; i++) {
            if (baseExeDir[i] == '\\') baseExeDir[i] = '/';
        }
    }
}

// ============================================================================
// دوال الحفظ والقراءة الدائمة للألعاب (Persistence Layer)
// ============================================================================
static void get_games_config_path(char* path_out, DWORD max_len) {
    init_base_directory();
    char win_dir[MAX_PATH];
    strcpy_s(win_dir, sizeof(win_dir), baseExeDir);
    for (int i = 0; win_dir[i] != '\0'; i++) if (win_dir[i] == '/') win_dir[i] = '\\';
    snprintf(path_out, max_len, "%s\\zentix_games.txt", win_dir);
}

static void save_games_to_disk(void) {
    char path[MAX_PATH];
    get_games_config_path(path, sizeof(path));

    FILE* f = fopen(path, "w");
    if (f) {
        for (int i = 0; i < customGamesCount; i++) {
            fprintf(f, "%s|%s|%s\n", customGamesList[i].title, customGamesList[i].exePath, customGamesList[i].posterPath);
        }
        fclose(f);
    }
}

static void load_games_from_disk(void) {
    char path[MAX_PATH];
    get_games_config_path(path, sizeof(path));

    FILE* f = fopen(path, "r");
    if (f) {
        customGamesCount = 0;
        char line[1024];
        while (fgets(line, sizeof(line), f) && customGamesCount < 100) {
            line[strcspn(line, "\r\n")] = 0;
            if (strlen(line) > 0) {
                char* title = strtok(line, "|");
                char* exePath = strtok(NULL, "|");
                char* posterPath = strtok(NULL, "|");

                if (title && exePath) {
                    strncpy_s(customGamesList[customGamesCount].title, sizeof(customGamesList[customGamesCount].title), title, _TRUNCATE);
                    strncpy_s(customGamesList[customGamesCount].exePath, sizeof(customGamesList[customGamesCount].exePath), exePath, _TRUNCATE);
                    if (posterPath) {
                        strncpy_s(customGamesList[customGamesCount].posterPath, sizeof(customGamesList[customGamesCount].posterPath), posterPath, _TRUNCATE);
                    }
                    else {
                        customGamesList[customGamesCount].posterPath[0] = '\0';
                    }
                    customGamesList[customGamesCount].is_loading = FALSE;
                    customGamesCount++;
                }
            }
        }
        fclose(f);
    }
}

// استخراج أيقونة JUMBO فائقة الدقة (256x256) مع تأمين الذاكرة
static GdkTexture* get_exe_icon_texture(const char* exe_path) {
    if (!exe_path || exe_path[0] == '\0') return NULL;
    wchar_t wpath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, exe_path, -1, wpath, MAX_PATH);

    HICON hIcon = NULL;
    HMODULE hShell32 = LoadLibraryA("shell32.dll");
    if (hShell32) {
        typedef HRESULT(WINAPI* PFSHGetImageList)(int, REFIID, void**);
        PFSHGetImageList pSHGetImageList = (PFSHGetImageList)GetProcAddress(hShell32, "SHGetImageList");
        if (pSHGetImageList) {
            GUID IID_IImageList_Local = { 0x46eb5926, 0x582e, 0x4017,{ 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x09, 0x50 } };
            HIMAGELIST himl = NULL;
            if (SUCCEEDED(pSHGetImageList(4, &IID_IImageList_Local, (void**)&himl)) && himl) {
                SHFILEINFOW sfi = { 0 };
                DWORD_PTR res = SHGetFileInfoW(wpath, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
                if (res) {
                    typedef HICON(WINAPI* PFImageList_GetIcon)(HIMAGELIST, int, UINT);
                    PFImageList_GetIcon pImageList_GetIcon = (PFImageList_GetIcon)GetProcAddress(hShell32, "ImageList_GetIcon");
                    if (pImageList_GetIcon) {
                        hIcon = pImageList_GetIcon(himl, sfi.iIcon, 0);
                    }
                }
            }
        }
        FreeLibrary(hShell32);
    }

    if (!hIcon) {
        SHFILEINFOW sfi = { 0 };
        if (SHGetFileInfoW(wpath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON)) {
            hIcon = sfi.hIcon;
        }
    }

    if (!hIcon) return NULL;

    ICONINFO ii;
    if (!GetIconInfo(hIcon, &ii)) {
        DestroyIcon(hIcon);
        return NULL;
    }

    BITMAP bmp;
    GetObject(ii.hbmColor, sizeof(bmp), &bmp);

    int width = bmp.bmWidth > 0 ? bmp.bmWidth : 256;
    int height = bmp.bmHeight > 0 ? bmp.bmHeight : 256;
    int stride = width * 4;
    guchar* pixels = g_malloc0(height * stride);
    if (!pixels) {
        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        DestroyIcon(hIcon);
        return NULL;
    }

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, ii.hbmColor);
    GetDIBits(hMemDC, ii.hbmColor, 0, height, pixels, &bmi, DIB_RGB_COLORS);
    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);

    GBytes* bytes = g_bytes_new_take(pixels, height * stride);
    GdkTexture* texture = gdk_memory_texture_new(width, height, GDK_MEMORY_B8G8R8A8, bytes, stride);
    g_bytes_unref(bytes);

    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);
    DestroyIcon(hIcon);

    return texture;
}

static void fetch_game_poster(const char* gameTitle, const char* exePath, char* outPosterPath, size_t maxLen) {
    if (!gameTitle || !exePath) return;
    init_base_directory();
    char posterDir[512];
    snprintf(posterDir, sizeof(posterDir), "%s/raw/posters", baseExeDir);

    char winPosterDir[512];
    strcpy_s(winPosterDir, sizeof(winPosterDir), posterDir);
    for (int i = 0; winPosterDir[i] != '\0'; i++) if (winPosterDir[i] == '/') winPosterDir[i] = '\\';
    CreateDirectoryA(winPosterDir, NULL);

    char safeName[256] = "";
    for (int i = 0; gameTitle[i] != '\0' && i < 200; i++) {
        char c = gameTitle[i];
        if (isalnum((unsigned char)c) || c == '_' || c == '-') {
            char temp[2] = { c, '\0' };
            strcat_s(safeName, sizeof(safeName), temp);
        }
        else {
            strcat_s(safeName, sizeof(safeName), "_");
        }
    }

    char jpgPath[512];
    snprintf(jpgPath, sizeof(jpgPath), "%s/%s_banner.jpg", posterDir, safeName);
    char winJpgPath[512];
    strcpy_s(winJpgPath, sizeof(winJpgPath), jpgPath);
    for (int i = 0; winJpgPath[i] != '\0'; i++) if (winJpgPath[i] == '/') winJpgPath[i] = '\\';

    struct stat st;
    if (stat(winJpgPath, &st) == 0 && st.st_size > 2048) {
        strncpy_s(outPosterPath, maxLen, jpgPath, _TRUNCATE);
        return;
    }

    char dir[512];
    strcpy_s(dir, sizeof(dir), exePath);
    for (int i = 0; dir[i] != '\0'; i++) if (dir[i] == '\\') dir[i] = '/';
    char* slash = strrchr(dir, '/');
    if (slash) *slash = '\0';

    char appIdStr[32] = "";
    char appidPath[512];
    snprintf(appidPath, sizeof(appidPath), "%s/steam_appid.txt", dir);
    char winAppidPath[512];
    strcpy_s(winAppidPath, sizeof(winAppidPath), appidPath);
    for (int i = 0; winAppidPath[i] != '\0'; i++) if (winAppidPath[i] == '/') winAppidPath[i] = '\\';

    FILE* f = fopen(winAppidPath, "r");
    if (f) {
        if (fgets(appIdStr, sizeof(appIdStr), f)) {
            appIdStr[strcspn(appIdStr, "\r\n")] = 0;
        }
        fclose(f);
    }

    if (strlen(appIdStr) == 0) {
        char searchUrl[1024];
        snprintf(searchUrl, sizeof(searchUrl), "https://store.steampowered.com/api/storesearch/?term=%s&l=english&cc=US", gameTitle);

        HINTERNET hNet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hNet) {
            HINTERNET hUrl = InternetOpenUrlA(hNet, searchUrl, NULL, 0, INTERNET_FLAG_RELOAD, 0);
            if (hUrl) {
                char response[16384] = { 0 };
                DWORD bytesRead = 0;
                InternetReadFile(hUrl, response, sizeof(response) - 1, &bytesRead);
                InternetCloseHandle(hUrl);

                char* idPtr = strstr(response, "\"id\":");
                if (idPtr) {
                    int foundId = 0;
                    if (sscanf(idPtr, "\"id\":%d", &foundId) == 1 && foundId > 0) {
                        snprintf(appIdStr, sizeof(appIdStr), "%d", foundId);
                    }
                }
            }
            InternetCloseHandle(hNet);
        }
    }

    int downloaded = 0;
    if (strlen(appIdStr) > 0) {
        char url[512];
        snprintf(url, sizeof(url), "https://cdn.cloudflare.steamstatic.com/steam/apps/%s/library_600x900.jpg", appIdStr);

        HRESULT hr = URLDownloadToFileA(NULL, url, winJpgPath, 0, NULL);
        if (hr == S_OK && stat(winJpgPath, &st) == 0 && st.st_size > 2048) {
            downloaded = 1;
        }
        else {
            remove(winJpgPath);
        }
    }

    if (downloaded) {
        strncpy_s(outPosterPath, maxLen, jpgPath, _TRUNCATE);
    }
    else {
        outPosterPath[0] = '\0';
    }
}

typedef struct {
    char path[512];
    char clean_title[256];
} GameThreadData;

static void rebuild_carousel_ui(void);
static void update_bottom_info(void);
static void update_dynamic_background(void);

static gpointer background_load_game_thread(gpointer data) {
    GameThreadData* gdata = (GameThreadData*)data;
    if (gdata && customGamesCount < 100) {
        int idx = customGamesCount;
        strncpy_s(customGamesList[idx].title, sizeof(customGamesList[idx].title), gdata->clean_title, _TRUNCATE);
        strncpy_s(customGamesList[idx].exePath, sizeof(customGamesList[idx].exePath), gdata->path, _TRUNCATE);
        customGamesList[idx].is_loading = TRUE;

        fetch_game_poster(gdata->clean_title, gdata->path, customGamesList[idx].posterPath, sizeof(customGamesList[idx].posterPath));
        customGamesList[idx].is_loading = FALSE;

        customGamesCount++;

        save_games_to_disk();

        g_idle_add((GSourceFunc)rebuild_carousel_ui, NULL);
        g_idle_add((GSourceFunc)update_bottom_info, NULL);
    }
    if (gdata) g_free(gdata);
    return NULL;
}

static void play_mci_audio(const char* alias, const char* filename, gboolean repeat) {
    if (!alias || !filename) return;
    init_base_directory();
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
        snprintf(cmd, sizeof(cmd), "play %s%s", alias, repeat ? " repeat" : "");
        mciSendStringA(cmd, NULL, 0, NULL);
    }
}

static void stop_mci_audio(const char* alias) {
    if (!alias) return;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "stop %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
    snprintf(cmd, sizeof(cmd), "close %s", alias);
    mciSendStringA(cmd, NULL, 0, NULL);
}

static void play_click_sound(void) {
    play_mci_audio("clickalias", "ps3_click.mp3", FALSE);
}

static char* my_strcasestr(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

static void update_filtered_games(const char* query) {
    filteredGamesCount = 0;
    for (int i = 0; i < customGamesCount; i++) {
        if (!query || strlen(query) == 0 || my_strcasestr(customGamesList[i].title, query)) {
            filteredGamesList[filteredGamesCount++] = customGamesList[i];
        }
    }
    if (selectedGameIndex >= filteredGamesCount) {
        selectedGameIndex = filteredGamesCount > 0 ? filteredGamesCount - 1 : 0;
    }
}

static void update_bottom_info(void) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();
    if (!global_active_title_lbl || !GTK_IS_LABEL(global_active_title_lbl)) return;
    if (filteredGamesCount > 0 && selectedGameIndex >= 0 && selectedGameIndex < filteredGamesCount) {
        gtk_label_set_text(GTK_LABEL(global_active_title_lbl), filteredGamesList[selectedGameIndex].title);

        if (global_counter_lbl && GTK_IS_LABEL(global_counter_lbl)) {
            char counterStr[32];
            snprintf(counterStr, sizeof(counterStr), "[ %d / %d ]", selectedGameIndex + 1, filteredGamesCount);
            gtk_label_set_text(GTK_LABEL(global_counter_lbl), counterStr);
        }

        char safeName[256] = "";
        const char* title = filteredGamesList[selectedGameIndex].title;
        for (int i = 0; title[i] != '\0' && i < 200; i++) {
            char c = title[i];
            if (isalnum((unsigned char)c) || c == '_' || c == '-') {
                char temp[2] = { c, '\0' };
                strcat_s(safeName, sizeof(safeName), temp);
            }
            else {
                strcat_s(safeName, sizeof(safeName), "_");
            }
        }
        char audioFile[512];
        snprintf(audioFile, sizeof(audioFile), "audio/%s.mp3", safeName);
        char winAudioPath[512];
        snprintf(winAudioPath, sizeof(winAudioPath), "%s/raw/%s", baseExeDir, audioFile);
        for (int i = 0; winAudioPath[i] != '\0'; i++) if (winAudioPath[i] == '/') winAudioPath[i] = '\\';

        struct stat st;
        if (stat(winAudioPath, &st) == 0 && st.st_size > 1024) {
            play_mci_audio("gamesoundtrack", audioFile, TRUE);
        }
        else {
            stop_mci_audio("gamesoundtrack");
        }
    }
    else {
        gtk_label_set_text(GTK_LABEL(global_active_title_lbl), customGamesCount > 0 ? t->gm_no_matching : t->gm_no_games);
        if (global_counter_lbl && GTK_IS_LABEL(global_counter_lbl)) {
            gtk_label_set_text(GTK_LABEL(global_counter_lbl), "[ 0 / 0 ]");
        }
        stop_mci_audio("gamesoundtrack");
    }
    update_dynamic_background();
}

static void update_dynamic_background(void) {
    if (!global_bg_picture || !GTK_IS_PICTURE(global_bg_picture)) return;

    if (filteredGamesCount > 0 && selectedGameIndex >= 0 && selectedGameIndex < filteredGamesCount) {
        char winPosterPath[512];
        strcpy_s(winPosterPath, sizeof(winPosterPath), filteredGamesList[selectedGameIndex].posterPath);
        for (int j = 0; winPosterPath[j] != '\0'; j++) if (winPosterPath[j] == '/') winPosterPath[j] = '\\';

        if (filteredGamesList[selectedGameIndex].posterPath[0] != '\0' && GetFileAttributesA(winPosterPath) != INVALID_FILE_ATTRIBUTES) {
            if (strcmp(current_bg_path, winPosterPath) != 0) {
                strcpy_s(current_bg_path, sizeof(current_bg_path), winPosterPath);
                gtk_picture_set_filename(GTK_PICTURE(global_bg_picture), winPosterPath);
            }
            return;
        }
    }

    if (strcmp(current_bg_path, "DEFAULT_BG") != 0) {
        strcpy_s(current_bg_path, sizeof(current_bg_path), "DEFAULT_BG");
        gtk_picture_set_filename(GTK_PICTURE(global_bg_picture), NULL);
    }
}

static void on_exit_clicked(GtkButton* btn, gpointer user_data) {
    stop_mci_audio("bgmusic");
    stop_mci_audio("startupsound");
    stop_mci_audio("clickalias");
    stop_mci_audio("gamesoundtrack");
    current_bg_path[0] = '\0';
    if (global_root_widget && GTK_IS_WIDGET(global_root_widget)) {
        GtkRoot* root = gtk_widget_get_root(global_root_widget);
        if (root && GTK_IS_WINDOW(root)) {
            gtk_window_unfullscreen(GTK_WINDOW(root));
        }
    }
    if (global_on_navigate_back) {
        global_on_navigate_back();
    }
    (void)btn;
    (void)user_data;
}

static void on_file_dialog_response(GObject* source, GAsyncResult* res, gpointer user_data) {
    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GFile* file = gtk_file_dialog_open_finish(dialog, res, NULL);
    if (file) {
        char* path = g_file_get_path(file);
        if (path) {
            char* filename = strrchr(path, '\\');
            if (filename) filename++; else filename = path;

            char clean_title[256];
            strncpy_s(clean_title, sizeof(clean_title), filename, _TRUNCATE);
            char* ext = strrchr(clean_title, '.');
            if (ext) *ext = '\0';

            GameThreadData* gdata = g_new0(GameThreadData, 1);
            if (gdata) {
                strncpy_s(gdata->path, sizeof(gdata->path), path, _TRUNCATE);
                strncpy_s(gdata->clean_title, sizeof(gdata->clean_title), clean_title, _TRUNCATE);
                g_thread_new("game_load_thread", background_load_game_thread, gdata);
            }

            g_free(path);
        }
        g_object_unref(file);
    }
    (void)user_data;
}

static void on_add_game_clicked(GtkButton* btn, gpointer user_data) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();
    play_click_sound();
    GtkWindow* parent_window = NULL;
    if (global_root_widget && GTK_IS_WIDGET(global_root_widget)) {
        GtkRoot* root = gtk_widget_get_root(global_root_widget);
        if (root && GTK_IS_WINDOW(root)) parent_window = GTK_WINDOW(root);
    }

    GtkFileDialog* dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, t->gm_dlg_select_exe);

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, t->gm_filter_exe);
    gtk_file_filter_add_pattern(filter, "*.exe");
    GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

    gtk_file_dialog_open(dialog, parent_window, NULL, (GAsyncReadyCallback)on_file_dialog_response, NULL);
    (void)btn;
    (void)user_data;
}

static void on_custom_poster_dialog_response(GObject* source, GAsyncResult* res, gpointer user_data) {
    GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
    GFile* file = gtk_file_dialog_open_finish(dialog, res, NULL);
    if (file) {
        char* srcPath = g_file_get_path(file);
        if (srcPath && filteredGamesCount > 0 && selectedGameIndex >= 0 && selectedGameIndex < filteredGamesCount) {
            init_base_directory();
            char posterDir[512];
            snprintf(posterDir, sizeof(posterDir), "%s/raw/posters", baseExeDir);

            char winPosterDir[512];
            strcpy_s(winPosterDir, sizeof(winPosterDir), posterDir);
            for (int i = 0; winPosterDir[i] != '\0'; i++) if (winPosterDir[i] == '/') winPosterDir[i] = '\\';
            CreateDirectoryA(winPosterDir, NULL);

            char safeName[256] = "";
            const char* title = filteredGamesList[selectedGameIndex].title;
            for (int i = 0; title[i] != '\0' && i < 200; i++) {
                char c = title[i];
                if (isalnum((unsigned char)c) || c == '_' || c == '-') {
                    char temp[2] = { c, '\0' };
                    strcat_s(safeName, sizeof(safeName), temp);
                }
                else {
                    strcat_s(safeName, sizeof(safeName), "_");
                }
            }

            char destPath[512];
            snprintf(destPath, sizeof(destPath), "%s/%s_custom.jpg", posterDir, safeName);

            char winSrc[512], winDest[512];
            strcpy_s(winSrc, sizeof(winSrc), srcPath);
            strcpy_s(winDest, sizeof(winDest), destPath);
            for (int i = 0; winSrc[i] != '\0'; i++) if (winSrc[i] == '/') winSrc[i] = '\\';
            for (int i = 0; winDest[i] != '\0'; i++) if (winDest[i] == '/') winDest[i] = '\\';

            CopyFileA(winSrc, winDest, FALSE);
            strncpy_s(filteredGamesList[selectedGameIndex].posterPath, sizeof(filteredGamesList[selectedGameIndex].posterPath), destPath, _TRUNCATE);

            for (int i = 0; i < customGamesCount; i++) {
                if (strcmp(customGamesList[i].exePath, filteredGamesList[selectedGameIndex].exePath) == 0) {
                    strncpy_s(customGamesList[i].posterPath, sizeof(customGamesList[i].posterPath), destPath, _TRUNCATE);
                    break;
                }
            }

            save_games_to_disk();
            current_bg_path[0] = '\0';
            rebuild_carousel_ui();
            update_bottom_info();
            g_free(srcPath);
        }
        g_object_unref(file);
    }
    (void)user_data;
}

static void on_change_custom_poster_clicked(GtkWidget* item, gpointer user_data) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();
    play_click_sound();
    GtkWindow* parent_window = NULL;
    if (global_root_widget && GTK_IS_WIDGET(global_root_widget)) {
        GtkRoot* root = gtk_widget_get_root(global_root_widget);
        if (root && GTK_IS_WINDOW(root)) parent_window = GTK_WINDOW(root);
    }

    GtkFileDialog* dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, t->gm_dlg_select_poster);

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, t->gm_filter_images);
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.png");
    GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, filter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));

    gtk_file_dialog_open(dialog, parent_window, NULL, (GAsyncReadyCallback)on_custom_poster_dialog_response, NULL);
    (void)item;
    (void)user_data;
}

static void on_open_game_folder(GtkWidget* item, gpointer user_data) {
    play_click_sound();
    if (filteredGamesCount > 0 && selectedGameIndex >= 0 && selectedGameIndex < filteredGamesCount) {
        char dirPath[512];
        strcpy_s(dirPath, sizeof(dirPath), filteredGamesList[selectedGameIndex].exePath);
        for (int i = 0; dirPath[i] != '\0'; i++) if (dirPath[i] == '/') dirPath[i] = '\\';
        char* lastSlash = strrchr(dirPath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            ShellExecuteA(NULL, "explore", dirPath, NULL, NULL, SW_SHOWNORMAL);
        }
    }
    (void)item;
    (void)user_data;
}

static void on_game_card_clicked(GtkButton* btn, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    if (index >= 0 && index < filteredGamesCount) {
        selectedGameIndex = index;
        play_click_sound();
        rebuild_carousel_ui();
        update_bottom_info();
    }
    (void)btn;
}

static void on_prev_clicked(GtkButton* btn, gpointer user_data) {
    if (filteredGamesCount > 0) {
        selectedGameIndex = (selectedGameIndex - 1 + filteredGamesCount) % filteredGamesCount;
        play_click_sound();
        rebuild_carousel_ui();
        update_bottom_info();
    }
    (void)btn;
    (void)user_data;
}

static void on_next_clicked(GtkButton* btn, gpointer user_data) {
    if (filteredGamesCount > 0) {
        selectedGameIndex = (selectedGameIndex + 1) % filteredGamesCount;
        play_click_sound();
        rebuild_carousel_ui();
        update_bottom_info();
    }
    (void)btn;
    (void)user_data;
}

static void on_play_game_clicked(GtkButton* btn, gpointer user_data) {
    play_click_sound();
    stop_mci_audio("bgmusic");
    stop_mci_audio("startupsound");
    stop_mci_audio("gamesoundtrack");

    if (filteredGamesCount > 0 && selectedGameIndex >= 0 && selectedGameIndex < filteredGamesCount) {
        char winExe[512];
        strcpy_s(winExe, sizeof(winExe), filteredGamesList[selectedGameIndex].exePath);
        for (int i = 0; winExe[i] != '\0'; i++) if (winExe[i] == '/') winExe[i] = '\\';
        ShellExecuteA(NULL, "open", winExe, NULL, NULL, SW_SHOWNORMAL);
    }
    (void)btn;
    (void)user_data;
}

static void on_remove_selected_game(GtkWidget* item, gpointer user_data) {
    play_click_sound();
    if (filteredGamesCount > 0 && selectedGameIndex >= 0 && selectedGameIndex < filteredGamesCount) {
        char* targetPath = filteredGamesList[selectedGameIndex].exePath;
        int originalIndex = -1;
        for (int i = 0; i < customGamesCount; i++) {
            if (strcmp(customGamesList[i].exePath, targetPath) == 0) {
                originalIndex = i;
                break;
            }
        }
        if (originalIndex != -1) {
            for (int i = originalIndex; i < customGamesCount - 1; i++) {
                customGamesList[i] = customGamesList[i + 1];
            }
            customGamesCount--;
        }
        save_games_to_disk();
        current_bg_path[0] = '\0';
        update_filtered_games(NULL);
        rebuild_carousel_ui();
        update_bottom_info();
    }
    (void)item;
    (void)user_data;
}

static void on_options_dots_clicked(GtkButton* btn, gpointer user_data) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();
    play_click_sound();
    GtkWidget* popover = gtk_popover_new();
    gtk_widget_add_css_class(popover, "xmb-popover");

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);

    GtkWidget* poster_btn = gtk_button_new_with_label(t->gm_change_poster);
    gtk_widget_add_css_class(poster_btn, "xmb-popover-item");
    g_signal_connect(poster_btn, "clicked", G_CALLBACK(on_change_custom_poster_clicked), NULL);
    g_signal_connect_swapped(poster_btn, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
    gtk_box_append(GTK_BOX(box), poster_btn);

    GtkWidget* folder_btn = gtk_button_new_with_label(t->gm_open_folder);
    gtk_widget_add_css_class(folder_btn, "xmb-popover-item");
    g_signal_connect(folder_btn, "clicked", G_CALLBACK(on_open_game_folder), NULL);
    g_signal_connect_swapped(folder_btn, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
    gtk_box_append(GTK_BOX(box), folder_btn);

    GtkWidget* remove_btn = gtk_button_new_with_label(t->gm_remove_game);
    gtk_widget_add_css_class(remove_btn, "xmb-popover-item-danger");
    g_signal_connect(remove_btn, "clicked", G_CALLBACK(on_remove_selected_game), NULL);
    g_signal_connect_swapped(remove_btn, "clicked", G_CALLBACK(gtk_popover_popdown), popover);
    gtk_box_append(GTK_BOX(box), remove_btn);

    gtk_popover_set_child(GTK_POPOVER(popover), box);
    gtk_widget_set_parent(popover, GTK_WIDGET(btn));
    gtk_popover_popup(GTK_POPOVER(popover));
    (void)user_data;
}

static void on_search_text_changed(GtkSearchEntry* entry, gpointer user_data) {
    const char* text = gtk_editable_get_text(GTK_EDITABLE(entry));
    update_filtered_games(text);
    selectedGameIndex = 0;
    rebuild_carousel_ui();
    update_bottom_info();
    (void)user_data;
}

static void on_search_icon_clicked(GtkButton* btn, gpointer user_data) {
    play_click_sound();
    if (global_search_entry) {
        is_search_visible = !is_search_visible;
        if (is_search_visible) {
            gtk_widget_set_visible(global_search_entry, TRUE);
            gtk_widget_grab_focus(global_search_entry);
        }
        else {
            gtk_widget_set_visible(global_search_entry, FALSE);
            gtk_editable_set_text(GTK_EDITABLE(global_search_entry), "");
            update_filtered_games(NULL);
            rebuild_carousel_ui();
            update_bottom_info();
        }
    }
    (void)btn;
    (void)user_data;
}

static void rebuild_carousel_ui(void) {
    if (!global_carousel_box || !GTK_IS_WIDGET(global_carousel_box)) return;

    GtkWidget* child = gtk_widget_get_first_child(global_carousel_box);
    while (child != NULL) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_box_remove(GTK_BOX(global_carousel_box), child);
        child = next;
    }

    GtkWidget* prev_btn = gtk_button_new_with_label("‹");
    gtk_widget_add_css_class(prev_btn, "xmb-arrow-btn");
    g_signal_connect(prev_btn, "clicked", G_CALLBACK(on_prev_clicked), NULL);
    gtk_box_append(GTK_BOX(global_carousel_box), prev_btn);

    GtkWidget* add_card = gtk_button_new_with_label("+");
    gtk_widget_add_css_class(add_card, "xmb-add-card");
    g_signal_connect(add_card, "clicked", G_CALLBACK(on_add_game_clicked), NULL);
    gtk_box_append(GTK_BOX(global_carousel_box), add_card);

    update_filtered_games(NULL);

    for (int i = 0; i < filteredGamesCount; i++) {
        GtkWidget* card_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_set_valign(card_box, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(card_box, GTK_ALIGN_CENTER);

        GtkWidget* img_widget = NULL;
        char winPosterPath[512];
        strcpy_s(winPosterPath, sizeof(winPosterPath), filteredGamesList[i].posterPath);
        for (int j = 0; winPosterPath[j] != '\0'; j++) if (winPosterPath[j] == '/') winPosterPath[j] = '\\';

        if (filteredGamesList[i].posterPath[0] != '\0' && GetFileAttributesA(winPosterPath) != INVALID_FILE_ATTRIBUTES) {
            img_widget = gtk_picture_new_for_filename(winPosterPath);
            gtk_picture_set_content_fit(GTK_PICTURE(img_widget), GTK_CONTENT_FIT_COVER);
        }
        else {
            GdkTexture* icon_tex = get_exe_icon_texture(filteredGamesList[i].exePath);
            if (icon_tex) {
                img_widget = gtk_picture_new_for_paintable(GDK_PAINTABLE(icon_tex));
                gtk_picture_set_content_fit(GTK_PICTURE(img_widget), GTK_CONTENT_FIT_COVER);
                g_object_unref(icon_tex);
            }
            else {
                img_widget = gtk_label_new("🎮");
                gtk_widget_add_css_class(img_widget, "xmb-card-icon");
            }
        }

        if (img_widget) {
            if (i == selectedGameIndex) {
                gtk_widget_set_size_request(img_widget, 210, 290);
            }
            else {
                gtk_widget_set_size_request(img_widget, 105, 145);
            }
            gtk_box_append(GTK_BOX(card_box), img_widget);
        }

        GtkWidget* title_lbl = gtk_label_new(filteredGamesList[i].title);
        gtk_widget_add_css_class(title_lbl, "xmb-card-title");
        gtk_box_append(GTK_BOX(card_box), title_lbl);

        GtkWidget* card = gtk_button_new();
        gtk_button_set_child(GTK_BUTTON(card), card_box);

        if (i == selectedGameIndex) {
            gtk_widget_add_css_class(card, "xmb-game-card-selected");
        }
        else {
            gtk_widget_add_css_class(card, "xmb-game-card");
        }
        g_signal_connect(card, "clicked", G_CALLBACK(on_game_card_clicked), GINT_TO_POINTER(i));
        gtk_box_append(GTK_BOX(global_carousel_box), card);
    }

    GtkWidget* next_btn = gtk_button_new_with_label("›");
    gtk_widget_add_css_class(next_btn, "xmb-arrow-btn");
    g_signal_connect(next_btn, "clicked", G_CALLBACK(on_next_clicked), NULL);
    gtk_box_append(GTK_BOX(global_carousel_box), next_btn);
}

static gboolean update_clock_tick(gpointer user_data) {
    if (!global_clock_lbl || !GTK_IS_LABEL(global_clock_lbl) || !gtk_widget_get_mapped(global_root_widget)) {
        return G_SOURCE_CONTINUE;
    }
    time_t rawtime;
    struct tm timeinfo;
    char buffer[32];
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer, sizeof(buffer), "%I:%M %p", &timeinfo);
    gtk_label_set_text(GTK_LABEL(global_clock_lbl), buffer);
    (void)user_data;
    return G_SOURCE_CONTINUE;
}

static gboolean on_key_pressed(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    if (keyval == GDK_KEY_Right || keyval == GDK_KEY_d) {
        on_next_clicked(NULL, NULL);
        return TRUE;
    }
    if (keyval == GDK_KEY_Left || keyval == GDK_KEY_a) {
        on_prev_clicked(NULL, NULL);
        return TRUE;
    }
    if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) {
        on_play_game_clicked(NULL, NULL);
        return TRUE;
    }
    if (keyval == GDK_KEY_Escape) {
        on_exit_clicked(NULL, NULL);
        return TRUE;
    }
    (void)controller;
    (void)keycode;
    (void)state;
    (void)user_data;
    return FALSE;
}

static gboolean check_gamepad_input(gpointer user_data) {
    if (!global_root_widget || !GTK_IS_WIDGET(global_root_widget) || !gtk_widget_get_mapped(global_root_widget)) return G_SOURCE_CONTINUE;

    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    if (XInputGetState(0, &state) == ERROR_SUCCESS) {
        static WORD last_buttons = 0;
        WORD buttons = state.Gamepad.wButtons;

        if ((buttons & XINPUT_GAMEPAD_DPAD_RIGHT) && !(last_buttons & XINPUT_GAMEPAD_DPAD_RIGHT)) {
            on_next_clicked(NULL, NULL);
        }
        if ((buttons & XINPUT_GAMEPAD_DPAD_LEFT) && !(last_buttons & XINPUT_GAMEPAD_DPAD_LEFT)) {
            on_prev_clicked(NULL, NULL);
        }
        if ((buttons & XINPUT_GAMEPAD_A) && !(last_buttons & XINPUT_GAMEPAD_A)) {
            on_play_game_clicked(NULL, NULL);
        }
        if ((buttons & XINPUT_GAMEPAD_B) && !(last_buttons & XINPUT_GAMEPAD_B)) {
            on_exit_clicked(NULL, NULL);
        }
        last_buttons = buttons;
    }
    (void)user_data;
    return G_SOURCE_CONTINUE;
}

static void on_screen_mapped(GtkWidget* widget, gpointer user_data) {
    GtkRoot* root = gtk_widget_get_root(widget);
    if (root && GTK_IS_WINDOW(root)) {
        gtk_window_fullscreen(GTK_WINDOW(root));
    }
    play_mci_audio("startupsound", "ps3_startup.mp3", FALSE);
    play_mci_audio("bgmusic", "nostologia.mp3", TRUE);
    (void)user_data;
}

GtkWidget* GameModeScreen(void (*onNavigateBack)(void)) {
    const ZentixTranslationStrings* t = ZentixGetTranslations();

    global_on_navigate_back = onNavigateBack;
    init_base_directory();
    current_bg_path[0] = '\0';

    load_games_from_disk();

    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css_provider,
        ".xmb-brand-dot { color: #1e90ff; font-size: 16px; }"
        ".xmb-brand-text { color: rgba(255, 255, 255, 0.95); font-weight: 700; letter-spacing: 2.5px; font-size: 14px; text-shadow: 0 2px 8px rgba(0,0,0,0.8); }"
        ".xmb-clock-text { color: rgba(255, 255, 255, 0.95); font-weight: 600; font-size: 15px; margin: 0 12px; text-shadow: 0 2px 8px rgba(0,0,0,0.8); }"
        ".xmb-top-icon-btn { background: rgba(255, 255, 255, 0.08); border: 1px solid rgba(255, 255, 255, 0.18); border-radius: 14px; color: white; min-width: 46px; min-height: 46px; font-size: 16px; }"
        ".xmb-top-icon-btn:hover { background: rgba(255, 255, 255, 0.22); border-color: rgba(255, 255, 255, 0.5); }"
        ".xmb-exit-btn { background: rgba(255, 255, 255, 0.08); border: 1px solid rgba(255, 255, 255, 0.22); border-radius: 14px; color: white; min-width: 48px; min-height: 46px; font-weight: bold; font-size: 16px; }"
        ".xmb-exit-btn:hover { background: rgba(235, 64, 52, 0.8); border-color: #ff6b6b; }"
        ".xmb-search-entry { background: rgba(255, 255, 255, 0.12); border: 1px solid rgba(255, 255, 255, 0.3); border-radius: 12px; color: white; padding: 6px 14px; min-width: 220px; }"

        "scrolledwindow, scrolledwindow viewport, .xmb-carousel-scroll, .xmb-carousel-scroll viewport { background: transparent; background-color: transparent; border: none; box-shadow: none; }"
        "box.xmb-carousel-box, .xmb-carousel-box { background: transparent; background-color: transparent; }"

        ".xmb-arrow-btn { background: rgba(255, 255, 255, 0.06); border: 1px solid rgba(255, 255, 255, 0.15); border-radius: 14px; color: rgba(255, 255, 255, 0.8); min-width: 46px; min-height: 90px; font-size: 26px; font-weight: bold; }"
        ".xmb-arrow-btn:hover { background: rgba(255, 255, 255, 0.18); color: white; }"

        ".xmb-add-card { background: rgba(255, 255, 255, 0.07); border: 1px solid rgba(255, 255, 255, 0.18); border-radius: 20px; color: white; min-width: 105px; min-height: 145px; font-size: 32px; font-weight: 300; }"
        ".xmb-game-card { background: rgba(255, 255, 255, 0.06); border: 1px solid rgba(255, 255, 255, 0.15); border-radius: 16px; color: rgba(255, 255, 255, 0.85); min-width: 110px; min-height: 150px; padding: 6px; }"

        "@keyframes ambient-pulse {"
        "  0% { box-shadow: 0 0 25px rgba(30, 144, 255, 0.3), 0 20px 45px rgba(0, 0, 0, 0.85); border-color: rgba(255, 255, 255, 0.7); }"
        "  50% { box-shadow: 0 0 65px rgba(0, 191, 255, 0.75), 0 30px 75px rgba(0, 0, 0, 0.95); border-color: #00bfff; }"
        "  100% { box-shadow: 0 0 25px rgba(30, 144, 255, 0.3), 0 20px 45px rgba(0, 0, 0, 0.85); border-color: rgba(255, 255, 255, 0.7); }"
        "}"

        ".xmb-game-card-selected { background: rgba(255, 255, 255, 0.14); border: 3px solid #00bfff; border-radius: 24px; color: #ffffff; min-width: 220px; min-height: 300px; padding: 10px; animation: ambient-pulse 3s infinite ease-in-out; }"

        ".xmb-card-icon { font-size: 28px; margin-bottom: 4px; }"
        ".xmb-card-title { font-size: 12px; font-weight: 700; color: #ffffff; text-shadow: 0 2px 8px rgba(0,0,0,0.95); margin-top: 4px; }"

        ".xmb-active-title { color: #ffffff; font-size: 42px; font-weight: 800; text-shadow: 0 4px 20px rgba(0,0,0,0.95); }"

        ".xmb-counter-badge { color: rgba(255, 255, 255, 0.9); font-size: 16px; font-weight: 600; background: rgba(255, 255, 255, 0.12); border: 1px solid rgba(255, 255, 255, 0.25); border-radius: 8px; padding: 4px 12px; margin-bottom: 6px; text-shadow: 0 2px 6px rgba(0,0,0,0.8); }"

        ".xmb-play-btn { background: #ffffff; color: #090512; border-radius: 16px; border: none; min-width: 190px; min-height: 54px; font-size: 16px; font-weight: 800; box-shadow: 0 6px 20px rgba(255,255,255,0.3); }"
        ".xmb-play-btn:hover { background: #e0e0e0; }"
        ".xmb-more-btn { background: rgba(255, 255, 255, 0.1); border: 1px solid rgba(255, 255, 255, 0.22); border-radius: 16px; color: white; min-width: 54px; min-height: 54px; font-size: 20px; font-weight: bold; }"

        ".xmb-dark-overlay { background-color: rgba(12, 6, 22, 0.35); }"

        "popover, .xmb-popover {"
        "  background-color: #ffffff;"
        "  border: 1px solid rgba(0, 0, 0, 0.2);"
        "  border-radius: 14px;"
        "  box-shadow: 0 15px 40px rgba(0, 0, 0, 0.4);"
        "  padding: 6px;"
        "}"
        ".xmb-popover-item { background: transparent; border: none; color: #000000; font-weight: 700; padding: 10px 18px; text-align: left; font-size: 14px; }"
        ".xmb-popover-item:hover { background: rgba(0, 0, 0, 0.08); border-radius: 8px; color: #000000; }"
        ".xmb-popover-item-danger { background: transparent; border: none; color: #d32f2f; font-weight: 700; padding: 10px 18px; text-align: left; font-size: 14px; }"
        ".xmb-popover-item-danger:hover { background: rgba(211, 47, 47, 0.12); border-radius: 8px; color: #b71c1c; }"
    );
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(css_provider), 800);

    GtkWidget* overlay = gtk_overlay_new();
    global_root_widget = overlay;
    gtk_widget_set_vexpand(overlay, TRUE);
    gtk_widget_set_hexpand(overlay, TRUE);

    g_signal_connect(overlay, "map", G_CALLBACK(on_screen_mapped), NULL);

    global_bg_picture = gtk_picture_new();
    gtk_picture_set_content_fit(GTK_PICTURE(global_bg_picture), GTK_CONTENT_FIT_COVER);
    gtk_widget_set_hexpand(global_bg_picture, TRUE);
    gtk_widget_set_vexpand(global_bg_picture, TRUE);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), global_bg_picture);

    GtkWidget* dark_layer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(dark_layer, "xmb-dark-overlay");
    gtk_widget_set_hexpand(dark_layer, TRUE);
    gtk_widget_set_vexpand(dark_layer, TRUE);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), dark_layer);

    GtkWidget* content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(content_box, TRUE);
    gtk_widget_set_hexpand(content_box, TRUE);
    gtk_widget_set_margin_start(content_box, 48);
    gtk_widget_set_margin_end(content_box, 48);
    gtk_widget_set_margin_top(content_box, 32);
    gtk_widget_set_margin_bottom(content_box, 48);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), content_box);

    GtkEventController* key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), NULL);
    gtk_widget_add_controller(overlay, key_controller);

    GtkWidget* top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* dot_lbl = gtk_label_new("●");
    gtk_widget_add_css_class(dot_lbl, "xmb-brand-dot");
    gtk_box_append(GTK_BOX(top_bar), dot_lbl);

    GtkWidget* title_lbl = gtk_label_new(t->gm_brand);
    gtk_widget_add_css_class(title_lbl, "xmb-brand-text");
    gtk_widget_set_hexpand(title_lbl, TRUE);
    gtk_widget_set_halign(title_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(top_bar), title_lbl);

    global_search_entry = gtk_search_entry_new();
    gtk_widget_add_css_class(global_search_entry, "xmb-search-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(global_search_entry), t->gm_search_placeholder);
    gtk_widget_set_visible(global_search_entry, FALSE);
    g_signal_connect(global_search_entry, "search-changed", G_CALLBACK(on_search_text_changed), NULL);
    gtk_box_append(GTK_BOX(top_bar), global_search_entry);

    GtkWidget* search_btn = gtk_button_new_with_label("🔍");
    gtk_widget_set_tooltip_text(search_btn, "Search Games");
    gtk_widget_add_css_class(search_btn, "xmb-top-icon-btn");
    g_signal_connect(search_btn, "clicked", G_CALLBACK(on_search_icon_clicked), NULL);
    gtk_box_append(GTK_BOX(top_bar), search_btn);

    global_clock_lbl = gtk_label_new("10:30 PM");
    gtk_widget_add_css_class(global_clock_lbl, "xmb-clock-text");
    gtk_box_append(GTK_BOX(top_bar), global_clock_lbl);
    update_clock_tick(NULL);
    g_timeout_add_seconds(5, update_clock_tick, NULL);

    GtkWidget* exit_btn = gtk_button_new_with_label("✕");
    gtk_widget_set_tooltip_text(exit_btn, t->gm_exit);
    gtk_widget_add_css_class(exit_btn, "xmb-exit-btn");
    g_signal_connect(exit_btn, "clicked", G_CALLBACK(on_exit_clicked), NULL);
    gtk_box_append(GTK_BOX(top_bar), exit_btn);

    gtk_box_append(GTK_BOX(content_box), top_bar);

    GtkWidget* carousel_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(carousel_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_widget_add_css_class(carousel_scroll, "xmb-carousel-scroll");
    gtk_widget_set_vexpand(carousel_scroll, TRUE);
    gtk_widget_set_valign(carousel_scroll, GTK_ALIGN_CENTER);

    global_carousel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);
    gtk_widget_add_css_class(global_carousel_box, "xmb-carousel-box");
    gtk_widget_set_valign(global_carousel_box, GTK_ALIGN_CENTER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(carousel_scroll), global_carousel_box);
    gtk_box_append(GTK_BOX(content_box), carousel_scroll);

    update_filtered_games(NULL);
    rebuild_carousel_ui();

    GtkWidget* bottom_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_halign(bottom_box, GTK_ALIGN_START);

    global_counter_lbl = gtk_label_new("[ 1 / 1 ]");
    gtk_widget_add_css_class(global_counter_lbl, "xmb-counter-badge");
    gtk_widget_set_halign(global_counter_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(bottom_box), global_counter_lbl);

    global_active_title_lbl = gtk_label_new(t->gm_no_games);
    gtk_widget_add_css_class(global_active_title_lbl, "xmb-active-title");
    gtk_widget_set_halign(global_active_title_lbl, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(bottom_box), global_active_title_lbl);
    update_bottom_info();

    GtkWidget* actions_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* play_btn = gtk_button_new_with_label(t->gm_play);
    gtk_widget_add_css_class(play_btn, "xmb-play-btn");
    g_signal_connect(play_btn, "clicked", G_CALLBACK(on_play_game_clicked), NULL);
    gtk_box_append(GTK_BOX(actions_row), play_btn);

    GtkWidget* options_dots_btn = gtk_button_new_with_label("⋮");
    gtk_widget_set_tooltip_text(options_dots_btn, t->gm_settings);
    gtk_widget_add_css_class(options_dots_btn, "xmb-more-btn");
    g_signal_connect(options_dots_btn, "clicked", G_CALLBACK(on_options_dots_clicked), NULL);
    gtk_box_append(GTK_BOX(actions_row), options_dots_btn);

    gtk_box_append(GTK_BOX(bottom_box), actions_row);
    gtk_box_append(GTK_BOX(content_box), bottom_box);

    g_timeout_add(90, check_gamepad_input, NULL);

    return overlay;
}