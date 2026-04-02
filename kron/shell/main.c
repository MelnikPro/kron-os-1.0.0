#include "panel.h"
#include "launcher.h"
#include "notifications.h"
#include <gtk/gtk.h>

static void on_activate(GtkApplication *app, gpointer data) {
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, "/usr/share/kron/style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    kron_notifications_init(app);
    kron_launcher_init(app);
    kron_panel_create(app);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.kron.shell", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int ret = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return ret;
}
