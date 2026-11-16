#ifndef FPS_AND_OTHER_H
#define FPS_AND_OTHER_H

#include <gtk/gtk.h>
#include "zentix.h"

#ifdef __cplusplus
extern "C" {
#endif

    // إعلان دالة شاشة الإطارات والقياسات مدعومة بالترجمة المركزية
    __declspec(dllexport) GtkWidget* FpsAndOtherScreen(void (*on_back)(void));

#ifdef __cplusplus
}
#endif

#endif // FPS_AND_OTHER_H