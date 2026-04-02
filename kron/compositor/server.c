#include "server.h"
#include "output.h"
#include "toplevel.h"
#include "input.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_foreign_toplevel_management_v1.h>
#include <wlr/types/wlr_output_management_v1.h>

static void on_new_output(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, new_output);
    kron_output_init(server, (struct wlr_output *)data);
}

/* ── xdg-decoration: говорим приложениям использовать серверные рамки ── */
static void on_new_xdg_decoration(struct wl_listener *listener, void *data) {
    struct wlr_xdg_toplevel_decoration_v1 *deco = data;
    wlr_xdg_toplevel_decoration_v1_set_mode(deco, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

/* ── output-management: применяем конфигурацию мониторов ── */
static void on_output_mgr_apply(struct wl_listener *listener, void *data) {
    KronServer *server = wl_container_of(listener, server, output_mgr_apply);
    struct wlr_output_configuration_v1 *config = data;
    /* применяем конфигурацию к каждому выходу */
    struct wlr_output_configuration_head_v1 *head;
    bool ok = true;
    wl_list_for_each(head, &config->heads, link) {
        struct wlr_output *output = head->state.output;
        struct wlr_output_state state;
        wlr_output_state_init(&state);
        wlr_output_state_set_enabled(&state, head->state.enabled);
        if (head->state.enabled && head->state.mode)
            wlr_output_state_set_mode(&state, head->state.mode);
        if (!wlr_output_commit_state(output, &state))
            ok = false;
        wlr_output_state_finish(&state);
    }
    if (ok) wlr_output_configuration_v1_send_succeeded(config);
    else    wlr_output_configuration_v1_send_failed(config);
    wlr_output_configuration_v1_destroy(config);
}

static void on_output_mgr_test(struct wl_listener *listener, void *data) {
    struct wlr_output_configuration_v1 *config = data;
    wlr_output_configuration_v1_send_succeeded(config);
    wlr_output_configuration_v1_destroy(config);
}

KronServer *kron_server_create(void) {
    KronServer *server = calloc(1, sizeof(KronServer));

    server->wl_display = wl_display_create();
    server->backend    = wlr_backend_autocreate(wl_display_get_event_loop(server->wl_display), NULL);
    server->renderer   = wlr_renderer_autocreate(server->backend);
    wlr_renderer_init_wl_display(server->renderer, server->wl_display);
    server->allocator  = wlr_allocator_autocreate(server->backend, server->renderer);

    /* Протоколы */
    wlr_compositor_create(server->wl_display, 5, server->renderer);
    wlr_subcompositor_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);

    /* Сцена */
    server->scene        = wlr_scene_create();
    server->output_layout = wlr_output_layout_create(server->wl_display);
    server->scene_layout  = wlr_scene_attach_output_layout(server->scene, server->output_layout);

    /* Мониторы */
    wl_list_init(&server->outputs);
    server->new_output.notify = on_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);

    /* XDG shell */
    wl_list_init(&server->toplevels);
    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
    kron_xdg_shell_init(server);

    /* Layer shell (для панели KRON shell) */
    server->layer_shell = wlr_layer_shell_v1_create(server->wl_display, 4);

    /* xdg-decoration: серверные рамки окон */
    server->xdg_decoration_mgr = wlr_xdg_decoration_manager_v1_create(server->wl_display);
    server->new_xdg_decoration.notify = on_new_xdg_decoration;
    wl_signal_add(&server->xdg_decoration_mgr->events.new_toplevel_decoration,
                  &server->new_xdg_decoration);

    /* foreign-toplevel: taskbar */
    server->foreign_toplevel_mgr = wlr_foreign_toplevel_manager_v1_create(server->wl_display);

    /* output-management: настройка мониторов через wlr-randr и т.д. */
    server->output_mgr = wlr_output_manager_v1_create(server->wl_display);
    server->output_mgr_apply.notify = on_output_mgr_apply;
    server->output_mgr_test.notify  = on_output_mgr_test;
    wl_signal_add(&server->output_mgr->events.apply, &server->output_mgr_apply);
    wl_signal_add(&server->output_mgr->events.test,  &server->output_mgr_test);

    /* Seat */
    server->seat = wlr_seat_create(server->wl_display, "seat0");
    kron_input_init(server);

    return server;
}

int kron_server_run(KronServer *server) {
    const char *socket = wl_display_add_socket_auto(server->wl_display);
    if (!socket) {
        wlr_log(WLR_ERROR, "Не удалось создать Wayland socket");
        return 1;
    }

    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Не удалось запустить backend");
        return 1;
    }

    setenv("WAYLAND_DISPLAY", socket, 1);
    wlr_log(WLR_INFO, "KRON compositor запущен на %s", socket);

    /* Запускаем KRON shell */
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "kron-shell", NULL);
        _exit(1);
    }

    wl_display_run(server->wl_display);
    return 0;
}

void kron_server_destroy(KronServer *server) {
    wl_display_destroy_clients(server->wl_display);
    wlr_backend_destroy(server->backend);
    wlr_output_layout_destroy(server->output_layout);
    wl_display_destroy(server->wl_display);
    free(server);
}
