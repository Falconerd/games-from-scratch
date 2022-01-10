#include <glad/glad.h>  
#include <SDL2/SDL.h>
#include <stdio.h>
#include "render/render.h"
#include "input/input.h"
#include "config/config.h"
#include "entity/entity.h"
#include "physics/physics.h"
#include "util/util.h"

#define WIDTH 800
#define HEIGHT 600

int main(void) {
    Config_State *config_state = config_init();
    Input_State *input_state = input_init();
    Entity_State *entity_state = entity_init();
    Physics_State *physics_state = physics_init();
    SDL_Window *window = render_init(WIDTH, HEIGHT);

    uint32_t player_index = entity_create((vec2){100.f, 100.f}, (vec2){100.f, 100.f}, physics_state->bodies);
    Entity *player = &entity_state->entities[player_index];

    uint32_t a_index = entity_create((vec2){80.f, 800.f}, (vec2){30.f, 30.f}, physics_state->bodies);
    Entity *a = &entity_state->entities[a_index];

    uint32_t b_index = entity_create((vec2){90.f, 840.f}, (vec2){30.f, 30.f}, physics_state->bodies);
    Entity *b = &entity_state->entities[b_index];

    uint32_t ground_index = physics_static_body_create((vec2){WIDTH / 2.f, 20.f}, (vec2){WIDTH * 0.8f, 40.0f});
    Static_Body *ground = &physics_state->static_bodies[ground_index];

    uint32_t wall_index = physics_static_body_create((vec2){WIDTH / 2.f, HEIGHT / 2.f}, (vec2){40.f, HEIGHT});
    Static_Body *wall = &physics_state->static_bodies[wall_index];

    SDL_Event e;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        input_update(config_state->keybinds);

        float velocity_x = 0;

        if (input_state->left) {
            velocity_x -= 70;
        }

        if (input_state->right) {
            velocity_x += 70;
        }

        glClearColor(0.2f, 0.8f, 0.9f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        physics_update(0.16f);

        vec2_add(player->body->aabb.position, player->body->aabb.position, (vec2){velocity_x, 0});

        render_aabb(&player->body->aabb, GREEN);
        render_aabb(&ground->aabb, WHITE);
        render_aabb(&wall->aabb, WHITE);
        render_aabb(&a->body->aabb, RED);
        render_aabb(&b->body->aabb, YELLOW);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
