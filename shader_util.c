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

