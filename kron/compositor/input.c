#include "input.h"
#include "server.h"
#include "toplevel.h"
#include <stdlib.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <xkbcommon/xkbcommon.h>

/* ── Клавиатура ── */
typedef struct {
    struct wl_list      link;
    KronServer         *server;
    struct wlr_keyboard *wlr_keyboard;
    struct wl_listener  modifiers;
    struct wl_listener  key;
    struct wl_listener  destroy;
} KronKeyboard;

static bool handle_keybinding(KronServer *server, xkb_keysym_t sym) {
    switch (sym) {
    case XKB_KEY_Escape:
        wl_display_terminate(server->wl_display);
        return true;
    case XKB_KEY_F1: {
        /* Alt+F1: переключить фокус на следующее окно */
        if (wl_list_length(&server->toplevels) < 2) break;
        KronToplevel *next = wl_container_of(server->toplevels.prev, next, link);
        kron_toplevel_focus(next, next->xdg_toplevel->base->surface);
        return true;
    }
    default: break;
    }
    return false;
}

static void on_keyboard_modifiers(struct wl_listener *listener, void *data) {
    KronKeyboard *kb = wl_container_of(listener, kb, modifiers);
    wlr_seat_set_keyboard(kb->server->seat, kb->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(kb->server->seat, &kb->wlr_keyboard->modifiers);
}

static void on_keyboard_key(struct wl_listener *listener, void *data) {
    KronKeyboard *kb = wl_container_of(listener, kb, key);
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = kb->server->seat;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(kb->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t mods = wlr_keyboard_get_modifiers(kb->wlr_keyboard);
    if ((mods & WLR_MODIFIER_ALT) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED)
        for (int i = 0; i < nsyms; i++)
            handled = handle_keybinding(kb->server, syms[i]) || handled;

    if (!handled) {
        wlr_seat_set_keyboard(seat, kb->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
    }
}

static void on_keyboard_destroy(struct wl_listener *listener, void *data) {
    KronKeyboard *kb = wl_container_of(listener, kb, destroy);
    wl_list_remove(&kb->modifiers.link);
    wl_list_remove(&kb->key.link);
    wl_list_remove(&kb->destroy.link);
    wl_list_remove(&kb->link);
    free(kb);
}

static void add_keyboard(KronServer *server, struct wlr_input_device *device) {
    KronKeyboard *kb = calloc(1, sizeof(KronKeyboard));
    kb->server = server;
    kb->wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(ctx, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(kb->wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
    wlr_keyboard_set_repeat_info(kb->wlr_keyboard, 25, 600);

    kb->modifiers.notify = on_keyboard_modifiers;
    wl_signal_add(&kb->wlr_keyboard->events.modifiers, &kb->modifiers);
    kb->key.notify = on_keyboard_key;
    wl_signal_add(&kb->wlr_keyboard->events.key, &kb->key);
    kb->destroy.notify = on_keyboard_destroy;
    wl_signal_add(&device->events.destroy, &kb->destroy);

    wlr_seat_set_keyboard(server->seat, kb->wlr_keyboard);
}

/* ── Курсор / мышь ── */
static KronToplevel *toplevel_at(KronServer *server, double lx, double ly,
                                  struct wlr_surface **surface, double *sx, double *sy) {
    struct wlr_scene_node *node = wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
    if (!node || node->type != WLR_SCENE_NODE_BUFFER) return NULL;
    struct wlr_scene_buffer *sbuf = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *ssurface = wlr_scene_surface_try_from_buffer(sbuf);
    if (!ssurface) return NULL;
    *surface = ssurface->surface;
    struct wlr_scene_tree *tree = node->parent;
    while (tree && !tree->node.data) tree = tree->node.parent;
    return tree ? tree->node.data : NULL;
}

static void process_cursor_move(KronServer *server) {
    KronToplevel *tl = server->grabbed_toplevel;
    wlr_scene_node_set_position(&tl->scene_tree->node,
        server->cursor->x - server->grab_x,
        server->cursor->y - server->grab_y);
}

static void process_cursor_resize(KronServer *server) {
    KronToplevel *tl = server->grabbed_toplevel;
    double dx = server->cursor->x - server->grab_x;
    double dy = server->cursor->y - server->grab_y;
    struct wlr_box geo = server->grab_geobox;

    int new_left   = geo.x, new_right  = geo.x + geo.width;
    int new_top    = geo.y, new_bottom = geo.y + geo.height;

    if (server->resize_edges & WLR_EDGE_LEFT)        new_left   += dx;
    else if (server->resize_edges & WLR_EDGE_RIGHT)  new_right  += dx;
    if (server->resize_edges & WLR_EDGE_TOP)         new_top    += dy;
    else if (server->resize_edges & WLR_EDGE_BOTTOM) new_bottom += dy;

    double lx, ly;
    wlr_scene_node_coords(&tl->scene_tree->node, &lx, &ly);
    wlr_scene_node_set_position(&tl->scene_tree->node, new_left - (lx - geo.x), new_top - (ly - geo.y));
    wlr_xdg_toplevel_set_size(tl->xdg_toplevel, new_right - new_left, new_bottom - new_top);
}

static void process_cursor_motion(KronServer *server, uint32_t time) {
    if (server->cursor_mode == KRON_CURSOR_MOVE)   { process_cursor_move(server);   return; }
    if (server->cursor_mode == KRON_CURSOR_RESIZE) { process_cursor_resize(server); return; }

    double sx, sy;
    struct wlr_surface *surface = NULL;
    KronToplevel *tl = toplevel_at(server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (!tl)
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");

    if (surface) {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(server->seat);
    }
}

static void on_cursor_motion(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, cursor_motion);
    struct wlr_pointer_motion_event *event = data;
    wlr_cursor_move(server->cursor, &event->pointer->base, event->delta_x, event->delta_y);
    process_cursor_motion(server, event->time_msec);
}

static void on_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x, event->y);
    process_cursor_motion(server, event->time_msec);
}

static void on_cursor_button(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, cursor_button);
    struct wlr_pointer_button_event *event = data;
    wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);

    double sx, sy;
    struct wlr_surface *surface = NULL;
    KronToplevel *tl = toplevel_at(server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED)
        server->cursor_mode = KRON_CURSOR_PASSTHROUGH;
    else if (tl)
        kron_toplevel_focus(tl, surface);
}

static void on_cursor_axis(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    wlr_seat_pointer_notify_axis(server->seat, event->time_msec,
        event->orientation, event->delta, event->delta_discrete, event->source,
        event->relative_direction);
}

static void on_cursor_frame(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, cursor_frame);
    wlr_seat_pointer_notify_frame(server->seat);
}

/* ── Новое устройство ввода ── */
static void on_new_input(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        add_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        wlr_cursor_attach_input_device(server->cursor, device);
        break;
    default: break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->toplevels))
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    wlr_seat_set_capabilities(server->seat, caps);
}

static void on_request_cursor(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    if (server->seat->pointer_state.focused_client == event->seat_client)
        wlr_cursor_set_surface(server->cursor, event->surface, event->hotspot_x, event->hotspot_y);
}

static void on_request_set_selection(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

void kron_input_init(KronServer *server) {
    server->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    server->cursor_motion.notify           = on_cursor_motion;
    server->cursor_motion_absolute.notify  = on_cursor_motion_absolute;
    server->cursor_button.notify           = on_cursor_button;
    server->cursor_axis.notify             = on_cursor_axis;
    server->cursor_frame.notify            = on_cursor_frame;
    wl_signal_add(&server->cursor->events.motion,          &server->cursor_motion);
    wl_signal_add(&server->cursor->events.motion_absolute, &server->cursor_motion_absolute);
    wl_signal_add(&server->cursor->events.button,          &server->cursor_button);
    wl_signal_add(&server->cursor->events.axis,            &server->cursor_axis);
    wl_signal_add(&server->cursor->events.frame,           &server->cursor_frame);

    server->new_input.notify             = on_new_input;
    server->request_cursor.notify        = on_request_cursor;
    server->request_set_selection.notify = on_request_set_selection;
    wl_signal_add(&server->backend->events.new_input,              &server->new_input);
    wl_signal_add(&server->seat->events.request_set_cursor,        &server->request_cursor);
    wl_signal_add(&server->seat->events.request_set_selection,     &server->request_set_selection);
}
