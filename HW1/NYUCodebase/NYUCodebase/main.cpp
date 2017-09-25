/*
CS-UY 3113 
Geena Saji
HW 1
Due Sept 25, 2017
Displays a screen with 3 dice. Starting from left: White -3 , Red -1, White -2
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

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
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
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("1-2-3 Dice Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	//setup
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	Matrix modelviewMatrix;
	
	//dice textures
	GLuint die1Texture = LoadTexture(RESOURCE_FOLDER"die1.png");
	GLuint die2Texture = LoadTexture(RESOURCE_FOLDER"die2.png");
	GLuint die3Texture = LoadTexture(RESOURCE_FOLDER"die3.png");

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program.programID);

		//create die1
		glBindTexture(GL_TEXTURE_2D, die1Texture);
		float die1Vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, die1Vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		
		float die1_texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, die1_texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
				
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		
		//create die2
		glBindTexture(GL_TEXTURE_2D, die2Texture);
		float die2Vertices[] = { 1.5f, -0.5f, 2.5f, -0.5f, 2.5f, 0.5f, 1.5f, -0.5f, 2.5f, 0.5f, 1.5f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, die2Vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float die2_texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, die2_texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
			
		//create die3
		glBindTexture(GL_TEXTURE_2D, die3Texture);
		float die3Vertices[] = { -2.5f, -0.5f, -1.5f, -0.5f, -1.5f, 0.5f, -2.5f, -0.5f, -1.5f, 0.5f, -2.5f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, die3Vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float die3_texCoords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, die3_texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
