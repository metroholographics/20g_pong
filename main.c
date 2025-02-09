#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 80
#define NUM_PADDLES 1

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct paddle {
    SDL_FRect shape;
    int x;
    int y;
} Paddle;

Paddle paddles[NUM_PADDLES];


void update_paddles(Paddle* p) {
    for (int i = 0; i < NUM_PADDLES; i++) {
        p[i].shape.y += p[i].y * 10;
    }
}

void draw_paddles(Paddle* p) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < NUM_PADDLES; i++) {
        SDL_RenderFillRect(renderer, &p[i].shape);
    }
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Couldn't init SDL: %s\n", SDL_GetError());       
    }

    SDL_CreateWindowAndRenderer("20g_pong", WIDTH, HEIGHT, 0, &window, & renderer);

    if (window == NULL || renderer == NULL) {
        printf("Error: %s\n", SDL_GetError());
    }

    Paddle player = (Paddle) {
        .shape = (SDL_FRect) {
                    .x = 10,
                    .y = 100,
                    .w = PADDLE_WIDTH,
                    .h = PADDLE_HEIGHT
                },
        .x = 0,
        .y = 0
    };

    paddles[0] = player;

    SDL_Event e;
    bool running = true;
   
     while (running) {
         SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
         SDL_RenderClear(renderer);
         
         while (SDL_PollEvent(&e)) {
             if (e.type == SDL_EVENT_QUIT) {
                 running = false;
                 break;
             }
             if (e.type == SDL_EVENT_KEY_DOWN)  {
                switch (e.key.key) {
                    case SDLK_S:
                        paddles[0].y = 1;
                        break;
                    case SDLK_W:
                        paddles[0].y = -1;
                        break;
                 }
             }
             if (e.type == SDL_EVENT_KEY_UP)  {
                switch (e.key.key) {
                    case SDLK_S:
                        paddles[0].y = 0;
                        break;
                    case SDLK_W:
                        paddles[0].y = 0;
                        break;
                 }
             } 
         }
         
        update_paddles(paddles);
        draw_paddles(paddles);

        SDL_RenderPresent(renderer);   
        
        SDL_Delay(17);
           
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
    
}

