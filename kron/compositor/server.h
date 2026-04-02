#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

struct kron_toplevel;

typedef struct kron_server {
    struct wl_display              *wl_display;
    struct wlr_backend             *backend;
    struct wlr_renderer            *renderer;
    struct wlr_allocator           *allocator;
    struct wlr_scene               *scene;
    struct wlr_scene_output_layout *scene_layout;

    /* xdg-shell: обычные окна */
    struct wlr_xdg_shell           *xdg_shell;
    struct wl_listener              new_xdg_toplevel;
    struct wl_listener              new_xdg_popup;

    /* layer-shell: панель, лаунчер */
    struct wlr_layer_shell_v1      *layer_shell;
    struct wl_listener              new_layer_surface;

    /* курсор */
    struct wlr_cursor              *cursor;
    struct wlr_xcursor_manager     *cursor_mgr;
    struct wl_listener              cursor_motion;
    struct wl_listener              cursor_motion_absolute;
    struct wl_listener              cursor_button;
    struct wl_listener              cursor_axis;
    struct wl_listener              cursor_frame;

    /* seat (клавиатура + мышь) */
    struct wlr_seat                *seat;
    struct wl_listener              new_input;
    struct wl_listener              request_cursor;
    struct wl_listener              request_set_selection;

    /* мониторы */
    struct wlr_output_layout       *output_layout;
    struct wl_list                  outputs;
    struct wl_listener              new_output;

    /* список окон */
    struct wl_list                  toplevels;

    /* xdg-decoration: серверные рамки окон */
    struct wlr_xdg_decoration_manager_v1 *xdg_decoration_mgr;
    struct wl_listener              new_xdg_decoration;

    /* foreign-toplevel: taskbar видит открытые окна */
    struct wlr_foreign_toplevel_manager_v1 *foreign_toplevel_mgr;

    /* output-management: настройка мониторов */
    struct wlr_output_manager_v1   *output_mgr;
    struct wl_listener              output_mgr_apply;
    struct wl_listener              output_mgr_test;

    /* drag/resize состояние */
    enum { KRON_CURSOR_PASSTHROUGH, KRON_CURSOR_MOVE, KRON_CURSOR_RESIZE } cursor_mode;
    struct kron_toplevel           *grabbed_toplevel;
    double                          grab_x, grab_y;
    struct wlr_box                  grab_geobox;
    uint32_t                        resize_edges;
} KronServer;

KronServer *kron_server_create(void);
int         kron_server_run(KronServer *s);
void        kron_server_destroy(KronServer *s);
