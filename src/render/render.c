#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <linmath.h>
#include "render_internal.h"

static uint32_t default_shader;
static mat4x4 projection;
static uint32_t quad_vao, quad_vbo, quad_ebo;
static uint32_t line_vao, line_vbo;
static uint32_t color_texture;

SDL_Window *render_init(float width, float height) {
    SDL_Window *window = render_init_window(width, height);

    render_init_context(window);
    render_init_shaders(&default_shader, projection, width, height);
    render_init_quad(&quad_vao, &quad_vbo, &quad_ebo);
    render_init_line(&line_vao, &line_vbo);
    render_init_color_texture(&color_texture);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

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

void render_aabb(void *aabb, vec4 color) {
    float *pos = (float*)aabb;
    float *size = pos+2;
    vec2 q;
    vec2 r;

    vec2_sub(q, pos, size);
    vec2_scale(r, size, 2.f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    render_quad(q, r, color);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    render_quad(pos, (vec2){2.f, 2.f}, (vec4){0.f, 1.f, 0.f, 1.f});

    vec2 min = {pos[0] - size[0] - 1.f, pos[1] - size[1] - 1.f};
    vec2 max = {pos[0] + size[0] - 1.f, pos[1] + size[1] - 1.f};

    render_quad(min, (vec2){2.f, 2.f}, (vec4){1.f, 0.f, 0.f, 1.f});
    render_quad(max, (vec2){2.f, 2.f}, (vec4){1.f, 1.f, 0.f, 1.f});
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
    float x = end[0] - start[0];
    float y = end[1] - start[1];
    float line[6] = {0.f, 0.f, 0.f, x, y, 0.f};
    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, start[0], start[1], 0.f);

    glUniformMatrix4fv(glGetUniformLocation(default_shader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform4fv(glGetUniformLocation(default_shader, "color"), 1, color);

    glBindTexture(GL_TEXTURE_2D, color_texture);
    glBindVertexArray(line_vao);
    {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof line), &line);
        glDrawArrays(GL_LINES, 0, 2);
        glDisableVertexAttribArray(0);
    }
    glBindVertexArray(0);
}
