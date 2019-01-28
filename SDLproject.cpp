#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <windows.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
int gameStatus;

class lTexture
{
public:
	lTexture();

	~lTexture();

	bool loadFromFile(std::string path);

#ifdef _SDL_TTF_H
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif

	void free();

	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	void setBlendMode(SDL_BlendMode blending);

	void setAlpha(Uint8 alpha);

	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	int getWidth();
	int getHeight();

private:
	SDL_Texture* mTexture;

	int mWidth;
	int mHeight;
};

class Soldier
{
public:
	static const int SOLDIER_WIDTH = 50;
	static const int SOLDIER_HEIGHT = 50;
	static const int SOLDIER_VEL = 1;

	int accelerationTimer;
	int acceleration;

	Soldier();

	int mPosX, mPosY;
	void handleEvent(SDL_Event& e);
	void move();
	void render();
	SDL_Rect mCollider{};

private:
	int mVelX, mVelY;
};

class Bomb
{
public:
	static const int BOMB_WIDTH = 20;
	static const int BOMB_HEIGHT = 20;
	static const int BOMB_VEL = 2;

	Bomb();
	int mPosX, mPosY;

	void handleEvent(SDL_Event& e);
	void move(Soldier & soldier);
	void render() const;
	int direction;

	bool activated = false;
	SDL_Rect mCollider{};

private:
	int mVelX, mVelY;
};

class EnemyGun
{
public:
	static const int ENEMY_GUN_WIDTH = 40;
	static const int ENEMY_GUN_HEIGHT = 40;

	EnemyGun(int mPosX, int mPosY, int direction);

	void move(Bomb& bomb, int& gameStatus);
	void render() const;

	bool alive;

	int gunDirection;
	int mPosX, mPosY;

private:
	int mVelX, mVelY;
	SDL_Rect mCollider{};
};

class Cartidge
{
public:
	static const int CARTIDGE_WIDTH = 20;
	static const int CARTIDGE_HEIGHT = 20;

	Cartidge();
	int cartidgeDirection;
	int acceleration;
	int velFromAcceleration;
	int mPosX, mPosY;
	int mVelX, mVelY;
	bool available = true;
	bool activated = false;

	void move(EnemyGun& EnemyGun, Soldier& soldier, int direction, int& gameStatus);
	void render() const;

	SDL_Rect mCollider{};
};

bool init();
bool loadMedia();
void close();
bool checkCollision(SDL_Rect a, SDL_Rect b);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

lTexture gBgTexture;
lTexture gSoldierIcon;
lTexture gWonBackground;
lTexture gLoseTexture;
lTexture gBombTexture;
lTexture gEnemyIcon;
lTexture gStoneGifTexture;

lTexture::lTexture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

lTexture::~lTexture()
{
	free();
}

bool lTexture::loadFromFile(std::string path)
{
	free();

	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 255, 0, 255));

		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface != NULL)
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}
	else
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}


	//Return success
	return mTexture != NULL;
}
#endif

void lTexture::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void lTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void lTexture::setBlendMode(SDL_BlendMode blending)
{
	SDL_SetTextureBlendMode(mTexture, blending);
}

void lTexture::setAlpha(Uint8 alpha)
{
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void lTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int lTexture::getWidth()
{
	return mWidth;
}

int lTexture::getHeight()
{
	return mHeight;
}

Soldier::Soldier()
{
	mPosX = 250;
	mPosY = 250;

	mCollider.w = SOLDIER_WIDTH;
	mCollider.h = SOLDIER_HEIGHT;

	mVelX = 0;
	mVelY = 0;
}

Bomb::Bomb()
{
	mCollider.w = BOMB_WIDTH;
	mCollider.h = BOMB_HEIGHT;
}

EnemyGun::EnemyGun(int posX, int posY, int direction)
{
	mPosX = posX;
	mPosY = posY;
	alive = true;

	mCollider.w = ENEMY_GUN_WIDTH;
	mCollider.h = ENEMY_GUN_HEIGHT;
	gunDirection = direction;

	if (direction == 1 || direction == 4) {
		mVelX = 0;
		mVelY = 1;
	}
	else {
		mVelX = 1;
		mVelY = 0;
	}
}

Cartidge::Cartidge()
{
	acceleration = 0;
	velFromAcceleration = 0;

	mCollider.w = CARTIDGE_WIDTH;
	mCollider.h = CARTIDGE_HEIGHT;
}

void Soldier::handleEvent(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= SOLDIER_VEL; break;
		case SDLK_DOWN: mVelY += SOLDIER_VEL; break;
		case SDLK_LEFT: mVelX -= SOLDIER_VEL; break;
		case SDLK_RIGHT: mVelX += SOLDIER_VEL; break;
		}
	}
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += SOLDIER_VEL; break;
		case SDLK_DOWN: mVelY -= SOLDIER_VEL; break;
		case SDLK_LEFT: mVelX += SOLDIER_VEL; break;
		case SDLK_RIGHT: mVelX -= SOLDIER_VEL; break;
		}
	}
}

void Bomb::handleEvent(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_w:
			if (mPosY < 20) {
				mVelY = 2;
				activated = true;
				direction = 1;
			}
			break;
		case SDLK_a:
			if (mPosY < 20) {
				mVelX = 2;
				activated = true;
				direction = 3;
			}
			break;
		case SDLK_s:
			if (mPosY < 20) {
				mVelY = 2;
				activated = true;
				direction = 4;
			}
			break;
		case SDLK_d:
			if (mPosX < 20) {
				mVelX = 2;
				activated = true;
				direction = 2;
			}
			break;
		default:
			break;
		}
	}
}

void Soldier::move()
{
	accelerationTimer += 1;

	if (mVelX != 0 || mVelY != 0)
	{
		switch (accelerationTimer)
		{
		case 10:
			acceleration = 2;
			break;
		case 20:
			acceleration = 4;
			break;
		case 30:
			acceleration = 6;
			break;
		case 40:
			acceleration = 8;
			break;
		}
	}
	else
	{
		accelerationTimer = 0;
		acceleration = 0;
	}

	if (mVelX > 0)
	{
		mPosX += mVelX + acceleration;
	}
	else if (mVelX < 0)
	{
		mPosX += mVelX - acceleration;
	}
	else if (mVelY < 0)
	{
		mPosY += mVelY - acceleration;
	}
	else if (mVelY > 0)
	{
		mPosY += mVelY + acceleration;
	}
	else
	{
		mPosX += mVelX;
		mPosY += mVelY;
	}

	if (mPosX < 40) {
		mPosX = 40;
	}
	if (mPosX + SOLDIER_WIDTH > SCREEN_WIDTH - 40) {
		mPosX = SCREEN_WIDTH - SOLDIER_WIDTH - 40;
	}
	if (mPosY < 40) {
		mPosY = 40;
	}
	if (mPosY + SOLDIER_HEIGHT > SCREEN_HEIGHT - 40) {
		mPosY = SCREEN_HEIGHT - SOLDIER_HEIGHT - 40;
	}
	mCollider.x = mPosX;
	mCollider.y = mPosY;
}

void Bomb::move(Soldier & soldier)
{
	if (activated) {
		mPosX = soldier.mPosX;
		mPosY = soldier.mPosY;
		activated = !activated;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;
	switch (direction)
	{
	case 1:
		mPosY -= mVelY;
		break;
	case 2:
		mPosX += mVelX;
		break;
	case 3:
		mPosX -= mVelX;
		break;
	case 4:
		mPosY += mVelY;
		break;
	default:
		break;
	}

	if ((mPosY < 0) || (mPosY + BOMB_HEIGHT > SCREEN_HEIGHT) || (mPosX < 0) || (mPosX + BOMB_WIDTH > SCREEN_WIDTH))
	{
		mVelY = 0;
		mVelX = 0;
		mPosY = 1;
		mPosX = 1;
		mCollider.y = mPosY;
	}
}

void EnemyGun::move(Bomb& bomb, int& gameStatus)
{
	switch (gunDirection)
	{
	case 1:
	{
		mPosY -= mVelY;
		break;
	}
	case 2:
	{
		mPosX += mVelX;
		break;
	}
	case 3:
	{
		mPosX -= mVelX;
		break;
	}
	case 4:
	{
		mPosY += mVelY;
		break;
	}
	default:
		break;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;

	if ((mPosX < 0) || (mPosX + ENEMY_GUN_WIDTH > SCREEN_WIDTH))
	{
		mCollider.x = mPosX;
		if (gunDirection == 3)
		{
			gunDirection = 2;
		}
		else if (gunDirection == 2) {
			gunDirection = 3;
		}
	}

	if ((mPosY < 0) || (mPosY + ENEMY_GUN_HEIGHT > SCREEN_HEIGHT))
	{
		mCollider.y = mPosY;
		if (gunDirection == 4)
		{
			gunDirection = 1;
		}
		else if (gunDirection == 1) {
			gunDirection = 4;
		}
	}

	if (checkCollision(mCollider, bomb.mCollider)) {
		alive = false;
	}
}

void Cartidge::move(EnemyGun& enemyGun, Soldier& soldier, int direction, int& gameStatus)
{
	if (!available)
	{
		if (activated) {
			mPosX = enemyGun.mPosX;
			mPosY = enemyGun.mPosY;
			activated = !activated;
		}

		acceleration += 1;
		if (acceleration == 10 || acceleration == 20 || acceleration == 30 || acceleration == 40)
		{
			velFromAcceleration = acceleration / 10;
		}

		mCollider.x = mPosX;
		mCollider.y = mPosY;

		if (acceleration % 3 == 1) {

			switch (direction)
			{
			case 1:
			{
				mPosY += mVelY + velFromAcceleration;
				break;
			}
			case 2:
			{
				mPosX += mVelX + velFromAcceleration;
				break;
			}
			case 3:
			{
				mPosX -= mVelX + velFromAcceleration;
				break;
			}
			case 4:
			{
				mPosY -= mVelY + velFromAcceleration;
				break;
			}
			default:
				break;
			}
		}

		if (checkCollision(mCollider, soldier.mCollider)) {
			gameStatus = 2;
		}

		if (mPosX > SCREEN_WIDTH || mPosY > SCREEN_HEIGHT) {
			mVelX = 0;
			mVelY = 0;
			mPosY = 800;
			mPosX = 800;
			acceleration = 0;
			velFromAcceleration = 0;
			available = true;
		}
	}
}

void Soldier::render()
{
	gSoldierIcon.render(mPosX, mPosY);
}

void Bomb::render() const
{
	gBombTexture.render(mPosX, mPosY);
}

void EnemyGun::render() const
{
	gEnemyIcon.render(mPosX, mPosY);
}

void Cartidge::render() const
{
	gStoneGifTexture.render(mPosX, mPosY);
}

bool init()
{
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		gWindow = SDL_CreateWindow("SDL game for SGD lecture.", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	bool success = true;

	if (!gBgTexture.loadFromFile("joungle_background.png"))
	{
		printf("Failed to load joungle texture!\n");
		success = false;
	}

	if (!gSoldierIcon.loadFromFile("soldier.png"))
	{
		printf("Failed to load soldier texture!\n");
		success = false;
	}

	if (!gWonBackground.loadFromFile("won_background.png"))
	{
		printf("Failed to load won texture!\n");
		success = false;
	}

	if (!gLoseTexture.loadFromFile("lose_background.png"))
	{
		printf("Failed to load lose texture!\n");
		success = false;
	}

	if (!gBombTexture.loadFromFile("Plainlb.png"))
	{
		printf("Failed to load bomb texture!\n");
		success = false;
	}

	if (!gEnemyIcon.loadFromFile("tubylec.png"))
	{
		printf("Failed to load enemyIcon texture!\n");
		success = false;
	}
	
	
	if (!gStoneGifTexture.loadFromFile("stone_gif.gif"))
	{
		printf("Failed to load stone texture!\n");
		success = false;
	}

	return success;
}

void close()
{
	gSoldierIcon.free();
	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	return true;
}

int main(int argc, char* args[])
{
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			bool quit = false;
			gameStatus = 0;

			static const int cartidgeNumber = 4;

			SDL_Event e;
			Soldier soldier;
			Bomb bomb;

			EnemyGun enemyGun1(50, 440, 1);
			EnemyGun enemyGun2(600, 50, 2);
			EnemyGun enemyGun3(50, 440, 3);
			EnemyGun enemyGun4(600, 50, 4);

			Cartidge cartidge[4][cartidgeNumber];

			int bombTimer = 2000;
			unsigned int lastTime = 0;
			bool shot = false;

			while (!quit)
			{

				if (gameStatus == 0) {

					while (SDL_PollEvent(&e) != 0)
					{
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}

						soldier.handleEvent(e);
						bomb.handleEvent(e);
					}

					soldier.move();
					bomb.move(soldier);

					if (enemyGun1.alive) {
						enemyGun1.move(bomb, gameStatus);
					}
					if (enemyGun2.alive) {
						enemyGun2.move(bomb, gameStatus);
					}
					if (enemyGun3.alive) {
						enemyGun3.move(bomb, gameStatus);
					}
					if (enemyGun4.alive) {
						enemyGun4.move(bomb, gameStatus);
					}

					if (!enemyGun1.alive && !enemyGun2.alive && !enemyGun3.alive && !enemyGun4.alive) {
						gameStatus = 3;
					}

					const unsigned int currentTime = SDL_GetTicks();
					if (currentTime > lastTime + bombTimer) {
						bombTimer = 100 + (rand() % 300);
						lastTime = currentTime;

						for (auto& j : cartidge) {
							for (int i = 0; i < cartidgeNumber; i++)
							{
								if (j[i].available && !shot) {
									j[i].activated = true;
									j[i].available = false;
									shot = true;
								}
							}
						}
						shot = false;
					}

					for (int i = 0; i < cartidgeNumber; i++) {
						if (enemyGun1.alive) {
							cartidge[0][i].move(enemyGun1, soldier, 2, gameStatus);
						}
						if (enemyGun2.alive) {
							cartidge[1][i].move(enemyGun2, soldier, 1, gameStatus);
						}
						if (enemyGun3.alive) {
							cartidge[2][i].move(enemyGun3, soldier, 4, gameStatus);
						}
						if (enemyGun4.alive) {
							cartidge[3][i].move(enemyGun4, soldier, 3, gameStatus);
						}
					}

					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);

					gBgTexture.render(0, 0);
					soldier.render();
					bomb.render();

					if (enemyGun1.alive) {
						enemyGun1.render();
					}
					if (enemyGun2.alive) {
						enemyGun2.render();
					}
					if (enemyGun3.alive) {
						enemyGun3.render();
					}
					if (enemyGun4.alive) {
						enemyGun4.render();
					}

					for (int i = 0; i < cartidgeNumber; i++)
					{
						if (enemyGun1.alive) {
							cartidge[0][i].render();
						}
						if (enemyGun2.alive) {
							cartidge[1][i].render();
						}
						if (enemyGun3.alive) {
							cartidge[2][i].render();
						}
						if (enemyGun4.alive) {
							cartidge[3][i].render();
						}
					}
					SDL_RenderPresent(gRenderer);
				}

				if (gameStatus == 2) {
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);
					gLoseTexture.render(0, 0);
					SDL_RenderPresent(gRenderer);
					Sleep(50000);
					close();
				}

				if (gameStatus == 3) {
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);
					gWonBackground.render(0, 0);
					SDL_RenderPresent(gRenderer);
					Sleep(50000);
					close();
				}

			}
		}
	}

	close();
	return 0;
}