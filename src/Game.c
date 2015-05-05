#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "DrawUtils.h"

#define BG_SIZE_WIDTH 40
#define BG_SIZE_HEIGHT 40
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 640
#define MAX_PLAT_WIDTH 60
#define MAX_PLAT_HEIGHT 20
#define MAX_JUMP_HEIGHT 60
#define NUM_PLATFORMS 12

typedef struct AABB {
    int x, y, w, h;
} AABB;

typedef struct AnimFrameDef {
    // combined with the AnimDef's name to make
    // the actual texture name
    int frameNum;
    float frameTime;
} AnimFrameDef;

typedef struct AnimDef {
    const char* name;
    AnimFrameDef frames[4];
    int numFrames;
} AnimDef;

// Runtime state for an animation
typedef struct AnimData {
    AnimDef* def;
    int curFrame;
    float timeToNextFrame;
    bool isPlaying;
} AnimData;

typedef struct Player {
    float posX;
    float posY;
    int nearMissTries;
    AABB box;
} Player;

typedef struct Camera {
    AABB box;
    int posX;
    int posY;
} Camera;

typedef struct BackgroundTile {
    AABB box;
    int spriteId;
    bool collision;
} BackgroundTile;

typedef struct Platform {
    int posX;
    int posY;
    int width;
    int height;
    AABB box;
} Platform;

void animTick(AnimData*, float);
void animReset(AnimData*);
void animSet(AnimData*, AnimDef*);
void animDraw(AnimData*, int, int, int, int);
void updatePlayer(Player, int);
void updateCamera(Camera, int);
bool AABBIntersect(const AABB*, const AABB*);
void platformsTick(Platform platforms[], Camera);

GLuint textures[8];
int lastStep = 0;

int main(void) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    // Create the window, OpenGL context
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window* window = SDL_CreateWindow(
        "Press the Arrow Keys to make things happen",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    if (!window) {
        fprintf(stderr, "Could not create window. Error Code = %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GL_CreateContext(window);

    // Make sure we have a recent version of OpenGL
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Could not initialize glew. Error Code = %s\n", glewGetErrorString(glewError));
        SDL_Quit();
    }
    if (!GLEW_VERSION_3_0) {
        fprintf(stderr, "OpenGL max supported version is too low now.\n");
        SDL_Quit();
        return 1;
    }

    // Setup OpenGL state
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 100);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create the background texture array. Going to load
    // everything at the same time for now. Maybe there
    // is a more efficient way to load this later
    GLuint lambda = glTexImageTGAFile("../assets/test/lambda.tga", NULL, NULL);
    textures[0] = glTexImageTGAFile("ryu_walk_1.tga", NULL, NULL);
    textures[1] = glTexImageTGAFile("ryu_walk_2.tga", NULL, NULL);
    textures[2] = glTexImageTGAFile("ryu_walk_3.tga", NULL, NULL);
    textures[3] = glTexImageTGAFile("ryu_walk_4.tga", NULL, NULL);
    textures[4] = glTexImageTGAFile("mushroom_1.tga", NULL, NULL);
    textures[5] = glTexImageTGAFile("mushroom_2.tga", NULL, NULL);
    textures[6] = glTexImageTGAFile("mushroom_3.tga", NULL, NULL);
    textures[7] = glTexImageTGAFile("mushroom_4.tga", NULL, NULL);

    // Logic to keep track of keyboard pushes
    unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};
    const unsigned char* kbState = NULL;
    kbState = SDL_GetKeyboardState(NULL);

    // Need to keep track of when to redraw frames
    Uint32 lastFrameMs = 0;
    Uint32 currentFrameMs = SDL_GetTicks();

    // Some initialization for the background
    /* TODO Put this back in with actual textures
     * BackgroundTile background[40][40];
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 40; j++) {
            BackgroundTile tile;
            tile.spriteId = ((i % 2 == 0) || (j % 2 == 0)) ? 0 : 1;
            tile.collision = (tile.spriteId == 1) ? true : false;
            tile.box.x = j * 40;
            tile.box.y = i * 40;
            tile.box.w = 40;
            tile.box.h = 40;
            background[i][j] = tile;
        }
    }*/

    // Set options for camera coordinates to draw background
    Camera camera;
    camera.posX = 0;
    camera.posY = 0;
    camera.box.x = 0;
    camera.box.y = 0;
    camera.box.w = 640;
    camera.box.h = 480;

    // Set options for the player coordinates
    /*Player player;
    player.posX = 321;
    player.posY = 241;
    player.box.x = 321;
    player.box.y = 241;
    player.box.w = 30;
    player.box.h = 30;
    player.nearMissTries = 5;
    int playerPrevX = 321;
    int playerPrevY = 241;

    AnimData playerAnimData;
    AnimDef playerAnimDef;
    playerAnimData.curFrame = 0;
    playerAnimData.timeToNextFrame = 0.1;
    playerAnimData.isPlaying = true;
    playerAnimDef.name = "ryu";
    playerAnimDef.numFrames = 4;
    playerAnimDef.frames[0].frameNum = 0;
    playerAnimDef.frames[0].frameTime = 0.1;
    playerAnimDef.frames[1].frameNum = 1;
    playerAnimDef.frames[1].frameTime = 0.1;
    playerAnimDef.frames[2].frameNum = 2;
    playerAnimDef.frames[2].frameTime = 0.1;
    playerAnimDef.frames[3].frameNum = 3;
    playerAnimDef.frames[3].frameTime = 0.1;
    playerAnimData.def = &playerAnimDef;*/

    // Create initial set of Platforms
    Platform platforms[8];
    Platform platform;
    AABB box;
    for (int i = 0; i < NUM_PLATFORMS; i++) {
        int posX = rand() % WINDOW_WIDTH;
        int posY = lastStep + (rand() % MAX_JUMP_HEIGHT);
        int width = rand() % MAX_PLAT_WIDTH + 10;
        platform.posY = posY;
        platform.posX = posX;
        platform.width = width;
        platform.height = MAX_PLAT_HEIGHT;
        platform.box.x = posX;
        platform.box.y = posY;
        platform.box.w = width;
        platform.box.h = MAX_PLAT_HEIGHT;
        platform.box = box;
        platforms[i] = platform;
        lastStep = posY;
    }

    // The game loop
    char shouldExit = 0;
    while(!shouldExit) {
        // kbState is updated by the message pump. Copy over the old state before the pump!
        lastFrameMs = currentFrameMs;
        //playerPrevX = player.posX;
        //playerPrevY = player.posY;

        memcpy(kbPrevState, kbState, sizeof(kbPrevState));

        // Handle OS message pump
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    shouldExit = 1;
            }
        }

        // Going to handle keyboard events to move the camera or player
        kbState = SDL_GetKeyboardState(NULL);
        if (kbState[SDL_SCANCODE_RIGHT]) {
            //player.posX = (player.posX < 640) ? player.posX += 1 : player.posX;
            //player.box.x = (player.box.x < 640) ? player.box.x += 1 : player.box.x;
        }
        if (kbState[SDL_SCANCODE_LEFT]) {
            //player.posX = (player.posX > 0) ? player.posX -= 1 : player.posX;
            //player.box.x = (player.box.x > 0) ? player.box.x -= 1 : player.box.x;
        }
        if (kbState[SDL_SCANCODE_UP]) {
            //player.posY = (player.posY > 0) ? player.posY -= 1: player.posY;
            //player.box.y = (player.box.y > 0) ? player.box.y -= 1 : player.box.y;
        }
        if (kbState[SDL_SCANCODE_DOWN]) {
            //player.posY = (player.posY < 640) ? player.posY += 1 : player.posY;
            //player.box.y = (player.box.y < 640) ? player.box.y += 1 : player.box.y;
        }

        if (kbState[SDL_SCANCODE_D]) {
            camera.posX = (camera.posX < 640) ? camera.posX += 4 : camera.posX;
            camera.box.x = (camera.box.x < 640) ? camera.box.x += 4 : camera.box.x;
        }
        if (kbState[SDL_SCANCODE_A]) {
            camera.posX = (camera.posX > 0) ? camera.posX -= 4 : camera.posX;
            camera.box.x = (camera.box.x > 0) ? camera.box.x -= 4 : camera.box.x;
        }
        if (kbState[SDL_SCANCODE_W]) {
            camera.posY = (camera.posY > 0) ? camera.posY -= 4: camera.posY;
            camera.box.y = (camera.box.y > 0) ? camera.box.y -= 4: camera.box.y;
        }
        if (kbState[SDL_SCANCODE_S]) {
            camera.posY = (camera.posY < 640) ? camera.posY += 4 : camera.posY;
            camera.box.y = (camera.box.y < 640) ? camera.box.y += 4 : camera.box.y;
        }

        // Update platforms to move down
        platformsTick(platforms, camera);

        // Calculating frame updates
        currentFrameMs = SDL_GetTicks();
        //float deltaTime = (currentFrameMs - lastFrameMs) / 1000.0f;

        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update player
        /*if (playerAnimData.curFrame == 3) {
            animReset(&playerAnimData);
        } else {
            animTick(&playerAnimData, deltaTime);
        }*/

        // Check for wall collisions and update
        /* TODO Put this back in when we have background tiles
         * for (int i = 0; i < 40; i++) {
            for (int j = 0; j < 40; j++) {
                if (AABBIntersect(&camera.box, &background[i][j].box)) {
                    // If a player collides with wall reset position
                    if (AABBIntersect(&player.box, &background[i][j].box) && background[i][j].collision) {
                        player.posX = playerPrevX;
                        player.box.x = playerPrevX;
                        player.posY = playerPrevY;
                        player.box.y = playerPrevY;
                    }
                }
            }
        }*/

        //playerPrevX = player.posX;
        //playerPrevY = player.posY;

        // This draws the background.
        // TODO Put this back in when we have actual background artwork
        /*
        for (int i = 0; i < 40; i++) {
            for (int j = 0; j < 40; j++) {
                if (AABBIntersect(&camera.box, &background[i][j].box)) {
                    if (background[i][j].spriteId == 0) {
                        glDrawSprite(aperture,
                                    (j * 40) - camera.posX,
                                    (i * 40) - camera.posY,
                                    40,
                                    40);
                    } else {
                        glDrawSprite(lambda,
                                    (j * 40) - camera.posX,
                                    (i * 40) - camera.posY,
                                    40,
                                    40);
                    }
                }
            }
        }*/

        // Draw the platforms
        for (int i = 0; i < NUM_PLATFORMS; i++) {
            // Draw simple sprite here. Can make this more advanced later
            glDrawSprite(lambda,
                         platforms[i].posX ,
                         platforms[i].posY ,
                         platforms[i].width,
                         platforms[i].height);
        }

        // This draws the player
        //animDraw(&playerAnimData, player.posX - camera.posX, player.posY - camera.posY, 40, 40);
        SDL_GL_SwapWindow(window);
    }

    SDL_Quit();
    return 0;
}

void animTick(AnimData* data, float dt) {
    if(!data->isPlaying) {
        return;
    }
    int numFrames = sizeof(data->def->frames) / sizeof(AnimFrameDef);
    data->timeToNextFrame -= dt;
    if(data->timeToNextFrame < 0) {
        ++data->curFrame;
        if(data->curFrame >= numFrames) {
            // end of the animation, stop it
            data->curFrame = numFrames - 1;
            data->timeToNextFrame = 0;
            data->isPlaying = false;
        } else {
            AnimFrameDef curFrame = data->def->frames[data->curFrame];
            data->timeToNextFrame += curFrame.frameTime;
        }
    }
}

void animReset(AnimData* anim) {
    animSet(anim, anim->def);
}

void animSet(AnimData* anim, AnimDef* toPlay) {
    int setFrame = anim->curFrame - (anim->def->numFrames - 1);
    anim->def = toPlay;
    anim->curFrame = setFrame;
    anim->timeToNextFrame = anim->def->frames[setFrame].frameTime;
    anim->isPlaying = true;
}

void animDraw(AnimData* anim, int x, int y, int w, int h) {
    int curFrameNum = anim->def->frames[anim->curFrame].frameNum;
    GLuint tex = textures[curFrameNum];
    glDrawSprite(tex, x, y, w, h);
}

bool AABBIntersect(const AABB* box1, const AABB* box2) {
    // box1 to the right
    if( box1->x > box2->x + box2->w ) {
        return false;
    }

    // box1 to the left
    if( box1->x + box1->w < box2->x ) {
        return false;
    }

    // box1 below
    if( box1->y > box2->y + box2->h ) {
        return false;
    }
    // box1 above
    if( box1->y + box1->h < box2->y ) {
        return false;
    }
    return true;
}

void platformsTick(Platform platforms[], Camera camera) {
    for (int i = 0; i < NUM_PLATFORMS; i++) {
        // If the platform is in the camera view draw it
        if (AABBIntersect(&platforms[i].box, &camera.box)) {
            platforms[i].posY = platforms[i].posY + 1;
            platforms[i].box.y = platforms[i].box.y + 1;
            platforms[i] = platforms[i];
        }
        else {
            Platform newPlat;
            AABB box;
            int posX = rand() % WINDOW_WIDTH;
            int posY = lastStep + (rand() % MAX_JUMP_HEIGHT);
            int width = rand() % MAX_PLAT_WIDTH + 10;
            newPlat.posY = posY;
            newPlat.posX = posX;
            newPlat.width = width;
            newPlat.height = MAX_PLAT_HEIGHT;
            newPlat.box.x = posX;
            newPlat.box.y = posY;
            newPlat.box.w = width;
            newPlat.box.h = MAX_PLAT_HEIGHT;
            newPlat.box = box;
            platforms[i] = newPlat;
            lastStep = posY;
        }
    }
}
