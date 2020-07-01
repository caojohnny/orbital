#include "shader_util.h"

#include <SDL.h>
#include <errno.h>

char *get_shader_src(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "No such file '%s'\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek to the end: %s\n", strerror(errno));
        fclose(file);
        return NULL;
    }

    long length = ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to the end: %s\n", strerror(errno));
        fclose(file);
        return NULL;
    }

    char *buf = malloc((length + 1) * sizeof(*buf));
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return NULL;
    }

    if (fread(buf, sizeof(*buf), length, file) != length) {
        fprintf(stderr, "Failed to read\n");
        free(buf);
        fclose(file);
        return NULL;
    }

    fclose(file);

    *(buf + length) = '\0';

    return buf;
}

int compile_shader(GLenum type, const char *shader_path, GLuint *shader_id) {
    GLuint shader = glCreateShader(type);
    char *src = get_shader_src(shader_path);
    if (!src) {
        return 0;
    }

    glShaderSource(shader, 1, &src, NULL);
    free(src);

    glCompileShader(shader);

    GLint compile_ec = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ec);
    if (compile_ec != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, log);
        fprintf(stderr, "Failed to compile shader: %s\n", log);

        return 0;
    }

    *shader_id = shader;
    return 1;
}

int create_prog(const char *vs_path, const char *fs_path, GLuint *prog_id) {
    GLuint vs;
    if (!compile_shader(GL_VERTEX_SHADER, vs_path, &vs)) {
        return 0;
    }

    GLuint fs;
    if (!compile_shader(GL_FRAGMENT_SHADER, fs_path, &fs)) {
        return 0;
    }

    GLuint prog = glCreateProgram();
    glUseProgram(prog);

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint prog_ec = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &prog_ec);
    if (prog_ec != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog(vs, 1024, NULL, log);
        fprintf(stderr, "Failed to link the shader pipeline: %s\n", log);

        return 0;
    }

    *prog_id = prog;
    return 1;
}

int bind_shader(const char *vs_src, const char *fs_src, struct gl_shader_wrapper *out) {
    GLuint prog;
    if (!create_prog(vs_src, fs_src, &prog)) {
        return 0;
    }

    glUseProgram(prog);

    GLuint vbo;
    GLuint vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    struct gl_shader_wrapper wrapper = {prog, vbo, vao, 0};
    *out = wrapper;

    return 1;
}

void buffer_data_2f(struct gl_shader_wrapper *wrapper, int n_points, size_t arr_len, const float *arr) {
    glUseProgram(wrapper->prog);

    glBindVertexArray(wrapper->vao);
    glBindBuffer(GL_ARRAY_BUFFER, wrapper->vbo);

    glBufferData(GL_ARRAY_BUFFER, arr_len, arr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    wrapper->n_points = n_points;
}

void draw_shader_arrays(struct gl_shader_wrapper *wrapper, GLenum mode) {
    glUseProgram(wrapper->prog);
    glBindVertexArray(wrapper->vao);
    glDrawArrays(mode, 0, wrapper->n_points);
}

void destroy_shader(struct gl_shader_wrapper *wrapper) {
    glDeleteVertexArrays(1, &wrapper->vao);
    glDeleteBuffers(1, &wrapper->vbo);
    glDeleteProgram(wrapper->prog);
}
