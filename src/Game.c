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
#define PLAT_HEIGHT 20
#define MAX_JUMP_HEIGHT 40
#define NUM_PLATFORMS 12
#define WALKING_ANIMS_PATH "../assets/walking-animation/tga/"

/*Game States*/
enum game_states{
	menu_screen,
	gameplay
};

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
	AnimFrameDef frames[8];
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
	float yVelocity;
	float jumpTimeRemaining;
	int nearMissTries;
    bool isJumping;
    bool jumpAgain;
	AABB box;
} Player;

typedef struct Camera {
	AABB innerBox;
	AABB outerBox;
	int posX;
	int posY;
} Camera;

typedef struct Lava {
	AABB box;
	int posX;
	int posY;
} Lava;

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
void platformsTick(Platform platforms[]);
void cyclePlatforms(Platform platforms1[], Camera);
bool jumpingAABBIntersect(const AABB*, const AABB*);

GLuint textures[18];
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

	/*game_state*/
	enum game_states currentState = menu_screen;

	// Create the background texture array. Going to load
	// everything at the same time for now. Maybe there
	// is a more efficient way to load this later
	GLuint thelava = glTexImageTGAFile("../assets/test/lava.tga", NULL, NULL);
	GLuint lambda = glTexImageTGAFile("../assets/test/lambda.tga", NULL, NULL);
	GLuint title_screen = glTexImageTGAFile("../assets/test/title_screen.tga", NULL, NULL);

	/*PLayer Standing Left Animation*/
	textures[0] = glTexImageTGAFile("standFaceLeft.tga", NULL, NULL);

	/*Player Standing Right Animation*/
	textures[1] = glTexImageTGAFile("standFaceRight.tga", NULL, NULL);

	/*Player Walking Left Animation*/
	textures[2] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft1.tga", NULL, NULL);
	textures[3] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft2.tga", NULL, NULL);
	textures[4] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft3.tga", NULL, NULL);
	textures[5] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft4.tga", NULL, NULL);
	textures[6] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft5.tga", NULL, NULL);
	textures[7] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft6.tga", NULL, NULL);
	textures[8] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft7.tga", NULL, NULL);
	textures[9] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft8.tga", NULL, NULL);

	/*Player Walking Right Animation*/
	textures[10] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight1.tga", NULL, NULL);
	textures[11] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight2.tga", NULL, NULL);
	textures[12] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight3.tga", NULL, NULL);
	textures[13] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight4.tga", NULL, NULL);
	textures[14] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight5.tga", NULL, NULL);
	textures[15] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight6.tga", NULL, NULL);
	textures[16] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight7.tga", NULL, NULL);
	textures[17] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight8.tga", NULL, NULL);


	// Logic to keep track of keyboard pushes
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };
	const unsigned char* kbState = NULL;
	kbState = SDL_GetKeyboardState(NULL);

	// Need to keep track of when to redraw frames
	Uint32 lastFrameMs = 0;
	Uint32 currentFrameMs = SDL_GetTicks();

	/*Lava initialization*/
	Lava lava1;
	lava1.posX = 0;
	lava1.posY = WINDOW_HEIGHT;
	lava1.box.x = 0;
	lava1.box.y = WINDOW_HEIGHT;
	lava1.box.w = WINDOW_WIDTH;
	lava1.box.h = 1;

	// Set options for camera coordinates to draw background
	Camera camera;
	camera.posX = 0;
	camera.posY = 0;
	camera.outerBox.x = 0;
	camera.outerBox.y = 0;
	camera.outerBox.w = WINDOW_WIDTH;
	camera.outerBox.h = WINDOW_HEIGHT;
	camera.innerBox.x = 0;
	camera.innerBox.y = 0;
	camera.innerBox.w = 240;
	camera.innerBox.h = 380;

	// Set options for the player coordinates
	Player player;
	player.posX = 150;
	player.posY = 560;
	player.box.x = 150;
	player.box.y = 560;
	player.box.w = 30;
	player.box.h = 30;
	player.yVelocity = 10;
	player.jumpTimeRemaining = 3;
	player.isJumping = false;
	player.jumpAgain = true;
	player.nearMissTries = 5;
	int playerPrevX = 0;
	int playerPrevY = 0;

	/*player anim data*/
	AnimData playerAnimData;
	playerAnimData.curFrame = 0;
	playerAnimData.timeToNextFrame = 0.1;
	playerAnimData.isPlaying = true;

	/*Player standing Left Animation Def*/
	AnimDef playerStandLeftDef;

	playerStandLeftDef.name = "PlayerStandLeft";
	playerStandLeftDef.numFrames = 1;
	playerStandLeftDef.frames[0].frameNum = 0;
	playerStandLeftDef.frames[0].frameTime = 0.1;

	/*Player standing right Animation Def*/
	AnimDef playerStandRightDef;
	playerStandRightDef.name = "PlayerStandRight";
	playerStandRightDef.numFrames = 1;
	playerStandRightDef.frames[0].frameNum = 1;
	playerStandRightDef.frames[0].frameTime = 0.1;

	/*Player Walking Left Animation Def*/
	AnimDef playerWalkingLeftDef;
	playerWalkingLeftDef.name = "PlayerWalkingLeft";
	playerWalkingLeftDef.numFrames = 8;
	playerWalkingLeftDef.frames[0].frameNum = 2;
	playerWalkingLeftDef.frames[0].frameTime = 0.1;
	playerWalkingLeftDef.frames[1].frameNum = 3;
	playerWalkingLeftDef.frames[1].frameTime = 0.1;
	playerWalkingLeftDef.frames[2].frameNum = 4;
	playerWalkingLeftDef.frames[2].frameTime = 0.1;
	playerWalkingLeftDef.frames[3].frameNum = 5;
	playerWalkingLeftDef.frames[3].frameTime = 0.1;
	playerWalkingLeftDef.frames[4].frameNum = 6;
	playerWalkingLeftDef.frames[4].frameTime = 0.1;
	playerWalkingLeftDef.frames[5].frameNum = 7;
	playerWalkingLeftDef.frames[5].frameTime = 0.1;
	playerWalkingLeftDef.frames[6].frameNum = 8;
	playerWalkingLeftDef.frames[6].frameTime = 0.1;
	playerWalkingLeftDef.frames[7].frameNum = 9;
	playerWalkingLeftDef.frames[7].frameTime = 0.1;
	playerAnimData.def = &playerWalkingLeftDef;

	AnimDef playerWalkingRightDef;
	playerWalkingRightDef.name = "PlayerWalkingRight";
	playerWalkingRightDef.numFrames = 8;
	playerWalkingRightDef.frames[0].frameNum = 10;
	playerWalkingRightDef.frames[0].frameTime = 0.1;
	playerWalkingRightDef.frames[1].frameNum = 11;
	playerWalkingRightDef.frames[1].frameTime = 0.1;
	playerWalkingRightDef.frames[2].frameNum = 12;
	playerWalkingRightDef.frames[2].frameTime = 0.1;
	playerWalkingRightDef.frames[3].frameNum = 13;
	playerWalkingRightDef.frames[3].frameTime = 0.1;
	playerWalkingRightDef.frames[4].frameNum = 14;
	playerWalkingRightDef.frames[4].frameTime = 0.1;
	playerWalkingRightDef.frames[5].frameNum = 15;
	playerWalkingRightDef.frames[5].frameTime = 0.1;
	playerWalkingRightDef.frames[6].frameNum = 16;
	playerWalkingRightDef.frames[6].frameTime = 0.1;
	playerWalkingRightDef.frames[7].frameNum = 17;
	playerWalkingRightDef.frames[7].frameTime = 0.1;

	// Create initial set of Platforms
	Platform platforms[NUM_PLATFORMS];
	Platform platform;
	AABB box;
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		int posX = rand() % WINDOW_WIDTH;
		// Add 10 from jump height to make things easier to reach
		int posY = lastStep + (MAX_JUMP_HEIGHT + 5);
		int width = rand() % MAX_PLAT_WIDTH + 10;
		platform.posY = posY;
		platform.posX = posX;
		platform.width = width;
		platform.height = PLAT_HEIGHT;
		box.x = posX;
		box.y = posY;
		box.w = width;
		box.h = PLAT_HEIGHT;
		platform.box = box;
		platforms[i] = platform;
		lastStep = posY;
	}

	// For testing purposes
	platform.posY = 600;
	platform.posX = 0;
	platform.width = WINDOW_WIDTH - 10;
	platform.height = PLAT_HEIGHT;
	box.y = 600;
	box.x = 0;
	box.w = WINDOW_WIDTH;
	box.h = PLAT_HEIGHT;
	platform.box = box;
	platforms[0] = platform;

	// The game loop
	char shouldExit = 0;
	char startPlats = false;
	while (!shouldExit)
	{
		playerAnimData.isPlaying = false;

		// Calculating frame updates
		currentFrameMs = SDL_GetTicks();
		float deltaTime = (currentFrameMs - lastFrameMs) / 1000.0f;


		// kbState is updated by the message pump. Copy over the old state before the pump!
		lastFrameMs = currentFrameMs;
		playerPrevX = player.posX;
		playerPrevY = player.posY;

		memcpy(kbPrevState, kbState, sizeof(kbPrevState));

		// Handle OS message pump
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				shouldExit = 1;
			}
		}

		// Going to handle keyboard events to move the camera or player
		kbState = SDL_GetKeyboardState(NULL);
        
        switch (currentState) {
            case menu_screen:
            if (kbState[SDL_SCANCODE_RETURN]) {
                currentState = gameplay;
            }

            glDrawSprite(title_screen, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
            break;

        case gameplay:
		    if (kbState[SDL_SCANCODE_RIGHT]) {
			    playerAnimData.def = &playerWalkingRightDef;
			    playerAnimData.isPlaying = true;
			    if (player.posX < WINDOW_HEIGHT) {
				    player.posX += 4;
				    player.box.x += 4;
                }
            }

	        if (kbState[SDL_SCANCODE_LEFT]) {
				playerAnimData.def = &playerWalkingLeftDef;
				playerAnimData.isPlaying = true;
				if (player.posX > 0){
					player.posX -= 4;
					player.box.x -= 4;
				}
			}

		    if (kbState[SDL_SCANCODE_UP] && player.jumpAgain) {
                startPlats = true;
                player.isJumping = true;
                player.jumpAgain = false;
		    }

            // Update player position based on gravity. This should happen when no keys are pressed too
            if (player.isJumping) {
                player.yVelocity = 30;
                player.jumpTimeRemaining -= 1;

                // Accounting for gravity with player
                player.posY = player.posY - player.yVelocity;
                player.box.y = player.box.y - player.yVelocity;
            }
            else {
                // Accounting for gravity with player
                player.posY = player.posY + player.yVelocity;
                player.box.y = player.box.y + player.yVelocity;
            }

			// Update platforms to move down
			if (startPlats) {
				platformsTick(platforms);
			}

		    glClearColor(0, 0, 0, 0);
		    glClear(GL_COLOR_BUFFER_BIT);
			
            // Update player animation
			if (playerAnimData.curFrame == 7) {
				animReset(&playerAnimData);
			}
			else {
				animTick(&playerAnimData, deltaTime);
			}

            // Check for collisions with platforms and player. Might need to move this first
            if (player.isJumping) {
                for (int i = 0; i < NUM_PLATFORMS; i++) {
                    if (jumpingAABBIntersect(&player.box, &platforms[i].box)) {
                        player.yVelocity = 10;
                        player.isJumping = false;
                        player.jumpAgain = true;
                    }
                }
            }
            else {
                // Need to make alternate check for when no jumping is occuring
                bool isFalling = true;
                for (int i = 0; i < NUM_PLATFORMS; i++) {
                    if (AABBIntersect(&player.box, &platforms[i].box)) {
                        player.yVelocity = 1;
                        player.posY= playerPrevY;
                        player.box.y = playerPrevY;
                        isFalling = false;
                    }
                }
                if (isFalling) {
                    player.posY = player.posY + 5;
                    player.box.y = player.posY + 5;
                }
            }

			// Need to cycle out old platforms and create new ones
	        cyclePlatforms(platforms, camera);

            // Draw the platforms
		    for (int i = 0; i < NUM_PLATFORMS; i++) {
			    // Draw simple sprite here. Can make this more advanced later
			    if (AABBIntersect(&platforms[i].box, &camera.outerBox)) {
				    glDrawSprite(lambda,
					    platforms[i].posX,
					    platforms[i].posY,
					    platforms[i].width,
					    platforms[i].height);
                }
            }

			/*This draws the lava*/
			if (startPlats) {
			    lava1.box.h++;
			    lava1.posY--;
				glDrawSprite(thelava, lava1.posX, lava1.posY, WINDOW_WIDTH, lava1.box.h);
			}

            // Draws the player
			animDraw(&playerAnimData, player.posX - camera.posX, player.posY - camera.posY, 40, 40);
			break;
        }
		SDL_GL_SwapWindow(window);
    }
    SDL_Quit();
    return 0;
}

void animTick(AnimData* data, float dt) {
	if (!data->isPlaying) {
		return;
	}
	int numFrames = sizeof(data->def->frames) / sizeof(AnimFrameDef);
	data->timeToNextFrame -= dt;
	if (data->timeToNextFrame < 0) {
		++data->curFrame;
		if (data->curFrame >= numFrames) {
			// end of the animation, stop it
			data->curFrame = numFrames - 1;
			data->timeToNextFrame = 0;
			data->isPlaying = false;
		}
		else {
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
	if (box1->x > box2->x + box2->w) {
		return false;
	}

	// box1 to the left
	if (box1->x + box1->w < box2->x) {
		return false;
	}

	// box1 below
	if (box1->y > box2->y + box2->h) {
		return false;
	}
	// box1 above
	if (box1->y + box1->h < box2->y) {
		return false;
	}
	return true;
}

// Modified collision detection for one-way platforms only
bool jumpingAABBIntersect(const AABB* box1, const AABB* box2) {
	// box1 below
	if (box1->y > box2->y + box2->h) {
		return false;
	}
	// box1 above. Don't need to care since 1 way
	if (box1->y + box1->h < box2->y) {
		return true;
	}
	return true;
}

void platformsTick(Platform platforms[]) {
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		platforms[i].posY = platforms[i].posY + 1;
		platforms[i].box.y = platforms[i].box.y + 1;
		platforms[i] = platforms[i];
	}
}

void cyclePlatforms(Platform platforms[], Camera camera) {
	Platform platform;
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		// Platform has dropped out of frame. Need to add new one
		if (!AABBIntersect(&platforms[i].box, &camera.outerBox)) {
			int posX = rand() % WINDOW_WIDTH;
			int posY = 0;
			int width = rand() % MAX_PLAT_WIDTH + 10;
			platform.posY = posY;
			platform.posX = posX;
			platform.width = width;
			platform.height = PLAT_HEIGHT;
			platform.box.x = posX;
			platform.box.y = posY;
			platform.box.w = width;
			platform.box.h = PLAT_HEIGHT;
			platforms[i] = platform;
		}
	}
}
