#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <linmath.h>
#include "render_internal.h"

static uint32_t default_shader;
static mat4x4 projection;
static uint32_t quad_vao, quad_vbo, quad_ebo;
static uint32_t color_texture;

SDL_Window *render_init(float width, float height) {
    SDL_Window *window = render_init_window(width, height);

    render_init_context(window);
    render_init_shaders(&default_shader, projection, width, height);
    render_init_quad(&quad_vao, &quad_vbo, &quad_ebo);
    render_init_color_texture(&color_texture);

    return window;
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, pos[0] + size[0] * 0.5f, pos[1] + size[1] * 0.5f, 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 1);

    glUniformMatrix4fv(glGetUniformLocation(default_shader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform4fv(glGetUniformLocation(default_shader, "color"), 1, color);

    glBindVertexArray(quad_vao);
    {
        glBindTexture(GL_TEXTURE_2D, color_texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    }
    glBindVertexArray(0);
}
