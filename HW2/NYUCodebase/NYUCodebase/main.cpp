/*
CS-UY 3113 
Geena Saji
HW 2
Due October 2, 2017
PONG
*/
#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <windows.h>

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
//entity code taken from CS3113 Repo lecture slides
class Paddle{
public:
	float x;
	float y;
	float rotation;
	int textureID;
	float width;
	float height;
	float velocity = 0.7f;
	float direction_x;
	float direction_y;
	Paddle(float x_pos, float y_pos, float w, float h) : x(x_pos), y(y_pos), width(w), height(h){}

};

class Ball{
public:
	
	float x;
	float y; 
	float rotation;
	int textureID;
	float width;
	float height;
	float velocity = 1.2f;
	float direction_x;
	float direction_y;
	void reset(){
		x = 0.0f;
		y = 0.0f;
	}
	
	Ball(float x_pos, float y_pos, float w, float h) : x(x_pos), y(y_pos), width(w), height(h){}

};

//load texture function taken from CS3113 Repo lecture slides
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	
	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	stbi_image_free(image);
	return retTexture;
}

int main(int argc, char *argv[])
{	//setup
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360); 
		
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	float lastFrameTicks = 0.0f;
	float angle = 0.0f;
	//pong texture
	GLuint pongTexture = LoadTexture(RESOURCE_FOLDER"white_paddle.png"); 

	Ball pBall(0.0f,0.0f,0.5f,0.5f);
	Paddle leftPaddle(-3.275f, 0.0f, 0.5f, 1.0f);
	Paddle rightPaddle(3.275f, 0.0f, 0.5f, 1.0f);
	
	SDL_Event event;
	bool done = false;
	while (!done) {
		float ticks = (float)SDL_GetTicks()/1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		pBall.x += cos(angle)*elapsed*pBall.velocity; 
		pBall.y += sin(angle)*elapsed*pBall.velocity; 
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN){
					angle = 0.0;
				}
			}
		}

		//move paddles
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_W]){ //left paddle up
			if (leftPaddle.y +(leftPaddle.height/2) < 2.0f){
				leftPaddle.y += elapsed;
			}
		}
		if (keys[SDL_SCANCODE_S]){ // left paddle down
			if (leftPaddle.y - (leftPaddle.height / 2) > -2.0f){
				leftPaddle.y -= elapsed;
			}
		}
		if (keys[SDL_SCANCODE_UP]){ //right paddle up
			if (rightPaddle.y+(rightPaddle.height/2) < 2.0f){
				rightPaddle.y += elapsed;
			}

		}
		if (keys[SDL_SCANCODE_DOWN]){
			if (rightPaddle.y - (rightPaddle.height / 2) > -2.0f){
				rightPaddle.y -= elapsed;
			}
		}

		//All Events that can occur
		//right player wins 
		if (pBall.x < (leftPaddle.x - (leftPaddle.width / 2))){
			glClearColor(0.9f, 0.0f, 0.0f, 0.0f); // if right player wins screen turns red
			glClear(GL_COLOR_BUFFER_BIT);
			std::cout << "~~~~~~Congrats Right player wins!~~~~~~\n";
			pBall.reset();

		}
		//left player wins
		else if (pBall.x >(rightPaddle.x+(rightPaddle.width/2))){
			glClearColor(0.0f, 0.9f, 0.0f, 0.0f); // if left player wins screen turns green
			glClear(GL_COLOR_BUFFER_BIT);
			std::cout << "~~~~~~Congrats Left player wins!~~~~~~\n";
			pBall.reset();

		}
		//movement based on ball collision with right paddle
		else if (
			!((pBall.y + (pBall.height / 2)) <= (rightPaddle.y - (rightPaddle.height / 2))) && //ball's top < rightPaddle bottom
			!((pBall.y - (pBall.height / 2)) >= (rightPaddle.y + (rightPaddle.height / 2))) && //ball's bottom > rightPaddle top
			!((pBall.x + (pBall.width / 2)) <= (rightPaddle.x - (rightPaddle.width / 2)))      //ball's right < rightPaddle left 
			){
			angle += elapsed*rand()*3.14;
		}
		//movement based on ball collision with left paddle
		else if (
			!((pBall.y - (pBall.height / 2)) >= (leftPaddle.y + (leftPaddle.height / 2))) && //ball's bottom > leftPaddle top
			!((pBall.y + (pBall.height / 2)) <= (leftPaddle.y - (leftPaddle.height / 2))) &&  //ball's top < leftPaddle bottom
			!((pBall.x - (pBall.width / 2)) >= (leftPaddle.x + (leftPaddle.width / 2)))       //ball's left > leftPaddle's right
			){
			angle += elapsed*rand()*3.14;
		}
		//movement based on ball collision with screen
		else if (pBall.y+(pBall.width/2)>2.0f){
			angle += elapsed*rand()*3.14;
		}
		else if (pBall.y-(pBall.width/2) <-2.0f){
			angle += elapsed*rand()*3.14;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);

		glUseProgram(program.programID);

		modelviewMatrix.Identity();
		float textureCoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

		//create LEFT paddle
		program.SetModelviewMatrix(modelviewMatrix);
		glBindTexture(GL_TEXTURE_2D, pongTexture);
		float leftPaddleVertices[] = { -3.55f, -0.5f+leftPaddle.y, -3.0f, -0.5f+leftPaddle.y, 
			-3.0f, 0.5f+leftPaddle.y, -3.55f, -0.5f+leftPaddle.y,
			-3.0f, 0.5f+leftPaddle.y, -3.55f, 0.5f+leftPaddle.y };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPaddleVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);				
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		
		//create RIGHT paddle
		program.SetModelviewMatrix(modelviewMatrix);
		glBindTexture(GL_TEXTURE_2D, pongTexture);
		float rightPaddleVertices[] = { 3.0f, -0.5f+rightPaddle.y, 3.55f, -0.5f+rightPaddle.y,
			3.55f, 0.5f+rightPaddle.y, 3.0f, -0.5f+rightPaddle.y,
			3.55f, 0.5f+rightPaddle.y, 3.0f, 0.5f+rightPaddle.y };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPaddleVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
			
		//create BALL
		program.SetModelviewMatrix(modelviewMatrix);
		glBindTexture(GL_TEXTURE_2D, pongTexture);
		float ballVertices[] = { -0.25f + pBall.x, -0.25f + pBall.y, 0.25f+pBall.x, -0.25f+pBall.y, 0.25f + pBall.x, 0.25f + pBall.y, 
			-0.25f + pBall.x, -0.25f + pBall.y, 0.25f + pBall.x, 0.25f + pBall.y, -0.25f + pBall.x, 0.25f + pBall.y };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);



		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
