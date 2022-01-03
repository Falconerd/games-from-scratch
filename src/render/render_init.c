#include <glad/glad.h>
#include "render_internal.h"

void render_init_window(SDL_Window **window, float width, float height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not init SDL\n");
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    *window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    if (!*window) {
        printf("Failed to init window: %s\n", SDL_GetError());
        exit(1);
    }
}

void render_init_context(SDL_Window **window) {
    SDL_GL_CreateContext(*window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to init GLAD\n");
        exit(1);
    }

    printf("OpenGL Loaded\n");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
}

void render_init_shaders(uint32_t *default_shader, uint32_t *text_shader, uint32_t *circle_shader, mat4x4 projection, float width, float height) {
    *default_shader = render_shader_create("./shaders/default.vert", "./shaders/default.frag");
    // *text_shader = render_shader_create("./shaders/text.vert", "./shaders/text.frag");
    // *circle_shader = render_shader_create("./shaders/circle.vert", "./shaders/circle.frag");

    mat4x4_ortho(projection, 0, width, 0, height, -2.0f, 2.0f);

    glUseProgram(*default_shader);
    glUniformMatrix4fv(glGetUniformLocation(*default_shader, "projection"), 1, GL_FALSE, &projection[0][0]);
}

void render_init_color_texture(uint32_t *color_texture) {
    glGenTextures(1, color_texture);
    render_texture_setup(*color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (uint8_t[4]){255, 255, 255, 255});
}

void render_init_quad(uint32_t *quad_vao, uint32_t *quad_vbo, uint32_t *quad_ebo) {
    float vertices[] = {
         0.5f,  0.5f, 0, 1.0f, 1.0f,
         0.5f, -0.5f, 0, 1.0f, 0.0f,
        -0.5f, -0.5f, 0, 0.0f, 0.0f,
        -0.5f,  0.5f, 0, 0.0f, 1.0f
    };
    uint32_t indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, quad_vao);
    glGenBuffers(1, quad_vbo);
    glGenBuffers(1, quad_ebo);

    glBindVertexArray(*quad_vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, *quad_vao);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *quad_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
}