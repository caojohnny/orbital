#ifndef ORBITAL_SHADER_UTIL_H
#define ORBITAL_SHADER_UTIL_H

#include <GL/glew.h>

char *get_shader_src(const char *path);

int compile_shader(GLenum type, const char *shader_path, GLuint *shader_id);

int create_prog(const char *vs_path, const char *fs_path, GLuint *prog_id);

#endif // ORBITAL_SHADER_UTIL_H
