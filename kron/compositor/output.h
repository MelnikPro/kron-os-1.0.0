#pragma once

#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <wayland-server-core.h>

struct kron_server;

typedef struct kron_output {
    struct wl_list          link;
    struct kron_server     *server;
    struct wlr_output      *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wl_listener      frame;
    struct wl_listener      request_state;
    struct wl_listener      destroy;
} KronOutput;

void kron_output_init(struct kron_server *server, struct wlr_output *wlr_output);
