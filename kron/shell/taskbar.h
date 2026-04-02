#pragma once
#include <gtk/gtk.h>

/* Инициализирует taskbar, возвращает GtkBox для вставки в панель */
GtkWidget *kron_taskbar_new(void);
