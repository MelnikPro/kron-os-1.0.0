#include "dock.h"
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

/* Приложения в доке — можно расширить через конфиг */
static const char *DOCK_APPS[] = {
    "org.gnome.Nautilus",
    "org.gnome.Terminal",
    "firefox",
    "org.gnome.gedit",
    "org.gnome.Calculator",
    "org.gnome.Settings",
    NULL
};

static void on_dock_app_clicked(GtkButton *btn, gpointer data) {
    GAppInfo *app = G_APP_INFO(data);
    g_app_info_launch(app, NULL, NULL, NULL);
}

static GtkWidget *make_dock_item(GAppInfo *app) {
    GtkWidget *btn = gtk_button_new();
    gtk_widget_add_css_class(btn, "dock-item");
    gtk_widget_set_tooltip_text(btn, g_app_info_get_name(app));
    gtk_widget_set_size_request(btn, 52, 52);

    GIcon *icon = g_app_info_get_icon(app);
    if (icon) {
        GtkWidget *img = gtk_image_new_from_gicon(icon);
        gtk_image_set_pixel_size(GTK_IMAGE(img), 36);
        gtk_button_set_child(GTK_BUTTON(btn), img);
    } else {
        gtk_button_set_label(GTK_BUTTON(btn), "?");
    }

    g_signal_connect(btn, "clicked", G_CALLBACK(on_dock_app_clicked), app);
    return btn;
}

void kron_dock_init(GtkApplication *app) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_widget_set_name(window, "kron-dock");

    /* layer-shell: прибиваем к низу экрана */
    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, 10);
    gtk_layer_set_namespace(GTK_WINDOW(window), "kron-dock");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top(box, 8);
    gtk_widget_set_margin_bottom(box, 8);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_window_set_child(GTK_WINDOW(window), box);

    /* Заполняем dock приложениями */
    for (int i = 0; DOCK_APPS[i]; i++) {
        GDesktopAppInfo *info = g_desktop_app_info_new_from_filename(
            g_strdup_printf("/usr/share/applications/%s.desktop", DOCK_APPS[i]));
        if (!info) {
            /* Пробуем без пути */
            char *id = g_strdup_printf("%s.desktop", DOCK_APPS[i]);
            info = g_desktop_app_info_new(id);
            g_free(id);
        }
        if (info) {
            GtkWidget *item = make_dock_item(G_APP_INFO(info));
            gtk_box_append(GTK_BOX(box), item);
        }
    }

    gtk_window_present(GTK_WINDOW(window));
}
