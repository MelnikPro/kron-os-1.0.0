#include "panel.h"
#include "launcher.h"
#include "taskbar.h"
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <gtk/gtk.h>
#include <time.h>

static GtkWidget *clock_label = NULL;

static gboolean update_clock(gpointer data) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M  %d.%m.%Y", t);
    gtk_label_set_text(GTK_LABEL(clock_label), buf);
    return G_SOURCE_CONTINUE;
}

static void on_launcher_clicked(GtkButton *btn, gpointer data) {
    kron_launcher_toggle();
}

static void on_logout_clicked(GtkButton *btn, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Завершить сессию KRON?");
    g_signal_connect_swapped(dialog, "response",
        G_CALLBACK(gtk_window_destroy), dialog);
    gtk_widget_show(dialog);
}

void kron_panel_create(GtkApplication *app) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_widget_set_name(window, "kron-panel");

    /* layer-shell: прибиваем панель к верху экрана */
    gtk_layer_init_for_window(GTK_WINDOW(window));
    gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP,    TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT,   TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT,  TRUE);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(window), 36);
    gtk_layer_set_namespace(GTK_WINDOW(window), "kron-panel");
    gtk_widget_set_size_request(window, -1, 36);

    /* Контент панели */
    GtkWidget *box = gtk_center_box_new();
    gtk_window_set_child(GTK_WINDOW(window), box);

    /* Левая часть: кнопка меню */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *launcher_btn = gtk_button_new_with_label("⬡  KRON");
    gtk_widget_add_css_class(launcher_btn, "panel-button");
    g_signal_connect(launcher_btn, "clicked", G_CALLBACK(on_launcher_clicked), NULL);
    gtk_box_append(GTK_BOX(left), launcher_btn);
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(box), left);

    /* Taskbar: список открытых окон */
    GtkWidget *taskbar = kron_taskbar_new();
    gtk_widget_set_hexpand(taskbar, TRUE);
    gtk_center_box_set_center_widget(GTK_CENTER_BOX(box), taskbar);

    /* Правая часть: часы + кнопка выхода */
    GtkWidget *right = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    clock_label = gtk_label_new("");
    gtk_widget_add_css_class(clock_label, "clock");
    gtk_box_append(GTK_BOX(right), clock_label);
    update_clock(NULL);
    g_timeout_add_seconds(1, update_clock, NULL);

    GtkWidget *logout_btn = gtk_button_new_with_label("⏻");
    gtk_widget_add_css_class(logout_btn, "panel-button");
    g_signal_connect(logout_btn, "clicked", G_CALLBACK(on_logout_clicked), NULL);
    gtk_box_append(GTK_BOX(right), logout_btn);
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(box), right);

    gtk_window_present(GTK_WINDOW(window));
}
