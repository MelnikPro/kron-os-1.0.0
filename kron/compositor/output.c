#include "output.h"
#include "server.h"
#include "animation.h"
#include <stdlib.h>
#include <time.h>

static void on_frame(struct wl_listener *listener, void *data) {
    (void)data;
    KronOutput *output = wl_container_of(listener, output, frame);
    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(output->server->scene, output->wlr_output);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint32_t time_ms = (uint32_t)(now.tv_sec * 1000 + now.tv_nsec / 1000000);

    kron_anim_tick(time_ms);

    wlr_scene_output_commit(scene_output, NULL);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void on_request_state(struct wl_listener *listener, void *data) {
    KronOutput *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    wlr_output_commit_state(output->wlr_output, event->state);
}

static void on_destroy(struct wl_listener *listener, void *data) {
    KronOutput *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

void kron_output_init(KronServer *server, struct wlr_output *wlr_output) {
    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    /* Выбираем предпочтительный режим */
    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode) wlr_output_state_set_mode(&state, mode);
    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    KronOutput *output = calloc(1, sizeof(KronOutput));
    output->server = server;
    output->wlr_output = wlr_output;

    output->frame.notify = on_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->request_state.notify = on_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    output->destroy.notify = on_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);

    struct wlr_output_layout_output *layout_output =
        wlr_output_layout_add_auto(server->output_layout, wlr_output);
    output->scene_output = wlr_scene_output_create(server->scene, wlr_output);
    wlr_scene_output_layout_add_output(server->scene_layout, layout_output, output->scene_output);
}
