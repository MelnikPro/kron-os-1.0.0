#pragma once

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>
#include <wayland-server-core.h>

struct kron_server;

typedef struct kron_toplevel {
    struct wl_list              link;
    struct kron_server         *server;
    struct wlr_xdg_toplevel    *xdg_toplevel;
    struct wlr_scene_tree      *scene_tree;

    /* foreign-toplevel handle — taskbar видит это окно */
    struct wlr_foreign_toplevel_handle_v1 *foreign_handle;

    struct wl_listener          map;
    struct wl_listener          unmap;
    struct wl_listener          commit;
    struct wl_listener          destroy;
    struct wl_listener          request_move;
    struct wl_listener          request_resize;
    struct wl_listener          request_maximize;
} KronToplevel;

void kron_toplevel_focus(KronToplevel *toplevel, struct wlr_surface *surface);
void kron_toplevel_begin_move(KronToplevel *toplevel, uint32_t time_msec);
void kron_toplevel_begin_resize(KronToplevel *toplevel, uint32_t edges, uint32_t time_msec);
void kron_xdg_shell_init(struct kron_server *server);
