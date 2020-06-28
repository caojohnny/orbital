#ifndef ORBITAL_SHADER_UTIL_H
#define ORBITAL_SHADER_UTIL_H

#include <GL/glew.h>

struct gl_shader_wrapper {
    GLuint prog;
    GLuint vbo;
    GLuint vao;

    int n_points;
};

char *get_shader_src(const char *path);

int compile_shader(GLenum type, const char *shader_path, GLuint *shader_id);

int create_prog(const char *vs_path, const char *fs_path, GLuint *prog_id);

int bind_shader(const char *vs_src, const char *fs_src, struct gl_shader_wrapper *out);

void buffer_data_2f(struct gl_shader_wrapper *wrapper, int n_points, size_t arr_len, const float *arr);

void draw_shader_arrays(struct gl_shader_wrapper *wrapper, GLenum mode);

void destroy_shader(struct gl_shader_wrapper *wrapper);

#endif // ORBITAL_SHADER_UTIL_H
