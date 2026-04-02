#pragma once
#include <gtk/gtk.h>

void kron_notifications_init(GtkApplication *app);
void kron_notify(const char *summary, const char *body, int timeout_ms);
