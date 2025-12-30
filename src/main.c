#define NOB_IMPLEMENTATION
#include "nob.h"
#include "raylib.h"
#include "raymath.h"

typedef struct
{
    Rectangle bounds;
} PlayerControllerOptions;

void player_move(Rectangle *player, const PlayerControllerOptions options, const Vector2 direction)
{
    if (direction.x == 0 && direction.y == 0)
    {
        return;
    }

    Vector2 new_position = Vector2Add(
        (Vector2){
            .x = player->x,
            .y = player->y,
        },
        direction);

    new_position = Vector2Clamp(new_position,
                                (Vector2){
                                    .x = options.bounds.x,
                                    .y = options.bounds.y,
                                },
                                (Vector2){
                                    .x = options.bounds.x + options.bounds.width - player->width,
                                    .y = options.bounds.y + options.bounds.height - player->height,
                                });

    player->x = new_position.x;
    player->y = new_position.y;
}

int main(void)
{
    InitWindow(800, 600, "Ray Tato Game in Raylib");
    MaximizeWindow();

    SetTargetFPS(60);

    srand(time(NULL));

    Rectangle player = {400, 280, 40, 40};

    PlayerControllerOptions controller_options = {
        .bounds =
            {
                .x = 0,
                .y = 0,
                .height = 800,
                .width = 1000,
            },
    };

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    Camera2D camera = {0};
    camera.target = (Vector2){player.x + player.width / 2.0f, player.y + player.height / 2.0f};
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        Vector2 direction = {0};

        const int16_t speed = 300;

        if (IsKeyDown(KEY_W))
            direction.y = -speed * dt;
        else if (IsKeyDown(KEY_S))
            direction.y = speed * dt;
        else if (IsKeyDown(KEY_D))
            direction.x = speed * dt;
        else if (IsKeyDown(KEY_A))
            direction.x = -speed * dt;

        player_move(&player, controller_options, direction);

        camera.target = (Vector2){player.x + player.width / 2.0f, player.y + player.height / 2.0f};

        BeginDrawing();
        {
            ClearBackground(BLACK);

            BeginMode2D(camera);
            {
                DrawRectangle(0, 0, 1000, 800, DARKGRAY);
                DrawRectangleRec(player, RED);
            }
            EndMode2D();
        }
        EndDrawing();
    }
}
