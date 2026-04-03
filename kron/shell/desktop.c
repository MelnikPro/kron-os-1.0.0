#include "desktop.h"
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <gtk/gtk.h>

void kron_desktop_init(GtkApplication *app, const char *wallpaper_path) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_widget_set_name(window, "kron-desktop");

    /* layer-shell: самый нижний слой — под всеми окнами */
    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_BACKGROUND);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP,    TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT,   TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT,  TRUE);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(window), -1);
    gtk_layer_set_namespace(GTK_WINDOW(window), "kron-desktop");

    if (wallpaper_path && wallpaper_path[0] != '\0') {
        GFile *file = g_file_new_for_path(wallpaper_path);
        GtkWidget *pic = gtk_picture_new_for_file(file);
        gtk_picture_set_content_fit(GTK_PICTURE(pic), GTK_CONTENT_FIT_COVER);
        gtk_widget_set_hexpand(pic, TRUE);
        gtk_widget_set_vexpand(pic, TRUE);
        gtk_window_set_child(GTK_WINDOW(window), pic);
        g_object_unref(file);
    } else {
        /* Дефолтный градиент через CSS если обоев нет */
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_add_css_class(box, "desktop-bg");
        gtk_window_set_child(GTK_WINDOW(window), box);
    }

    gtk_window_present(GTK_WINDOW(window));
}
