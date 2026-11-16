#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <gtk/gtk.h>
#include "zentix.h"

#ifdef __cplusplus
extern "C" {
#endif

    // إعلان دالة شاشة الرئيسية (Home Screen)
    __declspec(dllexport) GtkWidget* HomeScreen(HomeScreenCallbacks callbacks);

#ifdef __cplusplus
}
#endif

#endif // HOMESCREEN_H