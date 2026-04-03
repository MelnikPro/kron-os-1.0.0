#include "toplevel.h"
#include "server.h"
#include "animation.h"
#include <stdlib.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>

/* ── Фокус окна ── */
void kron_toplevel_focus(KronToplevel *toplevel, struct wlr_surface *surface) {
    if (!toplevel) return;
    KronServer *server = toplevel->server;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *prev = seat->keyboard_state.focused_surface;

    if (prev == surface) return;

    if (prev) {
        struct wlr_xdg_surface *xdg = wlr_xdg_surface_try_from_wlr_surface(prev);
        if (xdg && xdg->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
            wlr_xdg_toplevel_set_activated(xdg->toplevel, false);
    }

    wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
    wl_list_remove(&toplevel->link);
    wl_list_insert(&server->toplevels, &toplevel->link);
    wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);

    struct wlr_keyboard *kb = wlr_seat_get_keyboard(seat);
    if (kb)
        wlr_seat_keyboard_notify_enter(seat, surface,
            kb->keycodes, kb->num_keycodes, &kb->modifiers);
}

/* ── Обработчики событий toplevel ── */
static void on_map(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, map);

    /* Анимация открытия окна */
    kron_anim_open(toplevel->scene_tree);

    /* Регистрируем в foreign-toplevel чтобы taskbar видел окно */
    if (toplevel->server->foreign_toplevel_mgr) {
        toplevel->foreign_handle = wlr_foreign_toplevel_handle_v1_create(
            toplevel->server->foreign_toplevel_mgr);
        const char *title = toplevel->xdg_toplevel->title;
        const char *app_id = toplevel->xdg_toplevel->app_id;
        if (title)  wlr_foreign_toplevel_handle_v1_set_title(toplevel->foreign_handle, title);
        if (app_id) wlr_foreign_toplevel_handle_v1_set_app_id(toplevel->foreign_handle, app_id);
        wlr_foreign_toplevel_handle_v1_set_mapped(toplevel->foreign_handle, true);
    }

    kron_toplevel_focus(toplevel, toplevel->xdg_toplevel->base->surface);
}

static void on_unmap(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, unmap);
    if (toplevel == toplevel->server->grabbed_toplevel)
        toplevel->server->cursor_mode = KRON_CURSOR_PASSTHROUGH;

    /* Убираем из foreign-toplevel */
    if (toplevel->foreign_handle) {
        wlr_foreign_toplevel_handle_v1_set_mapped(toplevel->foreign_handle, false);
        wlr_foreign_toplevel_handle_v1_destroy(toplevel->foreign_handle);
        toplevel->foreign_handle = NULL;
    }
}

static void on_commit(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, commit);
    if (toplevel->xdg_toplevel->base->initial_commit)
        wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
}

static void on_destroy(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, destroy);
    wl_list_remove(&toplevel->map.link);
    wl_list_remove(&toplevel->unmap.link);
    wl_list_remove(&toplevel->commit.link);
    wl_list_remove(&toplevel->destroy.link);
    wl_list_remove(&toplevel->request_move.link);
    wl_list_remove(&toplevel->request_resize.link);
    wl_list_remove(&toplevel->request_maximize.link);
    wl_list_remove(&toplevel->link);
    free(toplevel);
}

static void on_request_move(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, request_move);
    kron_toplevel_begin_move(toplevel, 0);
}

static void on_request_resize(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    kron_toplevel_begin_resize(toplevel, event->edges, 0);
}

static void on_request_maximize(struct wl_listener *listener, void *data) {
    KronToplevel *toplevel = wl_container_of(listener, toplevel, request_maximize);
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
}

/* ── Новый toplevel ── */
static void on_new_xdg_toplevel(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, new_xdg_toplevel);
    struct wlr_xdg_toplevel *xdg_toplevel = data;

    KronToplevel *toplevel = calloc(1, sizeof(KronToplevel));
    toplevel->server = server;
    toplevel->xdg_toplevel = xdg_toplevel;
    toplevel->scene_tree = wlr_scene_xdg_surface_create(
        &server->scene->tree, xdg_toplevel->base);
    toplevel->scene_tree->node.data = toplevel;
    xdg_toplevel->base->data = toplevel->scene_tree;

#define LISTEN(event, field, cb) toplevel->field.notify = cb; wl_signal_add(&event, &toplevel->field)
    LISTEN(xdg_toplevel->base->surface->events.map,    map,              on_map);
    LISTEN(xdg_toplevel->base->surface->events.unmap,  unmap,            on_unmap);
    LISTEN(xdg_toplevel->base->surface->events.commit, commit,           on_commit);
    LISTEN(xdg_toplevel->events.destroy,               destroy,          on_destroy);
    LISTEN(xdg_toplevel->events.request_move,          request_move,     on_request_move);
    LISTEN(xdg_toplevel->events.request_resize,        request_resize,   on_request_resize);
    LISTEN(xdg_toplevel->events.request_maximize,      request_maximize, on_request_maximize);
#undef LISTEN

    wl_list_insert(&server->toplevels, &toplevel->link);
}

static void on_new_xdg_popup(struct wl_listener *listener, void *data) {
    struct wlr_xdg_popup *popup = data;
    struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(popup->parent);
    struct wlr_scene_tree *parent_tree = parent->data;
    popup->base->data = wlr_scene_xdg_surface_create(parent_tree, popup->base);
}

void kron_xdg_shell_init(KronServer *server) {
    server->new_xdg_toplevel.notify = on_new_xdg_toplevel;
    wl_signal_add(&server->xdg_shell->events.new_toplevel, &server->new_xdg_toplevel);

    server->new_xdg_popup.notify = on_new_xdg_popup;
    wl_signal_add(&server->xdg_shell->events.new_popup, &server->new_xdg_popup);
}

/* ── Move / Resize ── */
void kron_toplevel_begin_move(KronToplevel *toplevel, uint32_t time_msec) {
    KronServer *server = toplevel->server;
    server->grabbed_toplevel = toplevel;
    server->cursor_mode = KRON_CURSOR_MOVE;

    int lx, ly;
    wlr_scene_node_coords(&toplevel->scene_tree->node, &lx, &ly);
    server->grab_x = server->cursor->x - lx;
    server->grab_y = server->cursor->y - ly;
}

void kron_toplevel_begin_resize(KronToplevel *toplevel, uint32_t edges, uint32_t time_msec) {
    KronServer *server = toplevel->server;
    server->grabbed_toplevel = toplevel;
    server->cursor_mode = KRON_CURSOR_RESIZE;
    server->resize_edges = edges;

    int lx, ly;
    wlr_scene_node_coords(&toplevel->scene_tree->node, &lx, &ly);
    struct wlr_box geo;
    wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo);
    server->grab_geobox = geo;
    server->grab_geobox.x += lx;
    server->grab_geobox.y += ly;
    server->grab_x = server->cursor->x;
    server->grab_y = server->cursor->y;
}
