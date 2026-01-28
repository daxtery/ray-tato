#define NOB_IMPLEMENTATION
#include "nob.h"
#include "raylib.h"
#include "raymath.h"

#include "stdbool.h"
#include "stdint.h"

typedef enum
{
    When_Tick_Ends_Restart = 1,
    When_Tick_Ends_Keep = 0,
} TickEndBehaviour;

typedef struct
{
    uint16_t ms_to_trigger;
    uint16_t ms_accumulated;
} Accumulator;

static Accumulator make_accumulator(uint16_t ms_to_trigger)
{
    Accumulator accumulator = {
        .ms_to_trigger = ms_to_trigger,
        .ms_accumulated = 0,
    };
    return accumulator;
}

static void accumulator_reset(Accumulator *accumulator)
{
    accumulator->ms_accumulated = 0;
}

static bool accumulator_tick(Accumulator *accumulator, float dt, TickEndBehaviour when_tick_ends)
{
    if (accumulator->ms_accumulated > accumulator->ms_to_trigger)
    {
        if (when_tick_ends == When_Tick_Ends_Restart)
        {
            accumulator->ms_accumulated = 0;
        }
        return true;
    }

    float add = dt * 1000.0;
    accumulator->ms_accumulated += add;
    return false;
}

typedef struct
{
    Rectangle *player;
    Rectangle bounds;
} PlayerControllerOptions;

static void player_move(PlayerControllerOptions *this, const Vector2 direction)
{
    if (direction.x == 0 && direction.y == 0)
    {
        return;
    }

    Vector2 new_position = Vector2Add(
        (Vector2){
            .x = this->player->x,
            .y = this->player->y,
        },
        direction);

    new_position = Vector2Clamp(new_position,
                                (Vector2){
                                    .x = this->bounds.x,
                                    .y = this->bounds.y,
                                },
                                (Vector2){
                                    .x = this->bounds.x + this->bounds.width - this->player->width,
                                    .y = this->bounds.y + this->bounds.height - this->player->height,
                                });

    this->player->x = new_position.x;
    this->player->y = new_position.y;
}

typedef struct
{
    size_t health;
} Enemy;

typedef struct
{
    size_t count;
    Enemy *items;
} AWave;

#define make_wave(its)                                                                                                 \
    {                                                                                                                  \
        .items = (its),                                                                                                \
        .count = NOB_ARRAY_LEN((its)),                                                                                 \
    }

typedef struct
{
    AWave *items;
    size_t count;
    size_t capacity;
} Waves;

typedef struct {
    Waves waves;
    size_t current;
} WavesManager;

static bool is_wave_over(AWave *wave)
{
    nob_da_foreach(Enemy, it, wave)
    {
        if (it->health > 0)
        {
            return false;
        }
    }
    return true;
}

static Enemy wave_1_enemies[] = {
    {
        .health = 1,
    },
};

static Enemy wave_2_enemies[] = {
    {
        .health = 1,
    },
    {
        .health = 1,
    },
};

static Enemy wave_3_enemies[] = {
    {
        .health = 1,
    },
    {
        .health = 1,
    },
};

typedef enum {
    WAVE_GOING,
    WAVE_TRANSITION,
    WAVE_DONE,
} WaveResult;

static WaveResult waves_update(WavesManager *this)
{
    Waves *waves = &this->waves;
    AWave *wave = &waves->items[this->current];

    if (!is_wave_over(wave))
    {
        return WAVE_GOING;
    }

    if ((this->current + 1) == waves->count)
    {
        return WAVE_DONE;
    }

    this->current++;
    return WAVE_TRANSITION;
}

static AWave* waves_get_current(WavesManager *this)
{
    Waves *waves = &this->waves;
    AWave *wave = &waves->items[this->current];

    return wave;
}

typedef struct {
   bool in_transition;
}  GameState;

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_MAXIMIZED);
    InitWindow(0, 0, "Ray Tato Game in Raylib");

    SetTargetFPS(60);

    int renderWidth = GetRenderWidth();
    int renderHeight = GetRenderHeight();

    GameState state = {
        .in_transition = false
    };

    Rectangle player = {400, 280, 40, 40};

    PlayerControllerOptions controller_options = {
        .bounds =
            {
                .x = 0,
                .y = 0,
                .height = 800,
                .width = 1000,
            },
        .player = &player,
    };

    Camera2D camera = {
        .target = (Vector2){player.x + player.width / 2.0f, player.y + player.height / 2.0f},
        .offset = (Vector2){renderWidth / 2.0f, renderHeight / 2.0f},
        .rotation = 0.0f,
        .zoom = 1.0f,
    };

    Waves waves = {0};
    {
        nob_da_append(&waves, (AWave)make_wave(wave_1_enemies));
        nob_da_append(&waves, (AWave)make_wave(wave_2_enemies));
        nob_da_append(&waves, (AWave)make_wave(wave_3_enemies));
    };

    WavesManager waves_manager = {
        .waves = waves,
        .current = 0,
    };

    Accumulator accumulator = make_accumulator(5000);

    Nob_String_Builder sb = {0};

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (state.in_transition)
        {
            if (accumulator_tick(&accumulator, dt, When_Tick_Ends_Restart))
            {
                state.in_transition = false;
            }
        }
        else
        {
            Vector2 direction = {0};

            const int16_t speed = 300;

            if (IsKeyDown(KEY_W))
                direction.y = -speed * dt;
            else if (IsKeyDown(KEY_S))
                direction.y = speed * dt;
            if (IsKeyDown(KEY_D))
                direction.x = speed * dt;
            else if (IsKeyDown(KEY_A))
                direction.x = -speed * dt;

            player_move(&controller_options, direction);
            camera.target = (Vector2){player.x + player.width / 2.0f, player.y + player.height / 2.0f};

            AWave *wave = waves_get_current(&waves_manager);

            if (IsKeyPressed(KEY_SPACE))
            {
                nob_da_foreach(Enemy, it, wave)
                {
                    if (it->health > 0)
                    {
                        it->health = 0;
                        break;
                    }
                }
            }

            {
                WaveResult result = waves_update(&waves_manager);
                if (result == WAVE_TRANSITION)
                {
                    accumulator_reset(&accumulator);
                    state.in_transition = true;
                }
            }
        }

        BeginDrawing();
        {
            ClearBackground(BLACK);

            sb.count = 0;
            nob_sb_appendf(&sb, "Wave: %zu\n", waves_manager.current);

            AWave *wave = waves_get_current(&waves_manager);

            nob_da_foreach(Enemy, it, wave)
            {
                if (it->health > 0)
                {
                    nob_sb_appendf(&sb, "Enemy: %zu\n", it->health);
                }
                else
                {
                    nob_sb_appendf(&sb, "Enemy: <dead>\n");
                }
            }

            nob_sb_append_null(&sb);
            DrawText(sb.items, 100, 100, 15, WHITE);

            if (state.in_transition)
            {
                DrawText("TRANSITION", 200, 200, 25, WHITE);
            }

            BeginMode2D(camera);
            {
                DrawRectangle(0, 0, 1000, 800, DARKGRAY);
                DrawRectangleRec(player, RED);
            }
            EndMode2D();
        }
        EndDrawing();

        nob_temp_reset();
    }
}
