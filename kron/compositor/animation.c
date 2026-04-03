#include "animation.h"
#include <stdlib.h>
#include <math.h>
#include <wlr/types/wlr_scene.h>

#define MAX_ANIMS 64

static KronAnimation anims[MAX_ANIMS];
static int anim_count = 0;

/* ── Easing: cubic-bezier(0.34, 1.56, 0.64, 1) — spring ── */
static float ease_spring(float t) {
    /* Упрощённая аппроксимация spring easing */
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return 1.0f - (float)exp(-6.0f * t) * (float)cos(12.0f * t);
}

static float ease_out(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void kron_anim_open(struct wlr_scene_tree *tree) {
    if (anim_count >= MAX_ANIMS) return;
    KronAnimation *a = &anims[anim_count++];
    a->tree        = tree;
    a->type        = KRON_ANIM_OPEN;
    a->start_ms    = 0; /* будет установлен при первом тике */
    a->duration_ms = 300;
    a->from_scale  = 0.85f;
    a->to_scale    = 1.0f;
    a->from_alpha  = 0.0f;
    a->to_alpha    = 1.0f;
    a->done        = false;
    a->on_done     = NULL;
    a->data        = NULL;
}

void kron_anim_close(struct wlr_scene_tree *tree, void (*on_done)(void*), void *data) {
    if (anim_count >= MAX_ANIMS) { if (on_done) on_done(data); return; }
    KronAnimation *a = &anims[anim_count++];
    a->tree        = tree;
    a->type        = KRON_ANIM_CLOSE;
    a->start_ms    = 0;
    a->duration_ms = 200;
    a->from_scale  = 1.0f;
    a->to_scale    = 0.88f;
    a->from_alpha  = 1.0f;
    a->to_alpha    = 0.0f;
    a->done        = false;
    a->on_done     = on_done;
    a->data        = data;
}

void kron_anim_tick(uint32_t time_ms) {
    int i = 0;
    while (i < anim_count) {
        KronAnimation *a = &anims[i];

        if (a->start_ms == 0) a->start_ms = time_ms;

        float elapsed = (float)(time_ms - a->start_ms);
        float t = elapsed / (float)a->duration_ms;
        if (t > 1.0f) t = 1.0f;

        float eased = (a->type == KRON_ANIM_OPEN) ? ease_spring(t) : ease_out(t);
        float scale = lerp(a->from_scale, a->to_scale, eased);
        float alpha = lerp(a->from_alpha, a->to_alpha, eased);

        /* Применяем scale через scene node */
        wlr_scene_node_set_enabled(&a->tree->node, true);

        /* alpha через opacity */
        wlr_scene_node_set_opacity(&a->tree->node, alpha);

        /* scale — через transform если доступно */
        /* В wlroots 0.18 используем wlr_scene_node_set_transform */
        struct wlr_fbox transform = {
            .x = 0, .y = 0,
            .width = scale, .height = scale,
        };
        /* Центрируем scale */
        (void)transform; /* TODO: wlr_scene_node_set_transform когда API стабилизируется */

        if (t >= 1.0f) {
            a->done = true;
            if (a->on_done) a->on_done(a->data);
            /* Удаляем анимацию */
            anims[i] = anims[--anim_count];
            continue;
        }
        i++;
    }
}
