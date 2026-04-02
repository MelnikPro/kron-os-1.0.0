#include "notifications.h"
#include <gtk4-layer-shell/gtk4-layer-shell.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <string.h>

/* ── Стек уведомлений ── */
static GtkWidget *notif_box    = NULL;  /* вертикальный стек */
static GtkWidget *notif_window = NULL;

typedef struct {
    GtkWidget *widget;
    guint      timeout_id;
} KronNotif;

static void remove_notif(gpointer data) {
    KronNotif *n = (KronNotif *)data;
    if (n->timeout_id) g_source_remove(n->timeout_id);
    gtk_box_remove(GTK_BOX(notif_box), n->widget);
    g_free(n);
}

static gboolean on_notif_timeout(gpointer data) {
    remove_notif(data);
    return G_SOURCE_REMOVE;
}

void kron_notify(const char *summary, const char *body, int timeout_ms) {
    KronNotif *n = g_new0(KronNotif, 1);

    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_add_css_class(card, "notification");
    gtk_widget_set_margin_bottom(card, 4);

    GtkWidget *title = gtk_label_new(summary);
    gtk_widget_add_css_class(title, "notification-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(card), title);

    if (body && body[0]) {
        GtkWidget *text = gtk_label_new(body);
        gtk_widget_add_css_class(text, "notification-body");
        gtk_label_set_xalign(GTK_LABEL(text), 0);
        gtk_label_set_wrap(GTK_LABEL(text), TRUE);
        gtk_box_append(GTK_BOX(card), text);
    }

    /* Кнопка закрыть */
    GtkWidget *close_btn = gtk_button_new_with_label("✕");
    gtk_widget_add_css_class(close_btn, "notification-close");
    gtk_widget_set_halign(close_btn, GTK_ALIGN_END);
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(remove_notif), n);
    gtk_box_append(GTK_BOX(card), close_btn);

    n->widget = card;
    gtk_box_prepend(GTK_BOX(notif_box), card);

    if (timeout_ms > 0)
        n->timeout_id = g_timeout_add(timeout_ms, on_notif_timeout, n);
}

/* ── DBus: слушаем org.freedesktop.Notifications ── */
static void on_dbus_method(GDBusConnection *conn, const char *sender,
    const char *path, const char *iface, const char *method,
    GVariant *params, GDBusMethodInvocation *invoc, gpointer data)
{
    if (strcmp(method, "Notify") == 0) {
        const char *app_name, *summary, *body;
        guint32 replaces_id;
        g_variant_get(params, "(&su&s&s*&sa{sv}i)",
            &app_name, &replaces_id, NULL, &summary, &body,
            NULL, NULL, NULL);
        kron_notify(summary, body, 5000);
        g_dbus_method_invocation_return_value(invoc, g_variant_new("(u)", 1));
    } else if (strcmp(method, "GetCapabilities") == 0) {
        GVariantBuilder b;
        g_variant_builder_init(&b, G_VARIANT_TYPE("as"));
        g_variant_builder_add(&b, "s", "body");
        g_dbus_method_invocation_return_value(invoc, g_variant_new("(as)", &b));
    } else if (strcmp(method, "GetServerInformation") == 0) {
        g_dbus_method_invocation_return_value(invoc,
            g_variant_new("(ssss)", "kron-shell", "KRON", "0.1", "1.2"));
    } else if (strcmp(method, "CloseNotification") == 0) {
        g_dbus_method_invocation_return_value(invoc, NULL);
    }
}

static const GDBusInterfaceVTable notif_vtable = { on_dbus_method, NULL, NULL };

static void on_bus_acquired(GDBusConnection *conn, const char *name, gpointer data) {
    static const char *notif_xml =
        "<node>"
        "  <interface name='org.freedesktop.Notifications'>"
        "    <method name='Notify'>"
        "      <arg type='s' direction='in'/><arg type='u' direction='in'/>"
        "      <arg type='s' direction='in'/><arg type='s' direction='in'/>"
        "      <arg type='s' direction='in'/><arg type='as' direction='in'/>"
        "      <arg type='a{sv}' direction='in'/><arg type='i' direction='in'/>"
        "      <arg type='u' direction='out'/>"
        "    </method>"
        "    <method name='GetCapabilities'><arg type='as' direction='out'/></method>"
        "    <method name='GetServerInformation'>"
        "      <arg type='s' direction='out'/><arg type='s' direction='out'/>"
        "      <arg type='s' direction='out'/><arg type='s' direction='out'/>"
        "    </method>"
        "    <method name='CloseNotification'><arg type='u' direction='in'/></method>"
        "  </interface>"
        "</node>";

    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(notif_xml, NULL);
    g_dbus_connection_register_object(conn,
        "/org/freedesktop/Notifications",
        info->interfaces[0],
        &notif_vtable, NULL, NULL, NULL);
    g_dbus_node_info_unref(info);
}

void kron_notifications_init(GtkApplication *app) {
    /* Окно для стека уведомлений */
    notif_window = gtk_application_window_new(app);
    gtk_widget_set_name(notif_window, "kron-notifications");

    gtk_layer_init_for_window(GTK_WINDOW(notif_window));
    gtk_layer_set_layer(GTK_WINDOW(notif_window), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_anchor(GTK_WINDOW(notif_window), GTK_LAYER_SHELL_EDGE_TOP,   TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(notif_window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(notif_window), GTK_LAYER_SHELL_EDGE_TOP,   48);
    gtk_layer_set_margin(GTK_WINDOW(notif_window), GTK_LAYER_SHELL_EDGE_RIGHT, 8);
    gtk_layer_set_namespace(GTK_WINDOW(notif_window), "kron-notifications");

    notif_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_size_request(notif_box, 320, -1);
    gtk_window_set_child(GTK_WINDOW(notif_window), notif_box);
    gtk_window_present(GTK_WINDOW(notif_window));

    /* DBus */
    g_bus_own_name(G_BUS_TYPE_SESSION,
        "org.freedesktop.Notifications",
        G_BUS_NAME_OWNER_FLAGS_REPLACE,
        on_bus_acquired, NULL, NULL, NULL, NULL);
}
