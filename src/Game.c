#include <GL\glew.h>
#include <SDL\SDL.h>
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
	int nearMissTries;
	AABB box;
} Player;

typedef struct Camera {
	AABB innerBox;
	AABB outerBox;
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
void platformsTick(Platform platforms[]);
void cyclePlatforms(Platform platforms1[], Camera);

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

	// Create the background texture array. Going to load
	// everything at the same time for now. Maybe there
	// is a more efficient way to load this later
	GLuint lambda = glTexImageTGAFile("lambda.tga", NULL, NULL);

	/*PLayer Standing Left Animation*/
	textures[0] = glTexImageTGAFile("standFaceLeft.tga", NULL, NULL);

	/*Player Standing Right Animation*/
	textures[1] = glTexImageTGAFile("standFaceRight.tga", NULL, NULL);

	/*Player Walking Left Animation*/
	textures[2] = glTexImageTGAFile("walkLeft1.tga", NULL, NULL);
	textures[3] = glTexImageTGAFile("walkLeft2.tga", NULL, NULL);
	textures[4] = glTexImageTGAFile("walkLeft3.tga", NULL, NULL);
	textures[5] = glTexImageTGAFile("walkLeft4.tga", NULL, NULL);
	textures[6] = glTexImageTGAFile("walkLeft5.tga", NULL, NULL);
	textures[7] = glTexImageTGAFile("walkLeft6.tga", NULL, NULL);
	textures[8] = glTexImageTGAFile("walkLeft7.tga", NULL, NULL);
	textures[9] = glTexImageTGAFile("walkLeft8.tga", NULL, NULL);

	/*Player Walking Right Animation*/
	textures[10] = glTexImageTGAFile("walkRight1.tga", NULL, NULL);
	textures[11] = glTexImageTGAFile("walkRight2.tga", NULL, NULL);
	textures[12] = glTexImageTGAFile("walkRight3.tga", NULL, NULL);
	textures[13] = glTexImageTGAFile("walkRight4.tga", NULL, NULL);
	textures[14] = glTexImageTGAFile("walkRight5.tga", NULL, NULL);
	textures[15] = glTexImageTGAFile("walkRight6.tga", NULL, NULL);
	textures[16] = glTexImageTGAFile("walkRight7.tga", NULL, NULL);
	textures[17] = glTexImageTGAFile("walkRight8.tga", NULL, NULL);


	// Logic to keep track of keyboard pushes
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };
	const unsigned char* kbState = NULL;
	kbState = SDL_GetKeyboardState(NULL);

	// Need to keep track of when to redraw frames
	Uint32 lastFrameMs = 0;
	Uint32 currentFrameMs = SDL_GetTicks();

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
	player.posX = 0;
	player.posY = 0;
	player.box.x = 0;
	player.box.y = 0;
	player.box.w = 30;
	player.box.h = 30;
	player.nearMissTries = 5;
	int playerPrevX = 321;
	int playerPrevY = 241;


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
	//playerAnimData.def = &playerStandLeftDef;

	/*Player standing right Animation Def*/
	AnimDef playerStandRightDef;
	playerStandRightDef.name = "PlayerStandRight";
	playerStandRightDef.numFrames = 1;
	playerStandRightDef.frames[0].frameNum = 1;
	playerStandRightDef.frames[0].frameTime = 0.1;
	//playerAnimData.def = &playerStandRightDef;

	/*Player Walking Left Animation Def*/
	AnimDef playerWalkingLeftDef;
	playerWalkingLeftDef.name - "PlayerWalkingLeft";
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
	//playerAnimData.def = &playerWalkingLeftDef;

	/*Player Walking Right Animation Def*/
	AnimDef playerWalkingRightDef;
	playerWalkingRightDef.name - "PlayerWalkingRight";
	playerWalkingRightDef.numFrames = 8;
	playerWalkingRightDef.frames[0].frameNum =10;
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
	playerAnimData.def = &playerWalkingRightDef;


//////////////////////////////PLATFORMS////////////////////////////////////////////////////

	//// Create initial set of Platforms
	//Platform platforms[NUM_PLATFORMS];
	//Platform platform;
	//AABB box;
	//for (int i = 0; i < NUM_PLATFORMS; i++) {
	//	int posX = rand() % WINDOW_WIDTH;
	//	int posY = lastStep + MAX_JUMP_HEIGHT;
	//	int width = rand() % MAX_PLAT_WIDTH + 10;
	//	platform.posY = posY;
	//	platform.posX = posX;
	//	platform.width = width;
	//	platform.height = PLAT_HEIGHT;
	//	box.x = posX;
	//	box.y = posY;
	//	box.w = width;
	//	box.h = PLAT_HEIGHT;
	//	platform.box = box;
	//	platforms[i] = platform;
	//	lastStep = posY;
	//}
//////////////////////////////PLATFORMS////////////////////////////////////////////////////


	// The game loop
	char shouldExit = 0;
	while (!shouldExit) 
	{
		printf("%d\n", playerAnimData.curFrame);
	

	

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

		if (kbState[SDL_SCANCODE_RIGHT]) {
			if (player.posX < WINDOW_HEIGHT){
				player.posX += 1;
			}
			if (player.box.x < WINDOW_HEIGHT){
				player.box.x += 1;
			}
		/*	player.posX = (player.posX < 640) ? player.posX += 1 : player.posX;
			player.box.x = (player.box.x < 640) ? player.box.x += 1 : player.box.x;*/
		}


		if (kbState[SDL_SCANCODE_LEFT]) {
			if (player.posX > 0){
				//animSet(&playerAnimData, &playerWalkingLeftDef);
				player.posX -= 1;
			}
			if (player.box.x > 0){
				player.box.x -= 1;
			}
			//animSet(&playerAnimData, &playerStandLeftDef);
		

			//player.posX = (player.posX > 0) ? player.posX -= 1 : player.posX;
			//player.box.x = (player.box.x > 0) ? player.box.x -= 1 : player.box.x;
		}

		if (kbState[SDL_SCANCODE_UP]) {
			printf("Player: %f\n", player.posY);
			player.posY = (player.posY >= 0) ? player.posY -= 1 : player.posY;
			player.box.y = (player.box.y >= 0) ? player.box.y -= 1 : player.box.y;

			// If player intersects with inner camera we need to move it with him
			if (AABBIntersect(&player.box, &camera.innerBox)) {
				camera.posY = (camera.posY > 0) ? camera.posY -= 4 : camera.posY;
				camera.outerBox.x = (camera.outerBox.y > 0) ? camera.outerBox.y -= 4 : camera.outerBox.y;
				camera.innerBox.y = (camera.innerBox.y > 0) ? camera.innerBox.y -= 4 : camera.innerBox.y;
			}
		}

		// Need to handle player going downwards
		if (kbState[SDL_SCANCODE_DOWN]) {
			player.posY = (player.posY < 640) ? player.posY += 1 : player.posY;
			player.box.y = (player.box.y < 640) ? player.box.y += 1 : player.box.y;
		}

		// Update platforms to move down
		/*UNTICK WHEN DONE KEVIN*/
		//platformsTick(platforms);




		

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Update player
	
		if (playerAnimData.curFrame == 7) {
			animReset(&playerAnimData);
		} else {
		animTick(&playerAnimData, deltaTime);
		}

		/*Records previous player position (For collision detection)*/
		playerPrevX = player.posX;
		playerPrevY = player.posY;


//////////////////////////////PLATFORMS////////////////////////////////////////////////////
		//// Draw the platforms
		//for (int i = 0; i < NUM_PLATFORMS; i++) {
		//	// Draw simple sprite here. Can make this more advanced later
		//	if (AABBIntersect(&platforms[i].box, &camera.outerBox)){
		//		glDrawSprite(lambda,
		//			platforms[i].posX,
		//			platforms[i].posY,
		//			platforms[i].width,
		//			platforms[i].height);
		//	}
		//}

		//// Need to cycle out old platforms and create new ones
		//cyclePlatforms(platforms, camera);
//////////////////////////////PLATFORMS////////////////////////////////////////////////////




		// This draws the player
		animDraw(&playerAnimData, player.posX - camera.posX, player.posY - camera.posY, 40, 40);
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

void platformsTick(Platform platforms[]) {
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		platforms[i].posY = platforms[i].posY + 1;
		platforms[i].box.y = platforms[i].box.y + 1;
		platforms[i] = platforms[i];
	}
}

void cyclePlatforms(Platform platforms[], Camera camera) {
	Platform platform;
	AABB box;
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
			/*Undo this later for Tim*/
			//platform.box = box;
			platforms[i] = platform;
			lastStep = posY;
		}
	}
}
