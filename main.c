#include <SDL.h>

static void handle_key_press(SDL_Event *ev) {
    SDL_Keysym key_info = ev->key.keysym;
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
    }
}

int main() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return 1;
    }

    printf("Initialized SDL\n");

    SDL_Window *win = SDL_CreateWindow("Orbital", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, 0);
    while (1) {
        SDL_Event ev;
        if (!SDL_PollEvent(&ev)) {
            continue;
        }

        Uint32 ev_type = ev.type;
        if (ev_type == SDL_QUIT) {
            break;
        } else if (ev_type == SDL_KEYDOWN) {
            handle_key_press(&ev);
        }
    }

    SDL_DestroyWindow(win);

    return 0;
}
