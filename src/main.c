#define NOB_IMPLEMENTATION
#include "nob.h"
#include "raylib.h"
#include "raymath.h"

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

#define Entity_Id uint32_t
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
    size_t current;
} Waves;

bool is_wave_over(AWave *wave)
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
    {.health = 1},
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

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_MAXIMIZED);
    InitWindow(0, 0, "Ray Tato Game in Raylib");

    SetTargetFPS(60);

    int renderWidth = GetRenderWidth();
    int renderHeight = GetRenderHeight();

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

    Nob_String_Builder sb = {0};

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

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

        AWave *wave = &waves.items[waves.current];

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

        if (is_wave_over(wave))
        {
            waves.current++;
        }

        camera.target = (Vector2){player.x + player.width / 2.0f, player.y + player.height / 2.0f};

        BeginDrawing();
        {
            ClearBackground(BLACK);

            sb.count = 0;
            nob_sb_appendf(&sb, "Wave: %zu\n", waves.current);

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
