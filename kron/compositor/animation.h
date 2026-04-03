#pragma once
#include <wlr/types/wlr_scene.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    KRON_ANIM_OPEN,
    KRON_ANIM_CLOSE,
    KRON_ANIM_MINIMIZE,
} KronAnimType;

typedef struct kron_animation {
    struct wlr_scene_tree *tree;
    KronAnimType           type;
    uint32_t               start_ms;
    uint32_t               duration_ms;
    float                  from_scale;
    float                  to_scale;
    float                  from_alpha;
    float                  to_alpha;
    bool                   done;
    void                 (*on_done)(void *data);
    void                  *data;
} KronAnimation;

/* Запустить анимацию открытия окна */
void kron_anim_open    (struct wlr_scene_tree *tree);
/* Запустить анимацию закрытия, вызывает on_done когда завершится */
void kron_anim_close   (struct wlr_scene_tree *tree, void (*on_done)(void*), void *data);
/* Тик анимаций — вызывать каждый кадр */
void kron_anim_tick    (uint32_t time_ms);
