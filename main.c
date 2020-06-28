#include <SDL.h>
#include <GL/glew.h>
#include <time.h>
#include <cglm/cglm.h>
#include "shader_util.h"

static const double TWO_PI = 2.0 * M_PI;
static const int CIRCLE_DIVISIONS = 100;
static const long MS_PER_SEC = 1000;
static const long NS_PER_MS = 1000000;
static const long LOOP_DELAY_MS = 50;

struct gl_shader_wrapper {
    GLuint prog;
    GLuint vbo;
    GLuint vao;

    int n_points;
};

static struct gl_shader_wrapper circle;

static int create_circle(struct gl_shader_wrapper *out) {
    GLuint prog;
    if (!create_prog("./shaders/vs-fixed.glsl", "./shaders/fs-fixed.glsl", &prog)) {
        return 0;
    }

    glUseProgram(prog);

    GLuint vbo;
    GLuint vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    // Stole this code from: https://stackoverflow.com/questions/22444450/drawing-circle-with-opengl/24843626#24843626
    float circle_x = 0;
    float circle_y = 0;
    float circle_r = 0.2F;
    int n_points = CIRCLE_DIVISIONS + 1;
    size_t circle_array_size = 2 * n_points * sizeof(float);
    float *circle_arr = malloc(circle_array_size);
    for (int i = 0, arr_idx = 0; i <= CIRCLE_DIVISIONS; i++, arr_idx += 2) {
        float vx = circle_x + circle_r * (float) cos(i * TWO_PI / CIRCLE_DIVISIONS);
        float vy = circle_y + circle_r * (float) sin(i * TWO_PI / CIRCLE_DIVISIONS);

        circle_arr[arr_idx] = vx;
        circle_arr[arr_idx + 1] = vy;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, circle_array_size, circle_arr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    free(circle_arr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLint in_color = glGetUniformLocation(prog, "in_color");
    glUniform4f(in_color, 1.0, 1.0, 1.0, 1.0);

    struct gl_shader_wrapper wrapper = {prog, vbo, vao, n_points};
    *out = wrapper;

    return 1;
}

static int init_opengl(SDL_Window *win) {
    SDL_GLContext gl_ctx = SDL_GL_CreateContext(win);
    if (gl_ctx == NULL) {
        fprintf(stderr, "Failed to initialize OpenGL context: %s\n", SDL_GetError());
        return 0;
    }

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "Failed to initialize OpenGL: %s\n", gluErrorString(error));
        return 0;
    }

    GLenum glew_ec = glewInit();
    if (glew_ec != GLEW_OK) {
        fprintf(stderr, "Failed to init GLEW: %s\n", glewGetErrorString(glew_ec));
        return 0;
    }

    if (!create_circle(&circle)) {
        return 0;
    }

    return 1;
}

static void handle_key_press(SDL_Event *ev) {
    SDL_KeyboardEvent key_ev = ev->key;
    SDL_Keysym key_info = key_ev.keysym;
    SDL_Scancode sc = key_info.scancode;

    switch (sc) {
        case SDL_SCANCODE_LEFT:
            printf("Pressed 'left'\n");
            break;
        case SDL_SCANCODE_UP:
            printf("Pressed 'up'\n");
            break;
        case SDL_SCANCODE_RIGHT:
            printf("Pressed 'right'\n");
            break;
        case SDL_SCANCODE_DOWN:
            printf("Pressed 'down'\n");
            break;
        case SDL_SCANCODE_R:
            printf("Pressed 'r'\n");
            break;
        default:
            break;
    }
}

static void resized(int w, int h) {
    glViewport(0, 0, w, h);
}

static void handle_window_event(SDL_Event *ev) {
    SDL_WindowEvent window_ev = ev->window;
    Uint8 type = window_ev.event;
    switch (type) {
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
            Sint32 w = window_ev.data1;
            Sint32 h = window_ev.data2;
            printf("New window size: %dx%d\n", w, h);
            resized(w, h);
            break;
        }
        default:
            break;
    }
}

static void handle_events(int *close) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        Uint32 ev_type = ev.type;
        switch (ev_type) {
            case SDL_QUIT:
                *close = 1;
                return;
            case SDL_KEYDOWN:
                handle_key_press(&ev);
                break;
            case SDL_WINDOWEVENT:
                handle_window_event(&ev);
                break;
            default:
                break;
        }
    }
}

static void draw_rocket(int x, int y) {
    int radius = 10;
    int body_len = 20;
    int nose_len = 10;

    /*
     *    5     -
     *  /   \   |  nose_len
     * 4  0  1  -
     * |     |  |  body_len
     * |     |  |
     * 3_____2  -
     */
    glBegin(GL_POLYGON);
    glVertex2f(x + radius, y);
    glVertex2f(x + radius, y - body_len);
    glVertex2f(x - radius, y - body_len);
    glVertex2f(x - radius, y);
    glVertex2f(x, y + nose_len);
    glEnd();
}

static void draw_flame(int x, int y) {
    int half_w = 4;
    int h = 10;

    glBegin(GL_TRIANGLES);
    glVertex2f(x - half_w, y);
    glVertex2f(x + half_w, y);
    glVertex2f(x, y - h);
    glEnd();
}

static void update() {
}

static void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    /* glColor3f(0.0, 0.4, 1.0);
    draw_circle(100, 0, 0);

    glColor3f(1.0, 1.0, 1.0);
    draw_rocket(150, 0);

    glColor3f(1.0, 0.0, 0.0);
    draw_flame(150, 0 - 20); */

    glUseProgram(circle.prog);
    glBindVertexArray(circle.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circle.n_points);
}

static void run_process_loop(SDL_Window *win) {
    while (1) {
        struct timespec begin;
        clock_gettime(CLOCK_REALTIME, &begin);

        int close = 0;
        handle_events(&close);

        if (close) {
            break;
        }

        update();
        render();
        SDL_GL_SwapWindow(win);

        struct timespec end;
        clock_gettime(CLOCK_REALTIME, &end);

        long elapsed = ((end.tv_sec - begin.tv_sec) * MS_PER_SEC) +
                       ((end.tv_nsec - begin.tv_nsec) / NS_PER_MS);
        long delay = LOOP_DELAY_MS - elapsed;
        if (delay <= 0) {
            continue;
        }
        SDL_Delay(delay);
    }
}

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    printf("Initialized SDL\n");

    int initial_w = 1000;
    int initial_h = 1000;
    SDL_Window *win = SDL_CreateWindow("Orbital", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       initial_w, initial_h, SDL_WINDOW_OPENGL);
    if (!win) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    printf("Opened window\n");

    if (!init_opengl(win)) {
        return EXIT_FAILURE;
    }

    printf("Initialized OpenGL\n");

    resized(initial_w, initial_h);
    run_process_loop(win);

    glDeleteVertexArrays(1, &circle.vao);
    glDeleteBuffers(1, &circle.vbo);
    glDeleteProgram(circle.prog);

    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
