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

static struct gl_shader_wrapper circle;
static struct gl_shader_wrapper rocket;
static struct gl_shader_wrapper flame;

static int init_circle(struct gl_shader_wrapper *wrapper) {
    if (!bind_shader("./shaders/vs-fixed.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    // Stole this code from: https://stackoverflow.com/questions/22444450/drawing-circle-with-opengl/24843626#24843626
    float x = 0.0F;
    float y = 0.0F;
    float r = 0.2F;

    int n_points = CIRCLE_DIVISIONS + 1;
    size_t len = 2 * n_points * sizeof(float);
    float *arr = malloc(len);
    for (int i = 0, arr_idx = 0; i <= CIRCLE_DIVISIONS; i++, arr_idx += 2) {
        float vx = x + r * (float) cos(i * TWO_PI / CIRCLE_DIVISIONS);
        float vy = y + r * (float) sin(i * TWO_PI / CIRCLE_DIVISIONS);

        arr[arr_idx] = vx;
        arr[arr_idx + 1] = vy;
    }

    buffer_data_2f(wrapper, n_points, len, arr);
    free(arr);

    GLint in_color = glGetUniformLocation(wrapper->prog, "in_color");
    glUniform4f(in_color, 0.0F, 0.4F, 1.0F, 1.0F);

    return 1;
}

static int init_rocket(struct gl_shader_wrapper *wrapper) {
    if (!bind_shader("./shaders/vs-fixed.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    float x = 0.3F;
    float y = 0.0F;
    float radius = 0.01F;
    float body_len = 0.02F;
    float nose_len = 0.01F;

    int n_points = 5;
    size_t len = 2 * n_points * sizeof(float);
    /*
     *    5     -
     *  /   \   |  nose_len
     * 4  0  1  -
     * |     |  |  body_len
     * |     |  |
     * 3_____2  -
     */
    float arr[] = {
            x + radius, y,
            x + radius, y - body_len,
            x - radius, y - body_len,
            x - radius, y,
            x, y + nose_len
    };

    buffer_data_2f(wrapper, n_points, len, arr);

    GLint in_color = glGetUniformLocation(wrapper->prog, "in_color");
    glUniform4f(in_color, 1.0, 1.0, 1.0, 1.0);

    return 1;
}

static int init_flame(struct gl_shader_wrapper *wrapper) {
    if (!bind_shader("./shaders/vs-fixed.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    float x = 0.3F;
    float y = -0.02F;
    float half_w = 0.01F;
    float h = 0.02F;

    int n_points = 3;
    size_t len = 2 * n_points * sizeof(float);
    float arr[] = {
            x - half_w, y,
            x + half_w, y,
            x, y - h
    };

    buffer_data_2f(wrapper, n_points, len, arr);

    GLint in_color = glGetUniformLocation(wrapper->prog, "in_color");
    glUniform4f(in_color, 1.0, 0.0, 0.0, 1.0);

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

    return 1;
}

static int init_graphics() {
    if (!init_circle(&circle)) {
        return 0;
    }

    if (!init_rocket(&rocket)) {
        return 0;
    }

    if (!init_flame(&flame)) {
        return 0;
    }

    return 1;
}

static void destroy_graphics() {
    destroy_shader(&circle);
    destroy_shader(&rocket);
    destroy_shader(&flame);
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
    if (type == SDL_WINDOWEVENT_SIZE_CHANGED) {
        Sint32 w = window_ev.data1;
        Sint32 h = window_ev.data2;
        printf("New window size: %dx%d\n", w, h);
        resized(w, h);
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

static void update() {
}

static void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    draw_shader_arrays(&circle, GL_TRIANGLE_FAN);
    draw_shader_arrays(&rocket, GL_POLYGON);
    draw_shader_arrays(&flame, GL_TRIANGLES);
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

    if (!init_graphics()) {
        return EXIT_FAILURE;
    }

    printf("Initialized graphics\n");

    resized(initial_w, initial_h);
    run_process_loop(win);

    destroy_graphics();

    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
