#include <GL/glew.h>

/*Changing to SDL2/SDL.h for Tim*/
#include <SDL/SDL.h>
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
#define WALKING_ANIMS_PATH "../assets/walking-animation/tga/"
<<<<<<< HEAD

/*Game States*/
enum game_states{
	menu_screen,
	gameplay
};
=======
>>>>>>> origin/Kevin

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
<<<<<<< HEAD
	AnimFrameDef frames[4];
=======
	AnimFrameDef frames[8];
>>>>>>> origin/master
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
<<<<<<< HEAD
	float yVelocity;
	float jumpTimeRemaining;
=======
<<<<<<< HEAD
=======
    float yVelocity;
    float jumpTimeRemaining;
>>>>>>> origin/master
>>>>>>> origin/Kevin
	int nearMissTries;
	bool isJumping;
	bool jumpAgain;
	AABB box;
} Player;

typedef struct Camera {
<<<<<<< HEAD
	AABB box;
=======
	AABB innerBox;
	AABB outerBox;
>>>>>>> origin/master
	int posX;
	int posY;
} Camera;

<<<<<<< HEAD
typedef struct Lava {
	AABB box;
	int posX;
	int posY;
} Lava;

=======
<<<<<<< HEAD
typedef struct Mushroom {
	float posX;
	float posY;
	bool captured;
	AABB box;
} Mushroom;

=======
>>>>>>> origin/master
>>>>>>> origin/Kevin
typedef struct BackgroundTile {
	AABB box;
	int spriteId;
	bool collision;
} BackgroundTile;

typedef struct Platform {
<<<<<<< HEAD
	float posX;
	float posY;
=======
	int posX;
	int posY;
>>>>>>> origin/master
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
<<<<<<< HEAD

=======
int lastStep = 0;
>>>>>>> origin/master


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

<<<<<<< HEAD
	/*game_state*/
	enum game_states currentState = menu_screen;

	// Create the background texture array. Going to load
	// everything at the same time for now. Maybe there
	// is a more efficient way to load this later
	GLuint thelava = glTexImageTGAFile("lava.tga", NULL, NULL);
	GLuint lambda = glTexImageTGAFile("../assets/test/lambda.tga", NULL, NULL);
	GLuint title_screen = glTexImageTGAFile("title_screen.tga", NULL, NULL);
=======
<<<<<<< HEAD


	/*Loading of animation for player*/
	playerStandLeft = glTexImageTGAFile("standFaceLeft.tga", NULL, NULL);
	playerStandRight = glTexImageTGAFile("standFaceRight.tga", NULL, NULL);

	

	// Create tehe background texture array. Going to load
	// everything at the same time for now. Maybe there
	//// is a more efficient way to load this later
	//GLuint lambda = glTexImageTGAFile("../assets/test/lambda.tga", NULL, NULL);
	//GLuint aperture = glTexImageTGAFile("aperture.tga", NULL, NULL);

>>>>>>> origin/Kevin

	textures[0] = glTexImageTGAFile("standFaceLeft.tga", NULL, NULL);

	textures[1] = glTexImageTGAFile("standFaceRight.tga", NULL, NULL);

<<<<<<< HEAD


	/*Player Walking Left Animation*/
	textures[2] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft1.tga", NULL, NULL);
	textures[3] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft2.tga", NULL, NULL);
	textures[4] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft3.tga", NULL, NULL);
	textures[5] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft4.tga", NULL, NULL);
	textures[6] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft5.tga", NULL, NULL);
	textures[7] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft6.tga", NULL, NULL);
	textures[8] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft7.tga", NULL, NULL);
	textures[9] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkLeft8.tga", NULL, NULL);

=======
	textures[2] = glTexImageTGAFile("walkLeft1.tga", NULL, NULL);
	textures[3] = glTexImageTGAFile("walkLeft2.tga", NULL, NULL);
	textures[4] = glTexImageTGAFile("walkLeft3.tga", NULL, NULL);
	textures[5] = glTexImageTGAFile("walkLeft4.tga", NULL, NULL);
	textures[6] = glTexImageTGAFile("walkLeft5.tga", NULL, NULL);
	textures[7] = glTexImageTGAFile("walkLeft6.tga", NULL, NULL);
	textures[8] = glTexImageTGAFile("walkLeft7.tga", NULL, NULL);
	textures[9] = glTexImageTGAFile("walkLeft8.tga", NULL, NULL);

	textures[10] = glTexImageTGAFile("walkRight1.tga", NULL, NULL);
	textures[11] = glTexImageTGAFile("walkRight3.tga", NULL, NULL);
	textures[12] = glTexImageTGAFile("walkRight4.tga", NULL, NULL);
	textures[13] = glTexImageTGAFile("walkRight5.tga", NULL, NULL);
	textures[14] = glTexImageTGAFile("walkRight6.tga", NULL, NULL);
	textures[15] = glTexImageTGAFile("walkRight7.tga", NULL, NULL);
	textures[16] = glTexImageTGAFile("walkRight8.tga", NULL, NULL);


=======
	// Create the background texture array. Going to load
	// everything at the same time for now. Maybe there
	// is a more efficient way to load this later
	GLuint lambda = glTexImageTGAFile("../assets/test/lambda.tga", NULL, NULL);

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

>>>>>>> origin/Kevin
	/*Player Walking Right Animation*/
	textures[10] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight1.tga", NULL, NULL);
	textures[11] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight2.tga", NULL, NULL);
	textures[12] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight3.tga", NULL, NULL);
	textures[13] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight4.tga", NULL, NULL);
	textures[14] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight5.tga", NULL, NULL);
	textures[15] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight6.tga", NULL, NULL);
	textures[16] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight7.tga", NULL, NULL);
	textures[17] = glTexImageTGAFile(WALKING_ANIMS_PATH "walkRight8.tga", NULL, NULL);
<<<<<<< HEAD

=======
>>>>>>> origin/master
>>>>>>> origin/Kevin

	// Logic to keep track of keyboard pushes
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };
	const unsigned char* kbState = NULL;
	kbState = SDL_GetKeyboardState(NULL);

	// Need to keep track of when to redraw frames
	Uint32 lastFrameMs = 0;
	Uint32 currentFrameMs = SDL_GetTicks();
	Uint32 lavaTimer = SDL_GetTicks();

	/*Lava initialization*/
	Lava lava1;
	lava1.posX = 0;
	lava1.posY = WINDOW_HEIGHT;
	lava1.box.x = 0;
	lava1.box.y = WINDOW_HEIGHT;
	lava1.box.w = WINDOW_WIDTH;
	lava1.box.h = 1;

<<<<<<< HEAD
	// Some initialization for the background
	BackgroundTile background[40][40];
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
	}

=======
>>>>>>> origin/master
	// Set options for camera coordinates to draw background
	Camera camera;
	camera.posX = 0;
	camera.posY = 0;
<<<<<<< HEAD
	camera.box.x = 0;
	camera.box.y = 0;
	camera.box.w = 640;
	camera.box.h = 480;

	// Set options for the player coordinates
	Player player;
	player.posX = 321;
	player.posY = 241;
	player.box.x = 321;
	player.box.y = 241;
	player.box.w = 30;
	player.box.h = 30;
=======
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
<<<<<<< HEAD
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
=======
	player.posX = 0;
	player.posY = 610;
	player.box.x = 0;
	player.box.y = 610;
	player.box.w = 30;
	player.box.h = 30;
    player.yVelocity = 10;
    player.jumpTimeRemaining = 4000;
>>>>>>> origin/master
>>>>>>> origin/Kevin
	player.nearMissTries = 5;
	int playerPrevX = 0;
	int playerPrevY = 0;

<<<<<<< HEAD
	AnimData playerAnimData;
	AnimDef playerAnimDef;
	playerAnimData.curFrame = 0;
	playerAnimData.timeToNextFrame = 0.1;
	playerAnimData.isPlaying = true;
	playerAnimDef.name = "player";
	playerAnimDef.numFrames = 4;
	playerAnimDef.frames[0].frameNum = 0;
	playerAnimDef.frames[0].frameTime = 0.1;
	playerAnimDef.frames[1].frameNum = 1;
	playerAnimDef.frames[1].frameTime = 0.1;
	playerAnimDef.frames[2].frameNum = 2;
	playerAnimDef.frames[2].frameTime = 0.1;
	playerAnimDef.frames[3].frameNum = 3;
	playerAnimDef.frames[3].frameTime = 0.1;
	playerAnimData.def = &playerAnimDef;

	// Initializing other objects
	Mushroom m1;
	m1.captured = false;
	m1.posX = 160;
	m1.posY = 40;
	m1.box.x = 160;
	m1.box.y = 40;
	m1.box.w = 40;
	m1.box.h = 40;

	// Create initial set of Platforms
	Platform platforms[8];
	Platform platform;
	AABB box;
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		int posX = rand() % 30;
		int posY = i + MAX_JUMP_HEIGHT;
=======
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
	playerAnimData.def = &playerWalkingLeftDef;

	AnimDef playerWalkingRightDef;
	playerWalkingRightDef.name - "PlayerWalkingRight";
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

	//////////////////////////////PLATFORMS////////////////////////////////////////////////////

	// Create initial set of Platforms
	Platform platforms[NUM_PLATFORMS];
	Platform platform;
	AABB box;
	for (int i = 0; i < NUM_PLATFORMS; i++) {
		int posX = rand() % WINDOW_WIDTH;
<<<<<<< HEAD
		// Add 10 from jump height to make things easier to reach
		int posY = lastStep + (MAX_JUMP_HEIGHT + 5);
=======
		int posY = lastStep + MAX_JUMP_HEIGHT;
>>>>>>> origin/master
>>>>>>> origin/Kevin
		int width = rand() % MAX_PLAT_WIDTH + 10;
		platform.posY = posY;
		platform.posX = posX;
		platform.width = width;
<<<<<<< HEAD
		platform.height = MAX_PLAT_HEIGHT;
		platform.box.x = posX;
		platform.box.y = posY;
		platform.box.w = width;
		platform.box.h = MAX_PLAT_HEIGHT;
		platform.box = box;
		platforms[i] = platform;
	}

	// The game loop
	char shouldExit = 0;
	while (!shouldExit) {
=======
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
	//////////////////////////////PLATFORMS////////////////////////////////////////////////////


	// The game loop

	char shouldExit = 0;
<<<<<<< HEAD
	char startPlats = false;
	while (!shouldExit)
	{
		playerAnimData.isPlaying = false;

		/*Printing for diagnostic issue*/
		//printf("%d\n", SDL_GetTicks());

=======
    char startPlats = false;
	while (!shouldExit) 
	{
		playerAnimData.isPlaying = false;
	
>>>>>>> origin/Kevin
		// Calculating frame updates
		currentFrameMs = SDL_GetTicks();
		float deltaTime = (currentFrameMs - lastFrameMs) / 1000.0f;


>>>>>>> origin/master
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
<<<<<<< HEAD
		if (kbState[SDL_SCANCODE_RIGHT]) {
			player.posX = (player.posX < 640) ? player.posX += 1 : player.posX;
			player.box.x = (player.box.x < 640) ? player.box.x += 1 : player.box.x;

			/*Set Animation for walk right*/
		}
		if (kbState[SDL_SCANCODE_LEFT]) {
			player.posX = (player.posX > 0) ? player.posX -= 1 : player.posX;
			player.box.x = (player.box.x > 0) ? player.box.x -= 1 : player.box.x;

			/*Set Animation for walk left*/

		}
		if (kbState[SDL_SCANCODE_UP]) {
			player.posY = (player.posY > 0) ? player.posY -= 1 : player.posY;
			player.box.y = (player.box.y > 0) ? player.box.y -= 1 : player.box.y;

			/*This will be deleted*/
		}
		if (kbState[SDL_SCANCODE_DOWN]) {
			player.posY = (player.posY < 640) ? player.posY += 1 : player.posY;
			player.box.y = (player.box.y < 640) ? player.box.y += 1 : player.box.y;

			/*most likely deleted*/
		}

		if (kbState[SDL_SCANCODE_D]) {
			camera.posX = (camera.posX < 640) ? camera.posX += 4 : camera.posX;
			camera.box.x = (camera.box.x < 640) ? camera.box.x += 4 : camera.box.x;

			/*walk right*/
		}
		if (kbState[SDL_SCANCODE_A]) {
			camera.posX = (camera.posX > 0) ? camera.posX -= 4 : camera.posX;
			camera.box.x = (camera.box.x > 0) ? camera.box.x -= 4 : camera.box.x;

			/*walk left*/
		}
		if (kbState[SDL_SCANCODE_W]) {
			camera.posY = (camera.posY > 0) ? camera.posY -= 4 : camera.posY;
			camera.box.y = (camera.box.y > 0) ? camera.box.y -= 4 : camera.box.y;

			/*most likely deleted*/
		}
		if (kbState[SDL_SCANCODE_S]) {
			camera.posY = (camera.posY < 640) ? camera.posY += 4 : camera.posY;
			camera.box.y = (camera.box.y < 640) ? camera.box.y += 4 : camera.box.y;
		}

		// Calculating frame updates
		currentFrameMs = SDL_GetTicks();
		float deltaTime = (currentFrameMs - lastFrameMs) / 1000.0f;
=======

<<<<<<< HEAD
		/*Insert Game States Here*/

		switch (currentState){
		case menu_screen:
			if (kbState[SDL_SCANCODE_RETURN]){
				currentState = gameplay;
=======
		if (kbState[SDL_SCANCODE_RIGHT]) {
			playerAnimData.def = &playerWalkingRightDef;
			playerAnimData.isPlaying = true;
			if (player.posX < WINDOW_HEIGHT){
				player.posX += 4;
			}
			if (player.box.x < WINDOW_HEIGHT){
				player.box.x += 4;
>>>>>>> origin/Kevin
			}

<<<<<<< HEAD
			glDrawSprite(title_screen, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			break;
	
		case gameplay:

			if (kbState[SDL_SCANCODE_RIGHT]) {
				playerAnimData.def = &playerWalkingRightDef;
				playerAnimData.isPlaying = true;
				if (player.posX < WINDOW_HEIGHT){
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
=======
		if (kbState[SDL_SCANCODE_LEFT]) {
			playerAnimData.def = &playerWalkingLeftDef;
			playerAnimData.isPlaying = true;
			if (player.posX > 0){
				player.posX -= 4;
			}
			if (player.box.x > 0){
				player.box.x -= 4;
>>>>>>> origin/Kevin
			}

<<<<<<< HEAD
			if (kbState[SDL_SCANCODE_UP]) {
				startPlats = true;
				player.isJumping = (player.jumpAgain) ? true : false;
				player.jumpAgain = false;
=======
		
		if (kbState[SDL_SCANCODE_UP]) {
            startPlats = true;
			printf("Player: %f\n", player.posY);

            if (player.jumpTimeRemaining > 0) {
                player.yVelocity = MAX_JUMP_HEIGHT;
                player.jumpTimeRemaining -= 1000;

                // Accounting for gravity with player
                player.posY = player.posY - player.yVelocity;
                player.box.y = player.box.y - player.yVelocity;
            }
            else {
                player.yVelocity = 10;
                // Accounting for gravity with player
                player.posY = player.posY + player.yVelocity;
                player.box.y = player.box.y + player.yVelocity;
            }

			// If player intersects with inner camera we need to move it with him
			if (AABBIntersect(&player.box, &camera.innerBox)) {
				camera.posY = (camera.posY > 0) ? camera.posY -= 4 : camera.posY;
				camera.outerBox.x = (camera.outerBox.y > 0) ? camera.outerBox.y -= 4 : camera.outerBox.y;
				camera.innerBox.y = (camera.innerBox.y > 0) ? camera.innerBox.y -= 4 : camera.innerBox.y;
>>>>>>> origin/Kevin
			}

<<<<<<< HEAD
			// Update platforms to move down
			if (startPlats) {
				platformsTick(platforms);
			}

			glClearColor(1, 1, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
=======
		// Need to handle player going downwards
		if (kbState[SDL_SCANCODE_DOWN]) {
			//player.posY = (player.posY < 640) ? player.posY += 1 : player.posY;
			//player.box.y = (player.box.y < 640) ? player.box.y += 1 : player.box.y;
		}

		// Update platforms to move down
        if (startPlats) {
		    platformsTick(platforms);
        }
>>>>>>> origin/master
>>>>>>> origin/Kevin

			// Update player	
			if (playerAnimData.curFrame == 7) {
				animReset(&playerAnimData);
			}
			else {
				animTick(&playerAnimData, deltaTime);
			}

<<<<<<< HEAD
=======
<<<<<<< HEAD
		// Update player
		if (playerAnimData.curFrame == 3) {
			animReset(&playerAnimData);
		}
		else {
			animTick(&playerAnimData, deltaTime);
		}

		// Update Objects
		if (m1AnimData.curFrame == 3) {
			animReset(&m1AnimData);
		}
		else {
			animTick(&m1AnimData, deltaTime);
		}
		if (m2AnimData.curFrame == 3) {
			animReset(&m2AnimData);
		}
		else {
			animTick(&m2AnimData, deltaTime);
		}
		if (m3AnimData.curFrame == 3) {
			animReset(&m3AnimData);
		}
		else {
			animTick(&m3AnimData, deltaTime);
		}

		// Check for mushroom collisions and update
		if (AABBIntersect(&player.box, &m1.box)) {
			m1.captured = true;
		}
		if (AABBIntersect(&player.box, &m2.box)) {
			m2.captured = true;
		}
		if (AABBIntersect(&player.box, &m3.box)) {
			m3.captured = true;
		}





		// Check for wall collisions and update
		for (int i = 0; i < 40; i++) {
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
		}

		playerPrevX = player.posX;
		playerPrevY = player.posY;

		// This draws the background.
		// for (int i = 0; i < 40; i++) {
		// 	for (int j = 0; j < 40; j++) {
		// 		if (AABBIntersect(&camera.box, &background[i][j].box)) {
		// 			if (background[i][j].spriteId == 0) {
		// 				glDrawSprite(aperture,
		// 					(j * 40) - camera.posX,
		// 					(i * 40) - camera.posY,
		// 					40,
		// 					40);
		// 			}
		// 			else {
		// 				glDrawSprite(lambda,
		// 					(j * 40) - camera.posX,
		// 					(i * 40) - camera.posY,
		// 					40,
		// 					40);
		// 			}
		// 		}
		// 	}
		// }

		// This draws the other objects
		//if (AABBIntersect(&camera.box, &m1.box) && !m1.captured) {
		//	animDraw(&m1AnimData, m1.posX - camera.posX, m1.posY - camera.posY, 40, 40);
		//}
		//if (AABBIntersect(&camera.box, &m2.box) && !m2.captured) {
		//	animDraw(&m2AnimData, m2.posX - camera.posX, m2.posY - camera.posY, 40, 40);
		//}
		//if (AABBIntersect(&camera.box, &m3.box) && !m3.captured) {
		//	animDraw(&m2AnimData, m3.posX - camera.posX, m3.posY - camera.posY, 40, 40);
		//}

		// Draw the platforms
		for (int i = 0; i < NUM_PLATFORMS; i++) {
			// Draw simple sprite here. Can make this more advanced later
			glDrawSprite(lambda,
				platforms[i].posX - camera.posX,
				platforms[i].posY - camera.posY,
				platforms[i].width,
				platforms[i].height);
		}

=======
		// Update player	
		if (playerAnimData.curFrame == 7) {
			animReset(&playerAnimData);
		} else {
		    animTick(&playerAnimData, deltaTime);
		}
>>>>>>> origin/Kevin


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
				for (int i = 0; i < NUM_PLATFORMS; i++) {
					if (AABBIntersect(&player.box, &platforms[i].box)) {
						player.yVelocity = 1;
						player.posY = playerPrevY;
						player.box.y = playerPrevY;
					}
				}
			}

			// Draw the platforms
			for (int i = 0; i < NUM_PLATFORMS; i++) {
				// Draw simple sprite here. Can make this more advanced later
				if (AABBIntersect(&platforms[i].box, &camera.outerBox)){
					glDrawSprite(lambda,
						platforms[i].posX,
						platforms[i].posY,
						platforms[i].width,
						platforms[i].height);
				}
			}

			printf("He %f %f\n", player.posX, player.posY);
			// Need to cycle out old platforms and create new ones
			cyclePlatforms(platforms, camera);

			/*This draws the lava*/


			// This draws the player


			lava1.box.h++;
			lava1.posY--;
			if (SDL_GetTicks() >= 10000){
				glDrawSprite(thelava, lava1.posX, lava1.posY, WINDOW_WIDTH, lava1.box.h);
			}
			animDraw(&playerAnimData, player.posX - camera.posX, player.posY - camera.posY, 40, 40);
			break;

<<<<<<< HEAD
		}
=======
>>>>>>> origin/master
		// This draws the player
		animDraw(&playerAnimData, player.posX - camera.posX, player.posY - camera.posY, 40, 40);
>>>>>>> origin/Kevin
		SDL_GL_SwapWindow(window);
	}
<<<<<<< HEAD

=======
>>>>>>> origin/master
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
<<<<<<< HEAD
=======
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
	        AABB box;
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
<<<<<<< HEAD
=======
			platform.box = box;
>>>>>>> origin/Kevin
			platforms[i] = platform;
		}
	}
<<<<<<< HEAD
}
=======
>>>>>>> origin/master
}
>>>>>>> origin/Kevin
