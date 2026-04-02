#include "taskbar.h"
#include <gtk/gtk.h>
#include <gdk/wayland/gdkwayland.h>
#include <wayland-client.h>
#include "wlr-foreign-toplevel-management-unstable-v1-client.h"
#include <string.h>
#include <stdlib.h>

/* ── Состояние одного окна ── */
typedef struct {
    struct zwlr_foreign_toplevel_handle_v1 *handle;
    GtkWidget *button;
    char      *title;
    char      *app_id;
    bool       active;
} TaskbarItem;

static GtkWidget  *taskbar_box = NULL;
static GList      *items       = NULL;
static struct wl_display *wl_display_global = NULL;

/* ── Обновить кнопку ── */
static void item_update_button(TaskbarItem *item) {
    if (!item->button) return;
    const char *label = (item->title && item->title[0]) ? item->title
                      : (item->app_id ? item->app_id : "Окно");
    gtk_button_set_label(GTK_BUTTON(item->button), label);
    if (item->active)
        gtk_widget_add_css_class(item->button, "taskbar-active");
    else
        gtk_widget_remove_css_class(item->button, "taskbar-active");
}

/* ── Клик по кнопке — активируем окно ── */
static void on_taskbar_btn_clicked(GtkButton *btn, gpointer data) {
    TaskbarItem *item = (TaskbarItem *)data;
    struct wl_seat *seat = NULL;

    /* Получаем seat из GDK */
    GdkDisplay *gdk_display = gdk_display_get_default();
    GdkSeat *gdk_seat = gdk_display_get_default_seat(gdk_display);
    if (gdk_seat)
        seat = gdk_wayland_seat_get_wl_seat(gdk_seat);

    if (seat)
        zwlr_foreign_toplevel_handle_v1_activate(item->handle, seat);
}

/* ── Найти item по handle ── */
static TaskbarItem *find_item(struct zwlr_foreign_toplevel_handle_v1 *handle) {
    for (GList *l = items; l; l = l->next) {
        TaskbarItem *item = l->data;
        if (item->handle == handle) return item;
    }
    return NULL;
}

/* ── Обработчики foreign-toplevel handle ── */
static void handle_title(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle, const char *title) {
    TaskbarItem *item = find_item(handle);
    if (!item) return;
    free(item->title);
    item->title = strdup(title);
    item_update_button(item);
}

static void handle_app_id(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle, const char *app_id) {
    TaskbarItem *item = find_item(handle);
    if (!item) return;
    free(item->app_id);
    item->app_id = strdup(app_id);
    item_update_button(item);
}

static void handle_output_enter(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle, struct wl_output *output) {}

static void handle_output_leave(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle, struct wl_output *output) {}

static void handle_state(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle, struct wl_array *state) {
    TaskbarItem *item = find_item(handle);
    if (!item) return;
    item->active = false;
    uint32_t *s;
    wl_array_for_each(s, state) {
        if (*s == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED)
            item->active = true;
    }
    item_update_button(item);
}

static void handle_done(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle) {}

static void handle_closed(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle) {
    TaskbarItem *item = find_item(handle);
    if (!item) return;
    if (item->button && taskbar_box)
        gtk_box_remove(GTK_BOX(taskbar_box), item->button);
    items = g_list_remove(items, item);
    free(item->title);
    free(item->app_id);
    zwlr_foreign_toplevel_handle_v1_destroy(handle);
    free(item);
}

static void handle_parent(void *data,
    struct zwlr_foreign_toplevel_handle_v1 *handle,
    struct zwlr_foreign_toplevel_handle_v1 *parent) {}

static const struct zwlr_foreign_toplevel_handle_v1_listener handle_listener = {
    .title        = handle_title,
    .app_id       = handle_app_id,
    .output_enter = handle_output_enter,
    .output_leave = handle_output_leave,
    .state        = handle_state,
    .done         = handle_done,
    .closed       = handle_closed,
    .parent       = handle_parent,
};

/* ── Новое окно появилось ── */
static void on_toplevel(void *data,
    struct zwlr_foreign_toplevel_manager_v1 *mgr,
    struct zwlr_foreign_toplevel_handle_v1 *handle) {

    TaskbarItem *item = calloc(1, sizeof(TaskbarItem));
    item->handle = handle;

    item->button = gtk_button_new_with_label("...");
    gtk_widget_add_css_class(item->button, "taskbar-item");
    g_signal_connect(item->button, "clicked", G_CALLBACK(on_taskbar_btn_clicked), item);
    gtk_box_append(GTK_BOX(taskbar_box), item->button);

    items = g_list_append(items, item);
    zwlr_foreign_toplevel_handle_v1_add_listener(handle, &handle_listener, NULL);
}

static void on_finished(void *data,
    struct zwlr_foreign_toplevel_manager_v1 *mgr) {}

static const struct zwlr_foreign_toplevel_manager_v1_listener manager_listener = {
    .toplevel = on_toplevel,
    .finished = on_finished,
};

/* ── Регистрация глобальных объектов Wayland ── */
static void registry_global(void *data, struct wl_registry *registry,
    uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        struct zwlr_foreign_toplevel_manager_v1 *mgr =
            wl_registry_bind(registry, name,
                &zwlr_foreign_toplevel_manager_v1_interface,
                MIN(version, 3));
        zwlr_foreign_toplevel_manager_v1_add_listener(mgr, &manager_listener, NULL);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

GtkWidget *kron_taskbar_new(void) {
    taskbar_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    /* Подключаемся к Wayland display через GDK */
    GdkDisplay *gdk_display = gdk_display_get_default();
    struct wl_display *wl_dpy = gdk_wayland_display_get_wl_display(gdk_display);
    struct wl_registry *registry = wl_display_get_registry(wl_dpy);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(wl_dpy);

    return taskbar_box;
}
