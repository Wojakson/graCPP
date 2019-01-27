//  based on tutorial http://lazyfoo.net/tutorials/SDL/

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <string>
#include <ctime>
#include <list>

const int screen_width = 640;
const int screen_height = 480;
int gameStatus;
int difficultyLevel;

using namespace::std;

class l_texture
{
public:
	//Initializes variables
	l_texture();

	//Deallocates memory
	~l_texture();

	//Loads image at specified path
	bool loadFromFile(const string& path);

#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(string textureText, SDL_Color textColor);
#endif

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue) const;

	//Set blending
	void setBlendMode(SDL_BlendMode blending) const;

	//Set alpha modulation
	void setAlpha(Uint8 alpha) const;

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) const;

	//Gets image dimensions
	int getWidth() const;
	int getHeight() const;

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

class Difficulty
{
public:
	int angelMovement;
	int lightMovement;
	int devilMovement;
	int fireFrequency;
	int fireMovement;
	Difficulty();
};

class Menu
{
public:
	Menu();
	void handleEvent(SDL_Event & e, int& difficultyLevel, Difficulty& difficulty);
	static void render();
};

class Background
{
public:
	Background();
	static void render();
};

class WinScreen
{
public:
	WinScreen();
	static void render();
};

class LoseScreen
{
public:
	LoseScreen();
	static void render();
};

class Angel
{
public:
	static const int ANGEL_WIDTH = 46;
	static const int ANGEL_HEIGHT = 98;
	static const int ANGEL_VEL = 1;
	Angel();
	int mPosX, mPosY;

	void handleEvent(SDL_Event & e);
	void move(Difficulty& difficulty);
	void render() const;
	SDL_Rect mCollider{};

private:
	int mVelX, mVelY;

};

class Light
{
public:
	static const int LIGHT_WIDTH = 15;
	static const int LIGHT_HEIGHT = 60;
	static const int LIGHT_VEL = 2;

	Light();
	int mPosX, mPosY;

	void handleEvent(SDL_Event& e);
	void move(Angel & angel, Difficulty& difficulty);
	void render() const;

	bool activated = false;
	SDL_Rect mCollider{};

private:
	int mVelX, mVelY;
};

class Devil
{
public:
	static const int DEVIL_WIDTH = 45;
	static const int DEVIL_HEIGHT = 108;
	static const int DEVIL_VEL = 2;

	Devil();

	void move(Light & light, Difficulty& difficulty, int& gameStatus);
	void render() const;

	bool facingRight = true;
	int mPosX, mPosY;

private:
	int mVelX, mVelY;
	SDL_Rect mCollider{};
};

class Fire
{
public:
	static const int FIRE_WIDTH = 45;
	static const int FIRE_HEIGHT = 45;
	static const int FIRE_VEL = 2;

	Fire();
	int acceleration;
	int velFromAcceleration;
	int mPosX, mPosY;
	int mVelX, mVelY;
	bool available = true;
	bool activated = false;

	void move(Devil& devil, Angel& angel, SDL_Rect& horizonLine, int id, int& gameStatus, Difficulty& difficulty);
	void render() const;

	SDL_Rect mCollider{};
};

bool init();
bool loadMedia();
void close();
bool checkCollision(SDL_Rect a, SDL_Rect b);

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;


l_texture gBackgroundSeaTexture;
l_texture gAngelTexture;
l_texture gDevilTexture;
l_texture gLightTexture;
l_texture gFireTexture;
l_texture gWinScreenTexture;
l_texture gLoseScreenTexture;
l_texture gMenuTexture;

l_texture::l_texture()
{
	mTexture = nullptr;
	mWidth = 0;
	mHeight = 0;
}

l_texture::~l_texture()
{
	free();
}

bool l_texture::loadFromFile(const string& path)
{
	free();

	SDL_Texture* newTexture = nullptr;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == nullptr)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 255, 0, 255));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == nullptr)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	mTexture = newTexture;
	return mTexture != nullptr;
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

void l_texture::free()
{
	if (mTexture != nullptr)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = nullptr;
		mWidth = 0;
		mHeight = 0;
	}
}

void l_texture::setColor(const Uint8 red, Uint8 green, Uint8 blue) const
{
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void l_texture::setBlendMode(SDL_BlendMode blending) const
{
	SDL_SetTextureBlendMode(mTexture, blending);
}

void l_texture::setAlpha(Uint8 alpha) const
{
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void l_texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) const
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != nullptr)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int l_texture::getWidth() const
{
	return mWidth;
}

int l_texture::getHeight() const
{
	return mHeight;
}

Background::Background()
= default;

WinScreen::WinScreen()
= default;

LoseScreen::LoseScreen()
= default;

Menu::Menu()
= default;

Difficulty::Difficulty()
{
	angelMovement = 1;
	lightMovement = 1;
	devilMovement = 1;
	fireFrequency = 100;
	fireMovement = 1;
}


Angel::Angel()
{
	mPosX = 0;
	mPosY = 10;

	mCollider.w = ANGEL_WIDTH;
	mCollider.h = ANGEL_HEIGHT;

	mVelX = 0;
	mVelY = 0;
}

Light::Light()
{
	mPosX = 666;
	mPosY = 666;

	mCollider.w = LIGHT_WIDTH;
	mCollider.h = LIGHT_HEIGHT;

	mVelX = 0;
	mVelY = 0;
}

Devil::Devil()
{
	mPosX = 0;
	mPosY = 200 + (rand() % 200);

	mCollider.w = DEVIL_WIDTH;
	mCollider.h = DEVIL_HEIGHT;

	mVelX = 1;
	mVelY = 0;
}

Fire::Fire()
{
	mPosX = 800;
	mPosY = 800;

	acceleration = 0;
	velFromAcceleration = 0;

	mCollider.w = FIRE_WIDTH;
	mCollider.h = FIRE_HEIGHT;

	mVelX = 0;
	mVelY = 100;
}

//The music that will be played
// Mix_Music *music = NULL;

void Angel::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_DOWN: mVelY -= ANGEL_VEL; break;
		case SDLK_LEFT: mVelX -= ANGEL_VEL; break;
		case SDLK_RIGHT: mVelX += ANGEL_VEL; break;
		default:
			break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_DOWN: mVelY -= ANGEL_VEL; break;
		case SDLK_LEFT: mVelX += ANGEL_VEL; break;
		case SDLK_RIGHT: mVelX -= ANGEL_VEL; break;
		default:
			break;
		}
	}
}

void Light::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_DOWN:
			if (mPosY < 20) {
				mVelY = 1;
				activated = true;
			}
			break;
		default:
			break;
		}
	}
}

void Menu::handleEvent(SDL_Event& e, int& difficultyLevel, Difficulty& difficulty)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_1:
			difficultyLevel = 1;
			difficulty.fireFrequency = 3;
			difficulty.fireMovement = 3;
			difficulty.angelMovement = 2;
			difficulty.lightMovement = 5;
			difficulty.devilMovement = 1;
			break;
		case SDLK_2:
			difficultyLevel = 2;
			difficulty.fireFrequency = 4;
			difficulty.fireMovement = 4;
			difficulty.angelMovement = 2;
			difficulty.lightMovement = 4;
			difficulty.devilMovement = 3;
			break;
		case SDLK_3:
			difficultyLevel = 3;
			difficulty.fireFrequency = 6;
			difficulty.fireMovement = 5;
			difficulty.angelMovement = 2;
			difficulty.lightMovement = 3;
			difficulty.devilMovement = 4;
			break;
		case SDLK_4:
			difficultyLevel = 4;
			difficulty.fireFrequency = 1;
			difficulty.fireMovement = 7;
			difficulty.angelMovement = 1;
			difficulty.lightMovement = 5;
			difficulty.devilMovement = 10;
			break;
		case SDLK_ESCAPE:
			SDL_Quit();
			break;
		default:
			break;
		}
	}
}

void Angel::move(Difficulty& difficulty)
{
	mPosX += mVelX * difficulty.angelMovement;
	mCollider.x = mPosX;

	if ((mPosX < 0) || (mPosX + ANGEL_WIDTH > screen_width))
	{
		mPosX -= mVelX;
		mCollider.x = mPosX;
	}

	mCollider.y = mPosY;
}

void Light::move(Angel& angel, Difficulty& difficulty)
{
	if (activated) {
		mPosX = angel.mPosX;
		mPosY = angel.mPosY;
		activated = !activated;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;
	mPosY += mVelY * difficulty.lightMovement;

	if ((mPosY < 0) || (mPosY + LIGHT_HEIGHT > screen_height))
	{
		mVelY = 0;
		mPosY = 1;
		mPosX = 1;
		mCollider.y = mPosY;
	}
}

void Devil::move(Light& light, Difficulty& difficulty, int& gameStatus)
{
	if (facingRight) {
		mPosX += mVelX * difficulty.devilMovement;
	}
	else {
		mPosX -= mVelX * difficulty.devilMovement;
	}

	mCollider.x = mPosX;
	mCollider.y = mPosY;

	if ((mPosX < 0) || (mPosX + DEVIL_WIDTH > screen_width))
	{
		mCollider.x = mPosX;
		const int changePosition = mPosY + rand() % 50 + (-10);
		if (changePosition < (screen_width - 20) && changePosition > 5)
		{
			mPosY = changePosition;
		}
		facingRight = !facingRight;
	}

	if (checkCollision(mCollider, light.mCollider)) {
		gameStatus = 3;
	}

	mCollider.y = mPosY;
}

void Fire::move(Devil& devil, Angel& angel, SDL_Rect& horizonLine, int id, int& gameStatus, Difficulty& difficulty)
{
	if (!available)
	{
		if (activated) {
			mPosX = devil.mPosX;
			mPosY = devil.mPosY;
			mVelY = 1;
			activated = !activated;
		}
		//Sprites acceleration 
		acceleration += 1;
		velFromAcceleration = velFromAcceleration + FIRE_VEL * 1.2;
		//acceleration += 1;
		//if(acceleration == 10 || acceleration == 20 || acceleration == 30 || acceleration == 40)
		//{
		//	velFromAcceleration = acceleration/10;
		//}


		mCollider.x = mPosX;
		mCollider.y = mPosY;
		if (acceleration % 3 == 1) {
			mPosY -= (mVelY * difficulty.fireMovement) + velFromAcceleration + 5;
		}

		if (checkCollision(mCollider, angel.mCollider)) {
			gameStatus = 2;
		}
		else if (checkCollision(mCollider, horizonLine)) {
			mVelY = 0;
			mPosY = 800;
			mPosX = 800;
			acceleration = 0;
			velFromAcceleration = 0;
			available = true;

		}

	}
}

void Background::render() {
	gBackgroundSeaTexture.render(0, 0);
}

void Menu::render() {
	gMenuTexture.render(0, 0);
}


void WinScreen::render() {
	gWinScreenTexture.render(0, 0);
}

void LoseScreen::render() {
	gLoseScreenTexture.render(0, 0);
}


void Angel::render() const
{
	gAngelTexture.render(mPosX, mPosY);
}

void Light::render() const
{
	gLightTexture.render(mPosX, mPosY);
}

void Devil::render() const
{
	gDevilTexture.render(mPosX, mPosY);
}

void Fire::render() const
{
	gFireTexture.render(mPosX, mPosY);
}

bool init()
{
	auto success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		gWindow = SDL_CreateWindow("Wojciech Antoniuk s13912", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
		if (gWindow == nullptr)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == nullptr)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				const int imgFlags = IMG_INIT_PNG;
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


	if (!gBackgroundSeaTexture.loadFromFile("castles.bmp"))
	{
		printf("Failed to load castles texture!\n");
		success = false;
	}

	if (!gAngelTexture.loadFromFile("angelIMG.bmp"))
	{
		printf("Failed to load angel texture!\n");
		success = false;
	}

	if (!gLightTexture.loadFromFile("light.bmp"))
	{
		printf("Failed to load light texture!\n");
		success = false;
	}

	if (!gDevilTexture.loadFromFile("devilIMG.bmp"))
	{
		printf("Failed to load devil texture!\n");
		success = false;
	}

	if (!gFireTexture.loadFromFile("fire.bmp"))
	{
		printf("Failed to load fire texture!\n");
		success = false;
	}

	if (!gWinScreenTexture.loadFromFile("heroes3rampart.bmp"))
	{
		printf("Failed to load heroes3rampart texture!\n");
		success = false;
	}

	if (!gLoseScreenTexture.loadFromFile("heroes3nekro.bmp"))
	{
		printf("Failed to load heroes3nekro texture!\n");
		success = false;
	}


	if (!gMenuTexture.loadFromFile("heroes3.bmp"))
	{
		printf("Failed to load menu texture!\n");
		success = false;
	}

	//Load the music
	//music = Mix_LoadMUS("heroes3.wav");

	//If there was a problem loading the music
	//if (music == NULL)
	//{
		//return false;
	//}

	return success;
}

void close()
{
	gBackgroundSeaTexture.free();
	gAngelTexture.free();
	gLightTexture.free();
	gDevilTexture.free();
	gFireTexture.free();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;
	gRenderer = nullptr;

	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//Calculate the sides of rect A
	const int leftA = a.x;
	const int rightA = a.x + a.w;
	const int topA = a.y;
	const int bottomA = a.y + a.h;

	//Calculate the sides of rect B
	const int leftB = b.x;
	const int rightB = b.x + b.w;
	const int topB = b.y;
	const int bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
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

	//If none of the sides from A are outside B
	return true;
}


int main(int argc, char* args[])
{
	// Initialize SDL video and audio systems
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	// Initialize SDL mixer
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	// Load audio files
	Mix_Music *backgroundSound = Mix_LoadMUS("heroes3.wav");

	// Start the background music
	Mix_PlayMusic(backgroundSound, -1);

	srand(time(nullptr));
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
			difficultyLevel = 0;

			SDL_Event e;
			Menu menu;
			Difficulty difficulty;
			Background background;
			WinScreen win_screen;
			LoseScreen lose_screen;
			Angel angel;
			Light light;
			Devil devil;
			Fire fire[9];

			int lightTimer = 5000;
			unsigned int lastTime = 0;
			bool shot = false;

			SDL_Rect horizonLine;
			horizonLine.x = 0;
			horizonLine.y = 50;
			horizonLine.w = 640;
			horizonLine.h = 1;

			while (!quit)
			{
				if (gameStatus == 0)
				{
					while (SDL_PollEvent(&e) != 0)
					{
						menu.handleEvent(e, difficultyLevel, difficulty);
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
					menu.render();
					if (difficultyLevel > 0)
					{
						gameStatus = 1;
					}
				}

				if (gameStatus == 1) {

					while (SDL_PollEvent(&e) != 0)
					{
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
						angel.handleEvent(e);
						light.handleEvent(e);
					}

					angel.move(difficulty);
					light.move(angel, difficulty);
					devil.move(light, difficulty, gameStatus);

					const unsigned int currentTime = SDL_GetTicks();
					if (currentTime > lastTime + lightTimer) {
						lightTimer = 100 * (difficulty.fireFrequency) + (difficulty.fireFrequency * (rand() % 1000));
						lastTime = currentTime;

						for (auto& i : fire)
						{
							if (i.available && !shot)
							{
								i.activated = true;
								i.available = false;
								shot = true;
							}
						}
						shot = false;

					}

			
					for(int i=0; i < 9; i++)
					{
						fire[i].move(devil, angel, horizonLine, i, gameStatus, difficulty);
					}


					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);

					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderDrawRect(gRenderer, &horizonLine);

					background.render();
					angel.render();
					light.render();
					devil.render();
					for (int i = 0; i < 9; i++)
					{
						fire[i].render();
					}


				}

				if (gameStatus == 2)
				{
					while (SDL_PollEvent(&e) != 0)
					{
						menu.handleEvent(e, difficultyLevel, difficulty);
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
					lose_screen.render();
				}
				if (gameStatus == 3) {
					while (SDL_PollEvent(&e) != 0)
					{
						menu.handleEvent(e, difficultyLevel, difficulty);
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
					win_screen.render();
				}
				SDL_RenderPresent(gRenderer);
				SDL_Delay(15);

			}
		}
	}

	Mix_FreeMusic(backgroundSound);
	Mix_CloseAudio();

	close();
	return 0;
}