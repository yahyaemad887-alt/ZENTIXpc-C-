#ifndef GAMEMOD_H
#define GAMEMOD_H

#include <gtk/gtk.h>
#include "zentix.h"

#ifdef __cplusplus
extern "C" {
#endif

    // إعلان دالة شاشة وضع الألعاب مع جلب الترجمات مركزياً
    __declspec(dllexport) GtkWidget* GameModeScreen(void (*onNavigateBack)(void));

#ifdef __cplusplus
}
#endif

#endif // GAMEMOD_H