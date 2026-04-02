#include "launcher.h"
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <string.h>

static GtkWidget *launcher_window = NULL;
static GtkWidget *flow_box        = NULL;
static GList     *all_apps        = NULL;

static void launch_app(GtkButton *btn, gpointer data) {
    g_app_info_launch(G_APP_INFO(data), NULL, NULL, NULL);
    gtk_widget_set_visible(launcher_window, FALSE);
}

static GtkWidget *make_app_button(GAppInfo *app) {
    GtkWidget *btn  = gtk_button_new();
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);

    GIcon *icon = g_app_info_get_icon(app);
    if (icon) {
        GtkWidget *img = gtk_image_new_from_gicon(icon);
        gtk_image_set_pixel_size(GTK_IMAGE(img), 48);
        gtk_box_append(GTK_BOX(vbox), img);
    }

    GtkWidget *lbl = gtk_label_new(g_app_info_get_name(app));
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 12);
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_box_append(GTK_BOX(vbox), lbl);

    gtk_button_set_child(GTK_BUTTON(btn), vbox);
    gtk_widget_set_size_request(btn, 90, 90);
    gtk_widget_add_css_class(btn, "app-button");
    g_signal_connect(btn, "clicked", G_CALLBACK(launch_app), app);
    return btn;
}

static void on_search_changed(GtkSearchEntry *entry, gpointer data) {
    const char *q = gtk_editable_get_text(GTK_EDITABLE(entry));

    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(flow_box)))
        gtk_flow_box_remove(GTK_FLOW_BOX(flow_box), child);

    for (GList *l = all_apps; l; l = l->next) {
        GAppInfo *app = G_APP_INFO(l->data);
        if (!q || !q[0] || g_ascii_strcasestr(g_app_info_get_name(app), q))
            gtk_flow_box_append(GTK_FLOW_BOX(flow_box), make_app_button(app));
    }
}

void kron_launcher_init(GtkApplication *app) {
    all_apps = g_app_info_get_all();

    launcher_window = gtk_application_window_new(app);
    gtk_widget_set_name(launcher_window, "kron-launcher");

    /* layer-shell: поверх всего, по центру */
    gtk_layer_init_for_window(GTK_WINDOW(launcher_window));
    gtk_layer_set_layer(GTK_WINDOW(launcher_window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(launcher_window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
    gtk_layer_set_namespace(GTK_WINDOW(launcher_window), "kron-launcher");
    gtk_window_set_default_size(GTK_WINDOW(launcher_window), 800, 560);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 12);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_window_set_child(GTK_WINDOW(launcher_window), vbox);

    GtkWidget *search = gtk_search_entry_new();
    gtk_widget_add_css_class(search, "launcher-search");
    g_signal_connect(search, "search-changed", G_CALLBACK(on_search_changed), NULL);
    gtk_box_append(GTK_BOX(vbox), search);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_append(GTK_BOX(vbox), scroll);

    flow_box = gtk_flow_box_new();
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow_box), 4);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow_box), 8);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow_box), GTK_SELECTION_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), flow_box);

    on_search_changed(GTK_SEARCH_ENTRY(search), NULL);
    gtk_widget_set_visible(launcher_window, FALSE);
}

void kron_launcher_toggle(void) {
    gboolean visible = gtk_widget_get_visible(launcher_window);
    gtk_widget_set_visible(launcher_window, !visible);
}
