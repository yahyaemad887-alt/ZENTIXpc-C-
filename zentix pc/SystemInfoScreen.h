#ifndef SYSTEMINFOSCREEN_H
#define SYSTEMINFOSCREEN_H

#include <gtk/gtk.h>
#include "zentix.h"

#ifdef __cplusplus
extern "C" {
#endif

    // إعلان دالة شاشة معلومات النظام والمواصفات
    __declspec(dllexport) GtkWidget* SystemInfoScreen(SystemInfoCallbacks callbacks);

#ifdef __cplusplus
}
#endif

#endif // SYSTEMINFOSCREEN_H