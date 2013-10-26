/*
	Breakout
	Clinton Andrews
	January 5, 2013
*/

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cmath>

// Screen attributes
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;
const int FRAME_RATE = 60;
const int PADDLE_SPEED = 10;
const int BALL_SPEED = 5;
const int PADDLE_START_X = (SCREEN_WIDTH - 100) / 2;
const int PADDLE_START_Y = 550;

TTF_Font *font = NULL;
SDL_Color text_color = {0,0,0};

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *red_brick = NULL;
SDL_Surface *blue_brick = NULL;
SDL_Surface *green_brick = NULL;
SDL_Surface *paddle_image = NULL;
SDL_Surface *ball_image = NULL;
SDL_Surface *level_text = NULL;
SDL_Surface *bricks_remaining_text = NULL;
SDL_Surface *current_score_text = NULL;
SDL_Surface *high_score_text = NULL;
SDL_Surface *lives_left_text = NULL;

int level = 0;
int bricks_remaining = 0;
int score = 0;
int lives_left = 3;

std::ofstream logger("log.txt");

void log( std::string message)
{
	logger << message << std::endl;
	logger.flush();
}
void log( char x)
{
	logger<<x<<std::endl;
}

std::string convertInt(int number)
{
	std::stringstream ss;
	ss<<number;
	return ss.str();
}

class Paddle
{
public:
	int x, y, w, h;
	int xVel;

	Paddle::Paddle();
	void handleInput(int direction);
	void Paddle::move();
};

Paddle::Paddle()
{
	x = PADDLE_START_X;
	y = PADDLE_START_Y;
	w = 100;
	h = 1;
	xVel = 0;
}

void Paddle::handleInput(int direction)
{
	switch(direction)
	{
	case 1:
		Paddle::xVel = PADDLE_SPEED;
		break;
	case 0:
		Paddle::xVel = 0;
		break;
	case -1:
		Paddle::xVel = -1 * PADDLE_SPEED;
		break;
	}
}

void Paddle::move()
{
	x += xVel;

	if(x<150)
	{
		x = 150;
	}
	else if(x+w>650)
	{
		x = 650 - w;
	}
}

class Brick
{
public:
	int x, y, w, h;
	int quality;
	bool active;
};

Paddle paddle;
std::vector<Brick> brick;

class Ball
{
public:
	int x, y, w, h;
	int xVel, yVel;

	Ball::Ball();
	void Ball::move();
	int Ball::detectCollision(int i);
	int Ball::detectCollision(Paddle paddle);
};

Ball::Ball()
{
	w = 10;
	h = 10;
	x = (SCREEN_WIDTH - w) / 2;
	y = PADDLE_START_Y - h;
	xVel = BALL_SPEED;
	yVel= -1 * BALL_SPEED;
}

void Ball::move()
{
	x += xVel;
	y += yVel;

	// Test Boundaries
	if(x < 150)
	{
		x = 150;
		xVel *= -1;
	}

	else if((x+w) > 650)
	{
		x = 650-w;
		xVel *= -1;
	}

	else if(y < 75)
	{
		y = 75;
		yVel *= -1;
	}

	// Test for Paddle
	switch(detectCollision(paddle))
	{
		// No Collision
		case 0:
			break;
		// Left or Right
		case 1:
			x -= xVel;
			y -= yVel;
			xVel *= -1;
			break;
		// Top or Bottom
		case 2:
			x -= xVel;
			y -= yVel;
			yVel *= -1;
			break;
	}

	// Test for Bricks
	for(int i=0; i<brick.size();i++)
	{
		if(brick[i].quality>0)
		{
			switch(detectCollision(i))
			{
			
				// No Collision
				case 0:
					break;
				// Left or Right
				case 1:
					x -= xVel;
					y -= yVel;
					xVel *= -1;
					break;
				// Top or Bottom
				case 2:
					x -= xVel;
					y -= yVel;
					yVel *= -1;
					break;
			}
		}
	}
}


int Ball::detectCollision(int i)
{
	// Detect Collision
	// 0 for No Collision, 1 Left, 2 Right, 3 Top, 4 Bottom, 5 Corner
	if(x + w < brick[i].x)
		return 0;
	else if(y + h < brick[i].y)
		return 0;
	else if(x - brick[i].w > brick[i].x)
		return 0;
	else if(y - brick[i].h > brick[i].y)
		return 0;
	else
	{
		brick[i].quality -= 1;
		if(brick[i].quality == 0)
		{
			score += 100;
			bricks_remaining -=1;
		}
		else if(brick[i].quality == 1)
		{
			score += 200;
		}
		else if(brick[i].quality == 2)
		{
			score += 300;
		}
		// Determine direction

		// Calculate Center of Ball to Center of Brick
		int x_center = abs((brick[i].x+brick[i].w)/2 - (x+w)/2);
		int y_center = abs((brick[i].y+brick[i].h)/2 - (y+h)/2);

		if(x_center == (brick[i].w/2 + w/2 - xVel))
			return 1;
		else
			return 2;
	}
}


int Ball::detectCollision(Paddle paddle)
{
	// Detect Collision
	// 0 for No Collision, 1 Left, 2 Right, 3 Top, 4 Bottom, 5 Corner
	if(x + w < paddle.x)
		return 0;
	else if(y + h < paddle.y)
		return 0;
	else if(x - paddle.w > paddle.x)
		return 0;
	else if(y - paddle.h > paddle.y)
		return 0;
	else
	{
		// Determine direction

		// Calculate Center of Ball to Center of Brick
		int x_center = abs((paddle.x+paddle.w)/2 - (x+w)/2);
		int y_center = abs((paddle.y+paddle.h)/2 - (y+h)/2);

		if(x_center == (paddle.w/2 + w/2 - xVel))
			return 1;
		else
			return 2;
	}
}

bool initialize();
SDL_Surface *loadImage(std::string file_path);
void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *destination);
void loadLevel(int level);
void displayGraphics();
void displayText();

Ball ball;

int main(int argc, char* args[])
{
	bool quit = false;

	// Initialize SDL
	if(initialize() == false)
	{
		return 1;
	}

	int last_tick = SDL_GetTicks();

	// Game Loop
	while (quit == false)
	{
		// User Input
		// Check if User quits
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_LEFT)
					{
						paddle.handleInput(-1);
					}
					else if(event.key.keysym.sym == SDLK_RIGHT)
					{
						paddle.handleInput(1);
					}
					break;
				case SDL_KEYUP:
					paddle.handleInput(0);
					break;
			}
		}

		// Paddle Input

		// Game Logic
		// Check if Level needs to be loaded
		if(bricks_remaining == 0)
		{
			level++;
			loadLevel(level);

			ball.x = (SCREEN_WIDTH - ball.w) / 2;
			ball.y = PADDLE_START_Y - ball.h;
			ball.xVel = BALL_SPEED;
			ball.yVel= -1 * BALL_SPEED;
			
			SDL_Delay(1000);
		}

		//Check if ball has left boundaries
		if(ball.y > 600 && lives_left > 0)
		{
			lives_left -= 1;

			ball.x = (SCREEN_WIDTH - ball.w) / 2;
			ball.y = PADDLE_START_Y - ball.h;
			ball.xVel = BALL_SPEED;
			ball.yVel= -1 * BALL_SPEED;
			
			SDL_Delay(1000);
		}

		// Move paddle
		paddle.move();

		// Move ball 
		ball.move();

		// Display
		displayGraphics();
		displayText();
		SDL_Flip(screen);

		// Cap Frame Rate
		int delta = SDL_GetTicks() - last_tick;
		if( delta < 1000 / FRAME_RATE)
		{
			SDL_Delay( (1000/FRAME_RATE) - delta);
		}
		last_tick = SDL_GetTicks();
	}

	// Quit SDL
	SDL_Quit();
	return 0;
}

bool initialize(){
	// Start all SDL Systems
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return false;
	}

	// Initialize the screen
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if(screen == NULL)
	{
		return false;
	}

	if( TTF_Init() == -1 )
    {
        return false;    
    }

	font = TTF_OpenFont("data.ttf", 16);

	background = loadImage("images/borders.bmp");
	red_brick = loadImage("images/red.bmp");
	blue_brick = loadImage("images/blue.bmp");
	green_brick = loadImage("images/green.bmp");
	paddle_image = loadImage("images/paddle.bmp");
	ball_image = loadImage("images/ball.bmp");

	// Set Caption
	SDL_WM_SetCaption("Breakout by Clinton Andrews", NULL);
	return true;
}

SDL_Surface *loadImage(std::string file_path)
{
	SDL_Surface *image = NULL;

	image = IMG_Load(file_path.c_str());

	return image;
}

void applySurface(int x, int y, SDL_Surface *source, SDL_Surface *destination)
{
	SDL_Rect coords;
	coords.x = x;
	coords.y = y;

	SDL_BlitSurface(source, NULL, screen, &coords);
}

void loadLevel(int level){
	// Open text file that contains level
	std::string s;
	std::stringstream out;
	out << (level % 4);
	s = out.str();

	std::string file_name = "levels/" + s + ".txt";

	std::ifstream file;
	file.open(file_name.c_str());
	log("opening file..." + file_name);

	// load game_board vector
	int x = 150;
	int y = 150;
	while(file)
	{
		// Get the char and assign brick characteristics
		char type;
		file >> type;
		log(type);
		switch(type)
		{
		case 'r':
			bricks_remaining++;
			brick.resize(brick.size()+1);
			brick[brick.size()-1].x=x;
			brick[brick.size()-1].y=y;
			brick[brick.size()-1].w=50;
			brick[brick.size()-1].h=25;
			brick[brick.size()-1].quality=1;
			x+=50;
			break;
		case 'g':
			bricks_remaining++;
			brick.resize(brick.size()+1);
			brick[brick.size()-1].x=x;
			brick[brick.size()-1].y=y;
			brick[brick.size()-1].w=50;
			brick[brick.size()-1].h=25;
			brick[brick.size()-1].quality=2;
			x+=50;
			break;
		case 'b':
			bricks_remaining++;
			brick.resize(brick.size()+1);
			brick[brick.size()-1].x=x;
			brick[brick.size()-1].y=y;
			brick[brick.size()-1].w=50;
			brick[brick.size()-1].h=25;
			brick[brick.size()-1].quality=3;
			x+=50;
			break;
		case '-':
			x+=50;
			break;
		default:
			break;
		}

		if(x>600)
		{
			x=150;
			y+=25;
		}
	}
}

void displayGraphics(){
	// Background
	applySurface(0,0,background,screen);

	// Paddle
	applySurface(paddle.x, paddle.y, paddle_image, screen);

	// Ball
	applySurface(ball.x, ball.y, ball_image, screen);

	// Bricks
	for(int i = 0; i < brick.size(); i++)
	{
		switch(brick[i].quality)
		{
			case 1:
				applySurface(brick[i].x, brick[i].y, red_brick, screen);
				break;
			case 2:
				applySurface(brick[i].x, brick[i].y, green_brick, screen);
				break;
			case 3:
				applySurface(brick[i].x, brick[i].y, blue_brick, screen);
				break;
		}

	}
}

void displayText()
{
	// Display Level
	level_text = TTF_RenderText_Solid(font,("Level:" + convertInt(level)).c_str(),text_color);
	applySurface(1,1,level_text, screen);

	// Display Blocks Remaining
	bricks_remaining_text = TTF_RenderText_Solid(font,("Bricks Remaining:" + convertInt(bricks_remaining)).c_str(),text_color);
	applySurface(100,1,bricks_remaining_text, screen);

	// Display Score
	current_score_text = TTF_RenderText_Solid(font,("Current Score:" + convertInt(score)).c_str(),text_color);
	applySurface(300,1,current_score_text, screen);

	// Display High Score
	std::string highscore_file = "highscore.txt";
	std::ifstream load;
	load.open(highscore_file.c_str());

	std::string shigh_score;
	load>> shigh_score;
	load.close();

	int highscore = atoi(shigh_score.c_str());

	if(score > highscore)
		highscore=score;

	std::ofstream save;
	save.open(highscore_file.c_str());
	save << highscore;

	high_score_text = TTF_RenderText_Solid(font,("High Score:" + convertInt(highscore)).c_str(),text_color);
	applySurface(450,1,high_score_text, screen);

	// Display Lives Left
	lives_left_text = TTF_RenderText_Solid(font,("Lives Left:" + convertInt(lives_left)).c_str(),text_color);
	applySurface(600,1,lives_left_text, screen);

}