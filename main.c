#include <SDL.h>
#include <GL/glew.h>
#include <time.h>
#include <cglm/cglm.h>
#include "shader_util.h"
#include "system.h"

#define POS_BUF_SIZE 1024

static const double TWO_PI = 2.0 * M_PI;
static const int CIRCLE_DIVISIONS = 100;
static const long MS_PER_SEC = 1000;
static const long NS_PER_MS = 1000000;
static const long LOOP_DURATION_MS = 20;

// IAU 2015 Earth radius
static const double EARTH_RAD = 6378100.0;
static const double SCALE = 0.15 / EARTH_RAD;
// F9 Payload Guide
static const double F9_2_THRUST = 981000;

enum direction {
    IDLE,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

static struct body *bodies;

static struct gl_shader_wrapper circle_shader;
static struct gl_shader_wrapper rocket_shader;
static struct gl_shader_wrapper flame_shader;
static struct gl_shader_wrapper path_shader;

static enum direction cur_dir = IDLE;

static int pos_buf_idx = 0;
static struct vector pos_buf[POS_BUF_SIZE];

static int init_circle(struct gl_shader_wrapper *wrapper) {
    if (!bind_shader("./shaders/vs-fixed.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    // Stole this code from: https://stackoverflow.com/questions/22444450/drawing-circle-with-opengl/24843626#24843626
    float x = 0.0F;
    float y = 0.0F;
    float r = EARTH_RAD * SCALE;

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
    if (!bind_shader("./shaders/vs-transform.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    float x = 0.0F;
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
    if (!bind_shader("./shaders/vs-transform.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    float x = 0.0F;
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

static int init_path(struct gl_shader_wrapper *wrapper) {
    if (!bind_shader("./shaders/vs-fixed.glsl", "./shaders/fs-fixed.glsl", wrapper)) {
        return 0;
    }

    GLint in_color = glGetUniformLocation(wrapper->prog, "in_color");
    glUniform4f(in_color, 1.0, 1.0, 1.0, 1.0);

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
    SDL_ShowCursor(SDL_DISABLE);

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    // glViewport(0, 0, mode.w, mode.h);

    if (!init_circle(&circle_shader)) {
        return 0;
    }

    if (!init_rocket(&rocket_shader)) {
        return 0;
    }

    if (!init_flame(&flame_shader)) {
        return 0;
    }
    
    if (!init_path(&path_shader)) {
        return 0;
    }

    return 1;
}

static void destroy_graphics() {
    destroy_shader(&circle_shader);
    destroy_shader(&rocket_shader);
    destroy_shader(&flame_shader);
    destroy_shader(&path_shader);
}

static void reset_system() {
    struct body *earth = bodies;
    struct vector zero = {0.0F, 0.0F, 0.0F};
    earth->F_net_ext = zero;
    earth->pos = zero;
    earth->vel = zero;
    earth->acl = zero;

    struct body *rocket = bodies + 1;
    rocket->F_net_ext = zero;
    struct vector rocket_pos = {EARTH_RAD + 1000000.0, 0.0, 0.0};
    rocket->pos = rocket_pos;
    struct vector rocket_v = {0.0, 7200.0, 0.0};
    rocket->vel = rocket_v;
    rocket->acl = zero;

    pos_buf_idx = 0;
    path_shader.n_points = 0;
}

static void init_system() {
    bodies = malloc(2 * sizeof(struct body));

    // IAU 1976 value for Earth mass
    add_body(5972200000000000000000000.0, bodies);

    // https://www.spaceflightinsider.com/hangar/falcon-9/
    // F9 2nd stage mass
    add_body(96570.0, bodies + 1);

    reset_system();
}

static void destroy_system() {
    free(bodies);
}

static void set_transform(const struct gl_shader_wrapper *wrapper, const mat4 *transform) {
    GLint transform_loc = glGetUniformLocation(wrapper->prog, "transform");

    glUseProgram(wrapper->prog);
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, (float *) *transform);
}

static void handle_key_event(const SDL_Event *ev, int press_down) {
    SDL_KeyboardEvent key_ev = ev->key;
    SDL_Keysym key_info = key_ev.keysym;
    SDL_Scancode sc = key_info.scancode;

    if (press_down) {
        switch (sc) {
            case SDL_SCANCODE_LEFT:
                cur_dir = LEFT;
                break;
            case SDL_SCANCODE_UP:
                cur_dir = UP;
                break;
            case SDL_SCANCODE_RIGHT:
                cur_dir = RIGHT;
                break;
            case SDL_SCANCODE_DOWN:
                cur_dir = DOWN;
                break;
            case SDL_SCANCODE_R:
                printf("Reset the system\n");
                reset_system();
                break;
            default:
                break;
        }
    } else {
        switch (sc) {
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_RIGHT:
            case SDL_SCANCODE_DOWN: {
                cur_dir = IDLE;

                struct vector zero = {0.0F, 0.0F, 0.0F};
                bodies[1].F_net_ext = zero;
                break;
            }
            default:
                break;
        }
    }
}

static void resized(int w, int h) {
    glViewport(0, 0, 1000, 1000);
}

static void handle_window_event(SDL_Event *ev) {
    SDL_WindowEvent window_ev = ev->window;
    Uint8 type = window_ev.event;
    if (type == SDL_WINDOWEVENT_RESIZED) {
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
                handle_key_event(&ev, 1);
                break;
            case SDL_KEYUP:
                handle_key_event(&ev, 0);
                break;
            case SDL_WINDOWEVENT:
                handle_window_event(&ev);
                break;
            default:
                break;
        }
    }
}

static double mag(struct vector v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static struct vector cross(struct vector a, struct vector b) {
    struct vector result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    return result;
}

static void update() {
    struct body *rocket = bodies + 1;

    switch (cur_dir) {
        case LEFT: {
            struct vector vel = rocket->vel;
            struct vector axis = {0.0, 0.0, 1.0};
            struct vector cx = cross(vel, axis);
            double cx_mag = mag(cx);
            
            struct vector thrust = {-F9_2_THRUST * cx.x / cx_mag, -F9_2_THRUST * cx.y / cx_mag, -F9_2_THRUST * cx.z / cx_mag};
            rocket->F_net_ext = thrust;

            break;
        }
        case UP: {
            struct vector vel = rocket->vel;
            double v = mag(vel);

            struct vector thrust = {F9_2_THRUST * vel.x / v, F9_2_THRUST * vel.y / v, F9_2_THRUST * vel.z / v};
            rocket->F_net_ext = thrust;

            break;
        }
        case RIGHT: {
            struct vector vel = rocket->vel;
            struct vector axis = {0.0, 0.0, 1.0};
            struct vector cx = cross(vel, axis);
            double cx_mag = mag(cx);

            struct vector thrust = {F9_2_THRUST * cx.x / cx_mag, F9_2_THRUST * cx.y / cx_mag, F9_2_THRUST * cx.z / cx_mag};
            rocket->F_net_ext = thrust;

            break;
        }
        case DOWN: {
            struct vector vel = rocket->vel;
            double v = mag(vel);

            struct vector thrust = {-F9_2_THRUST * vel.x / v, -F9_2_THRUST * vel.y / v, -F9_2_THRUST * vel.z / v};
            rocket->F_net_ext = thrust;

            break;
        }
        default:
            break;
    }

    recompute_system(5, 2, bodies);
    
    int idx = pos_buf_idx;
    pos_buf[idx] = rocket->pos;

    if (pos_buf_idx < POS_BUF_SIZE - 1) {
        pos_buf_idx++;
    } else {
        memmove(pos_buf, pos_buf + 1, (POS_BUF_SIZE - 1) * sizeof(*pos_buf));
    }

    int n_pos = idx + 1;
    int len = 2 * n_pos * sizeof(float);
    float *points = malloc(len);
    for (int pos_idx = 0, points_idx = 0; pos_idx < n_pos; ++pos_idx, points_idx += 2) {
        struct vector pos = pos_buf[pos_idx];
        points[points_idx] = SCALE * pos.x;
        points[points_idx + 1] = SCALE * pos.y;
    }

    buffer_data_2f(&path_shader, n_pos, len, points);
    free(points);
}

static void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    draw_shader_arrays(&circle_shader, GL_TRIANGLE_FAN);
    draw_shader_arrays(&path_shader, GL_LINE_STRIP);

    mat4 rocket_transform;
    glm_mat4_identity(rocket_transform);

    struct body rocket = bodies[1];
    vec3 rocket_translation = {SCALE * rocket.pos.x, SCALE * rocket.pos.y, SCALE * rocket.pos.z};
    glm_translate(rocket_transform, rocket_translation);

    vec3 zero_v_rot = {0.0F, 1.0F, 0.0F};
    vec3 vel = {rocket.vel.x, rocket.vel.y, rocket.vel.z};
    float angle = glm_vec3_angle(zero_v_rot, vel);
    if (zero_v_rot[0] < vel[0]) {
        angle = -angle;
    }

    vec3 rot_axis = {0.0F, 0.0F, 1.0F};
    glm_rotate(rocket_transform, angle, rot_axis);

    set_transform(&rocket_shader, &rocket_transform);
    draw_shader_arrays(&rocket_shader, GL_POLYGON);

    if (cur_dir != IDLE) {
        mat4 flame_transform;
        glm_mat4_copy(rocket_transform, flame_transform);

        switch (cur_dir) {
            case LEFT: {
                vec3 flame_translation = {-0.01F, -0.01F, 0.0F};
                glm_translate(flame_transform, flame_translation);
                glm_rotate(flame_transform, M_PI / 2.0F, rot_axis);
                break;
            }
            case RIGHT: {
                vec3 flame_translation = {0.01F, -0.01F, 0.0F};
                glm_translate(flame_transform, flame_translation);
                glm_rotate(flame_transform, M_PI / -2.0F, rot_axis);
                break;
            }
            case DOWN: {
                vec3 flame_translation = {0.00F, -0.01F, 0.0F};
                glm_translate(flame_transform, flame_translation);
                glm_rotate(flame_transform, M_PI, rot_axis);
                break;
            }
            default:
                break;
        }

        set_transform(&flame_shader, &flame_transform);
        draw_shader_arrays(&flame_shader, GL_TRIANGLES);
    }
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
        long delay = LOOP_DURATION_MS - elapsed;
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

    init_system();
    printf("Initialized system\n");

    resized(initial_w, initial_h);
    run_process_loop(win);

    destroy_system();
    destroy_graphics();

    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
