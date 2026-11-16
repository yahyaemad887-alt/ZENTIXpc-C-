#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <gtk/gtk.h>
#include "zentix.h"

#ifdef __cplusplus
extern "C" {
#endif

    // إعلان دالة شاشة الإعدادات وال تفضيلات
    __declspec(dllexport) GtkWidget* SettingsScreen(void (*onNavigateBack)(void));

#ifdef __cplusplus
}
#endif

#endif // SETTINGSSCREEN_H