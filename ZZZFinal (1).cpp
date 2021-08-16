/*
 * ZZZFinal.cpp
 *
 *  Created on: Dec 10, 2020
 *      Author: Brian
 */

//Header Inclusions
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL image loader inclusion
#include "SOIL2/SOIL2.h"

using namespace std; //Standard namespace

#define WINDOW_TITLE "CS-330 Final Project - OpenGL" //Window title Macro

//Shader program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

//Variable declaration for shader, window size initialization, buffer and array objects
GLint shaderProgram,
	  WindowWidth = 800,
	  WindowHeight = 600;
GLuint VBO,
 	   VAO,
	   texture;

GLfloat degrees = glm::radians(-45.0f); //Convertes float to degrees
GLfloat cameraSpeed = 0.0005f; //Movement speed per frame
GLfloat lastMouseX = 400, lastMouseY = 300; 				  //Locks mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; //mouse offset, yaw and pitch varialbes
GLfloat sensitivity = 0.5f; 								  //Used for mouse / camera rotation sensitivity
bool mouseDetected = true; 									  //Initially true when mouse movement is detected

//Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); 	  //Initial camera position. placed 5 units in z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); 			  //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); 	  //Temporary z unit vector
glm::vec3 front;



//Function prototypes
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);
void UKeyboard(unsigned char key, int x, int y);
void UMouseMove(int x, int y);

// Vertex Shader Source code
const GLchar * vertexShaderSource = GLSL(400,
		layout (location = 0) in vec3 position; //Vertex data from vertex attrib pointer 0
		layout (location = 1) in vec2 textureCoordinate;


		out vec2 mobileTextureCoordinate;

		// Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;


void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f); //transforms vertices to clip coordinates
	mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); //flips the texture horizontal
});


// Fragment shader source code
const GLchar * fragmentShaderSource = GLSL (400,

	in vec2 mobileTextureCoordinate;



	out vec4 gpuTexture; // Variable to pass color data to the GPU

	uniform sampler2D uTexture; // Useful when working with multiple textures

void main(){
	gpuTexture = texture(uTexture, mobileTextureCoordinate);
});


// Main program
int main(int argc, char* argv[]){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK){
				std::cout << "Failed to initialize GLEW" << std::endl;
				return -1;
			}

	UCreateShader();
	UCreateBuffers();
	UGenerateTexture();

	glUseProgram(shaderProgram); //Uses Shader program

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Sets background color to black
	glutDisplayFunc(URenderGraphics);
	glutKeyboardFunc(UKeyboard); 		  	//Detects key press
	glutPassiveMotionFunc(UMouseMove);
	glutMainLoop();

	//Destroys Buffer once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	return 0;
}

// Resizes the window
void UResizeWindow(int w, int h) {

	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

//Renders Graphics
void URenderGraphics(void) {

	glEnable(GL_DEPTH_TEST); //Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clears the screen

	glBindVertexArray(VAO); // Activate the vertex array object before rendering and transforming them

	CameraForwardZ = front; 							//Replaces camera forward vector with Radians normalized as a unit vector


	//Transforms the object
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0, 0.0f, 0.0f)); //Place the object at the center of the viewport
	model = glm::rotate(model, degrees, glm::vec3(0.0, 1.0f, 0.0f)); // Rotate the object y -45 degrees
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // Increase the object size by a scale of 2

	//Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY);

	// Creates a perspective projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);

	//Retrieves and passes transform matrices to the shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));



	glutPostRedisplay();

	glBindTexture(GL_TEXTURE_2D, texture);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 330);

	glBindVertexArray(0); // Deactivate the Vertex array object

	glutSwapBuffers(); //Flips the back buffer with the front buffer every frame. Similar to GL Flush

}

// Creates the shader program
void UCreateShader() {

	//vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); //creates the vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attaches the vertex shader to the source code
	glCompileShader(vertexShader); // Compiles the vertex shader

	// Fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Attaches the fragment shader to the source code
	glCompileShader(fragmentShader); // Compiles the fragment shader

	// Shader program
	shaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); // Attach Vertex shader to the shader program
	glAttachShader(shaderProgram, fragmentShader); // Attach fragment shader to the shader program
	glLinkProgram(shaderProgram); // Link vertex and fragment shader to shader progarm

	//Delete the vertex and fragment shader once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void UCreateBuffers() {

	GLfloat vertices[] = {

			/* Bottom base, inner square made of two
			 * triangles.
			 */
			-0.3f, -0.1f, -0.5f,     0.0f, 0.0f,  //triangle one
			 0.3f, -0.1f, -0.5f,     0.5f, 1.0f,
			 0.3f,  0.1f, -0.5f,     1.0f, 0.0f,
			 0.3f,  0.1f, -0.5f,     1.0f, 0.0f,  //triangle two
			-0.3f,  0.1f, -0.5f,     0.5f, 1.0f,
			-0.3f, -0.1f, -0.5f,     0.0f, 0.0f,

			/* Bottom base - lower outer box.
			 * Inner box is surrounded by four additional
			 * boxes, each made of two triangles.
			 */
			-0.3f, -0.1f,  -0.5f,     0.0f, 0.0f,  //triangle one
			-0.3f, -0.2f,  -0.5f,     0.5f, 1.0f,
			 0.3f, -0.1f,  -0.5f,     1.0f, 0.0f,
			 0.3f, -0.1f,  -0.5f,     0.0f, 0.0f,  //triangle two
			 0.3f, -0.2f,  -0.5f,     0.5f, 1.0f,
			-0.3f, -0.2f,  -0.5f,     1.0f, 0.0f,

			/* Bottom base - upper outer box surrounding
			 * inner square. Made of two triangles
			 */
			-0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,  //triangle one
			-0.3f,  0.2f,  -0.5f,     0.5f, 1.0f,
			 0.3f,  0.1f,  -0.5f,     1.0f, 0.0f,
			 0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,  //triangle two
			 0.3f,  0.2f,  -0.5f,     0.5f, 1.0f,
			-0.3f,  0.2f,  -0.5f,     1.0f, 0.0f,

			/* Bottom base - Right side outer box.
			 * Made of two triangles.
			 */
			 0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle one
			 0.4f,  0.1f,  -0.5f,     0.5f, 1.0f,
			 0.3f, -0.1f,  -0.5f,     1.0f, 0.0f,
			 0.3f, -0.1f,  -0.5f,     0.0f, 0.0f,  //triangle two
			 0.4f, -0.1f,  -0.5f,     0.5f, 1.0f,
			 0.4f,  0.1f,  -0.5f,     1.0f, 0.0f,

			 /* Bottom base - Left side outer box.
			  * Made of two triangles
			  */
			-0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle one
			-0.4f,  0.1f,  -0.5f,     0.5f, 1.0f,
			-0.3f, -0.1f,  -0.5f,     1.0f, 0.0f,
			-0.3f, -0.1f,  -0.5f,     0.0f, 0.0f,  //triangle two
			-0.4f, -0.1f,  -0.5f,     0.5f, 1.0f,
			-0.4f,  0.1f,  -0.5f,     1.0f, 0.0f,

			/* Bottom base - Left corner made of three
			 * triangles to give the rounded edge.
			 */
			-0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle one - upper left
			-0.4f,  0.1f,  -0.5f,     0.5f, 1.0f,
			-0.39f, 0.15f, -0.5f,     1.0f, 0.0f,
			-0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle two - upper left
			-0.39f, 0.15f, -0.5f,     0.5f, 1.0f,
			-0.35f, 0.19f, -0.5f,     1.0f, 0.0f,
			-0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle three - upper left side
			-0.35f, 0.19f, -0.5f,     0.5f, 1.0f,
			-0.3f,  0.2f,  -0.5f,     1.0f, 0.0f,

			/* Bottom base - Right corner made of three
			 * triangles to give the rounded edge
			 */
			 0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle one - upper right
			 0.4f,  0.1f,  -0.5f,     0.5f, 1.0f,
			 0.39f, 0.15f, -0.5f,     1.0f, 0.0f,
			 0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle two - upper right side
			 0.39f, 0.15f, -0.5f,     0.5f, 1.0f,
			 0.35f, 0.19f, -0.5f,     1.0f, 0.0f,
			 0.3f,  0.1f,  -0.5f,     0.0f, 0.0f,   //triangle three -  upper right side
			 0.35f, 0.19f, -0.5f,     0.5f, 1.0f,
			 0.3f,  0.2f,  -0.5f,     1.0f, 0.0f,

			 /* Bottom base - Right lower corner made of
			  * three triangles to give the rounded
			  * edge.
			  */
			 0.3f,  -0.1f,  -0.5f,    0.0f, 0.0f,   //triangle one - lower right side
			 0.4f,  -0.1f,  -0.5f,    0.5f, 1.0f,
			 0.39f, -0.15f, -0.5f,    1.0f, 0.0f,
			 0.3f,  -0.1f,  -0.5f,    0.0f, 0.0f,   //triangle two - lower right side
			 0.39f, -0.15f, -0.5f,    0.5f, 1.0f,
			 0.35f, -0.19f, -0.5f,    1.0f, 0.0f,
			 0.3f,  -0.1f,  -0.5f,    0.0f, 0.0f,   //triangle three - lower right side
			 0.35f, -0.19f, -0.5f,    0.5f, 1.0f,
			 0.3f,  -0.2f,  -0.5f,    1.0f, 0.0f,

			 /* Bottom base - Left lower corner made of
			  * three triangles to give the rounded edge
			  */
			-0.3f,  -0.1f,  -0.5f,    0.0f, 0.0f,   //triangle one -  lower left side
			-0.4f,  -0.1f,  -0.5f,    0.5f, 1.0f,
			-0.39f, -0.15f, -0.5f,    1.0f, 0.0f,
			-0.3f,  -0.1f,  -0.5f,    0.0f, 0.0f,   //triangle two - lower left side
			-0.39f, -0.15f, -0.5f,    0.5f, 1.0f,
			-0.35f, -0.19f, -0.5f,    1.0f, 0.0f,
			-0.3f,  -0.1f,  -0.5f,    0.0f, 0.0f,   //triangle three - lower left side
			-0.35f, -0.19f, -0.5f,    0.5f, 1.0f,
			-0.3f,  -0.2f,  -0.5f,    1.0f, 0.0f,
//////////////////  END OF BOTTOM BASE   ////////////////////////////////////////////////////

			/* Top base - This section mirrors bottom base
			 * but z vertices are moved 1.0f in the
			 * positive direction.
			 * 0.5 positive from the center.
			 */
			-0.3f, -0.1f, 0.5f,     0.0f, 0.0f,  //triangle one
			 0.3f, -0.1f, 0.5f,     0.5f, 1.0f,
			 0.3f,  0.1f, 0.5f,     1.0f, 0.0f,
			 0.3f,  0.1f, 0.5f,     0.0f, 0.0f,  //triangle two
			-0.3f,  0.1f, 0.5f,     0.5f, 1.0f,
			-0.3f, -0.1f, 0.5f,     1.0f, 0.0f,

			/* Top base - outer box lower section
			 */
			-0.3f, -0.1f, 0.5f,     0.0f, 0.0f,  //triangle one
			-0.3f, -0.2f, 0.5f,     0.5f, 1.0f,
			 0.3f, -0.1f, 0.5f,     1.0f, 0.0f,
			 0.3f, -0.1f, 0.5f,     0.0f, 0.0f,  //triangle two
			 0.3f, -0.2f, 0.5f,     0.5f, 1.0f,
			-0.3f, -0.2f, 0.5f,     1.0f, 0.0f,

			/* Top base - outer box upper section
			 */
			-0.3f,  0.1f, 0.5f,     0.0f, 0.0f,  //triangle one
			-0.3f,  0.2f, 0.5f,     0.5f, 1.0f,
			 0.3f,  0.1f, 0.5f,     1.0f, 0.0f,
			 0.3f,  0.1f, 0.5f,     0.0f, 0.0f,  //triangle two
			 0.3f,  0.2f, 0.5f,     0.5f, 1.0f,
			-0.3f,  0.2f, 0.5f,     1.0f, 0.0f,

			/* Top base - outer box right side
			 */
			 0.3f,  0.1f, 0.5f,     0.0f, 0.0f,   //triangle one
			 0.4f,  0.1f, 0.5f,     0.5f, 1.0f,
			 0.3f, -0.1f, 0.5f,     1.0f, 0.0f,
			 0.3f, -0.1f, 0.5f,     0.0f, 0.0f,  //triangle two
			 0.4f, -0.1f, 0.5f,     0.5f, 1.0f,
			 0.4f,  0.1f, 0.5f,     1.0f, 0.0f,

			 /* Top base - outer box left side
			  */
			-0.3f,  0.1f, 0.5f,     0.0f, 0.0f,   //triangle one
			-0.4f,  0.1f, 0.5f,     0.5f, 1.0f,
			-0.3f, -0.1f, 0.5f,     1.0f, 0.0f,
			-0.3f, -0.1f, 0.5f,     0.0f, 0.0f,  //triangle two
			-0.4f, -0.1f, 0.5f,     0.5f, 1.0f,
			-0.4f,  0.1f, 0.5f,     1.0f, 0.0f,

			/* Top base - Left upper corner made of three
			 * triangles to give the rounded edge
			 */
			-0.3f,  0.1f,  0.5f,    0.0f, 0.0f,   // triangle one - upper left side
			-0.4f,  0.1f,  0.5f,    0.5f, 1.0f,
			-0.39f, 0.15f, 0.5f,    1.0f, 0.0f,
			-0.3f,  0.1f,  0.5f,    0.0f, 0.0f,   //triangle two - upper left side
			-0.39f, 0.15f, 0.5f,    0.5f, 1.0f,
			-0.35f, 0.19f, 0.5f,    1.0f, 0.0f,
			-0.3f,  0.1f,  0.5f,    0.0f, 0.0f,   //triangle three - upper left side
			-0.35f, 0.19f, 0.5f,    0.5f, 1.0f,
			-0.3f,  0.2f,  0.5f,    1.0f, 0.0f,

			/* Top base - Right upper corner made of
			 * three triangles to give the rounded edge
			 */
			 0.3f,  0.1f,  0.5f,    0.0f, 0.0f,   //triangle one - upper right side
			 0.4f,  0.1f,  0.5f,    0.5f, 1.0f,
			 0.39f, 0.15f, 0.5f,    1.0f, 0.0f,
			 0.3f,  0.1f,  0.5f,    0.0f, 0.0f,   //triangle two - upper right side
			 0.39f, 0.15f, 0.5f,    0.5f, 1.0f,
			 0.35f, 0.19f, 0.5f,    1.0f, 0.0f,
			 0.3f,  0.1f,  0.5f,    0.0f, 0.0f,   //triangle three - upper right side
			 0.35f, 0.19f, 0.5f,    0.5f, 1.0f,
			 0.3f,  0.2f,  0.5f,    1.0f, 0.0f,

			 /* Top base - Right lower corner made of
			  * three triangles to give the rounded edge
			  */
			 0.3f,  -0.1f,  0.5f,    0.0f, 0.0f,   //triangle one - lower right side
			 0.4f,  -0.1f,  0.5f,    0.5f, 1.0f,
			 0.39f, -0.15f, 0.5f,    1.0f, 0.0f,
			 0.3f,  -0.1f,  0.5f,    0.0f, 0.0f,   //triangle two - lower right side
			 0.39f, -0.15f, 0.5f,    0.5f, 1.0f,
			 0.35f, -0.19f, 0.5f,    1.0f, 0.0f,
			 0.3f,  -0.1f,  0.5f,    0.0f, 0.0f,   //triangle three - lower right side
			 0.35f, -0.19f, 0.5f,    0.5f, 1.0f,
			 0.3f,  -0.2f,  0.5f,    1.0f, 0.0f,

			 /* Top base - Left lower corner made of
			  * three triangles to give the rounded edge
			  */
			-0.3f,  -0.1f,  0.5f,    0.0f, 0.0f,   //triangle one - lower left side
		    -0.4f,  -0.1f,  0.5f,    0.5f, 1.0f,
		    -0.39f, -0.15f, 0.5f,    1.0f, 0.0f,
		    -0.3f,  -0.1f,  0.5f,    0.0f, 0.0f,   //triangle two - lower left side
		    -0.39f, -0.15f, 0.5f,    0.5f, 1.0f,
	    	-0.35f, -0.19f, 0.5f,    1.0f, 0.0f,
			-0.3f,  -0.1f,  0.5f,    0.0f, 0.0f,   //triangle three - lower left side
			-0.35f, -0.19f, 0.5f,    0.5f, 1.0f,
			-0.3f,  -0.2f,  0.5f,    1.0f, 0.0f,
//////////////////////// END OF TOP BASE //////////////////////////////////////

			/* Connecting Bottom base with Top base using
			 * two triangles for each side to create a side wall
			 * connecting both bases. This will make up the body
			 * of the bottle.
			 */
					//Sides for Front and Back
			-0.3f, -0.2,  -0.5f,     0.0f, 0.0f,  //triangle one
			-0.3f, -0.2f,  0.5f,     0.5f, 1.0f,
			 0.3f, -0.2f,  0.5f, 	 1.0f, 0.0f,
			 0.3f, -0.2,   0.5f,     0.0f, 0.0f,  //triangle two
		     0.3f, -0.2f, -0.5f,     0.5f, 1.0f,
			-0.3f, -0.2f, -0.5f, 	 1.0f, 0.0f,
			-0.3f,  0.2,  -0.5f,     0.0f, 0.0f,  //triangle one
			-0.3f,  0.2f,  0.5f,     0.5f, 1.0f,
			 0.3f,  0.2f,  0.5f, 	 1.0f, 0.0f,
			 0.3f,  0.2,   0.5f,     0.0f, 0.0f,  //triangle two
			 0.3f,  0.2f, -0.5f,     0.5f, 1.0f,
			-0.3f,  0.2f, -0.5f, 	 1.0f, 0.0f,

				 //Sides for Right and Left sides
			-0.4f,  0.1f,  0.5f,     0.0f, 0.0f,  //triangle one
			-0.4f, -0.1f,  0.5f,     0.5f, 1.0f,
			-0.4f, -0.1f, -0.5f,     1.0f, 0.0f,
			-0.4f, -0.1f, -0.5f,     0.0f, 0.0f,  //triangle two
			-0.4f,  0.1f, -0.5f,     0.5f, 1.0f,
			-0.4f,  0.1f,  0.5f,     1.0f, 0.0f,
			 0.4f,  0.1f,  0.5f,     0.0f, 0.0f,  //triangle one
			 0.4f, -0.1f,  0.5f,     0.5f, 1.0f,
			 0.4f, -0.1f, -0.5f,     1.0f, 0.0f,
			 0.4f, -0.1f, -0.5f,     0.0f, 0.0f,  //triangle two
			 0.4f,  0.1f, -0.5f,     0.5f, 1.0f,
			 0.4f,  0.1f,  0.5f,     1.0f, 0.0f,

			   //Corner side wall - Upper Left side
			-0.4f,  0.1f, 0.5f,      0.0f, 0.0f,  // upper left - triangle one - side one
			-0.39f, 0.15f, 0.5f,     0.5f, 1.0f,
			-0.39f, 0.15f, -0.5f,    1.0f, 0.0f,
			-0.4f,  0.1f, -0.5f,     0.0f, 0.0f,  // upper left - triangle one - side two
			-0.39f, 0.15f, -0.5f,    0.5f, 1.0f,
			-0.4f,  0.1f, 0.5f,      1.0f, 0.0f,
			-0.39f, 0.15f, -0.5f,    0.0f, 0.0f,  // upper left - triangle two - side one
			-0.35f, 0.19f, -0.5f,    0.5f, 1.0f,
			-0.35f, 0.19f, 0.5f,     1.0f, 0.0f,
			-0.39f, 0.15f, 0.5f,     0.0f, 0.0f,  // upper left - triangle two - side two
			-0.35f, 0.19f, 0.5f,     0.5f, 1.0f,
			-0.39f, 0.15f, -0.5f,    1.0f, 0.0f,
			-0.35f, 0.19f, -0.5f,    0.0f, 0.0f,  // upper left - triangle three - side one
			-0.3f,  0.2f,  -0.5f,    0.5f, 1.0f,
			-0.3f, 0.2f,  0.5f,      1.0f, 0.0f,
			-0.35f, 0.19f,  0.5f,    0.0f, 0.0f,  // upper left - triangle three - side two
			-0.3f,  0.2f,   0.5f,    0.5f, 1.0f,
			-0.35f, 0.19f, -0.5f,    1.0f, 0.0f,

			// Corner side wall - Upper Right side
			 0.4f,  0.1f,   0.5f,    0.0f, 0.0f,  // upper Right - triangle one - side one
			 0.39f, 0.15f,  0.5f,    0.5f, 1.0f,
			 0.39f, 0.15f, -0.5f,    1.0f, 0.0f,
			 0.4f,  0.1f,  -0.5f,    0.0f, 0.0f,  // upper Right - triangle one - side two
			 0.39f, 0.15f, -0.5f,    0.5f, 1.0f,
			 0.4f,  0.1f,   0.5f,    1.0f, 0.0f,
			 0.39f, 0.15f, -0.5f,    0.0f, 0.0f,  // upper Right  - triangle two - side one
			 0.35f, 0.19f, -0.5f,    0.5f, 1.0f,
			 0.35f, 0.19f,  0.5f,    1.0f, 0.0f,
			 0.39f, 0.15f,  0.5f,    0.0f, 0.0f,  // upper Right  - triangle two - side two
			 0.35f, 0.19f,  0.5f,    0.5f, 1.0f,
			 0.39f, 0.15f, -0.5f,    1.0f, 0.0f,
			 0.35f, 0.19f, -0.5f,    0.0f, 0.0f,  // upper Right - triangle three - side one
			 0.3f,  0.2f,  -0.5f,    0.5f, 1.0f,
			 0.3f,  0.2f,   0.5f,    1.0f, 0.0f,
			 0.35f, 0.19f,  0.5f,    0.0f, 0.0f,  // upper Right - triangle three - side two
			 0.3f,  0.2f,   0.5f,    0.5f, 1.0f,
			 0.35f, 0.19f, -0.5f,    1.0f, 0.0f,

			// Corner side wall - Lower right side
			 0.4f,  -0.1f,   0.5f,   0.0f, 0.0f,  // lower Right - triangle one - side one
			 0.39f, -0.15f,  0.5f,   0.5f, 1.0f,
			 0.39f, -0.15f, -0.5f,   1.0f, 0.0f,
			 0.4f,  -0.1f,  -0.5f,   0.0f, 0.0f,  // lower Right  - triangle one - side two
			 0.39f, -0.15f, -0.5f,   0.5f, 1.0f,
			 0.4f,  -0.1f,   0.5f,   1.0f, 0.0f,
			 0.39f, -0.15f, -0.5f,   0.0f, 0.0f,  // lower Right - triangle two - side one
			 0.35f, -0.19f, -0.5f,   0.5f, 1.0f,
			 0.35f, -0.19f,  0.5f,   1.0f, 0.0f,
			 0.39f, -0.15f,  0.5f,   0.0f, 0.0f,  // lower Right - triangle two - side two
			 0.35f, -0.19f,  0.5f,   0.5f, 1.0f,
			 0.39f, -0.15f, -0.5f,   1.0f, 0.0f,
			 0.35f, -0.19f, -0.5f,   0.0f, 0.0f,  // lower Right - triangle three - side one
			 0.3f,  -0.2f,  -0.5f,   0.5f, 1.0f,
			 0.3f,  -0.2f,   0.5f,   1.0f, 0.0f,
			 0.35f, -0.19f,  0.5f,   0.0f, 0.0f,  // lower Right - triangle three - side two
			 0.3f,  -0.2f,   0.5f,   0.5f, 1.0f,
			 0.35f, -0.19f, -0.5f,   1.0f, 0.0f,

				// Corner wall - Lower left side
			-0.4f,  -0.1f,   0.5f,   0.0f, 0.0f,  // lower Left - triangle one - side one
			-0.39f, -0.15f,  0.5f,   0.5f, 1.0f,
			-0.39f, -0.15f, -0.5f,   1.0f, 0.0f,
			-0.4f,  -0.1f,  -0.5f,   0.0f, 0.0f,  // lower Left - triangle one - side two
			-0.39f, -0.15f, -0.5f,   0.5f, 1.0f,
			-0.4f,  -0.1f,   0.5f,   1.0f, 0.0f,
			-0.39f, -0.15f, -0.5f,   0.0f, 0.0f,  // lower Left - triangle two - side one
			-0.35f, -0.19f, -0.5f,   0.5f, 1.0f,
			-0.35f, -0.19f,  0.5f,   1.0f, 0.0f,
			-0.39f, -0.15f,  0.5f,   0.0f, 0.0f,  // lower Left - triangle two - side two
			-0.35f, -0.19f,  0.5f,   0.5f, 1.0f,
			-0.39f, -0.15f, -0.5f,   1.0f, 0.0f,
			-0.35f, -0.19f, -0.5f,   0.0f, 0.0f,  // lower Left - triangle three - side one
			-0.3f,  -0.2f,  -0.5f,   0.5f, 1.0f,
			-0.3f,  -0.2f,   0.5f,   1.0f, 0.0f,
			-0.35f, -0.19f,  0.5f,   0.0f, 0.0f,  // lower Left - triangle three - side two
			-0.3f,  -0.2f,   0.5f,   0.5f, 1.0f,
			-0.35f, -0.19f, -0.5f,   1.0f, 0.0f,
///////////////////// END OF BODY SIDE WALLS ///////////////////////////////////////////////////////

			/* Cap is made of both a base and top just
			 * like the body and placed just above the body.
			 * This offset is to create a slope from the body
			 * to the cap
			 * Cap top is just a little smaller than
			 * the Cap base.
			 */
			        //Cap Bottom base (unseen when object is complete)
			-0.2f,  0.1f, 0.6f,       0.0f, 0.0f,  //triangle one
			-0.2f, -0.1f, 0.6f,       0.5f, 1.0f,
			 0.2f,  0.1f, 0.6f,       1.0f, 0.0f,
			 0.2f,  0.1f, 0.6f,       0.0f, 0.0f,  //triangle two
			 0.2f, -0.1f, 0.6f,       0.5f, 1.0f,
			-0.2f, -0.1f, 0.6f,       1.0f, 0.0f,

			// Slope from top of body to bottom of cap
						/* Front and Back slopes
						 * made of three triangles
						 */
			-0.3f, -0.2f, 0.5f,       0.0f, 0.0f,  // triangle one
			 0.0f, -0.2f, 0.5f,       0.5f, 1.0f,
			-0.2f, -0.1f, 0.6f,       1.0f, 0.0f,
			 0.3f, -0.2f, 0.5f,       0.0f, 0.0f,  // triangle two
			 0.0f, -0.2f, 0.5f,       0.5f, 1.0f,
			 0.2f, -0.1f, 0.6f,       1.0f, 0.0f,
			-0.2f, -0.1f, 0.6f,       0.0f, 0.0f,  // triangle three
			 0.2f, -0.1f, 0.6f,       0.5f, 1.0f,
			 0.0f, -0.2f, 0.5f,       1.0f, 0.0f,
			-0.3f,  0.2f, 0.5f,       0.0f, 0.0f,  // triangle one
			 0.0f,  0.2f, 0.5f,       0.5f, 1.0f,
			-0.2f,  0.1f, 0.6f,       1.0f, 0.0f,
			 0.3f,  0.2f, 0.5f,       0.0f, 0.0f,  // triangle two
			 0.0f,  0.2f, 0.5f,       0.5f, 1.0f,
			 0.2f,  0.1f, 0.6f,       1.0f, 0.0f,
			-0.2f,  0.1f, 0.6f,       0.0f, 0.0f,  // triangle three
			 0.2f,  0.1f, 0.6f,       0.5f, 1.0f,
			 0.0f,  0.2f, 0.5f,       1.0f, 0.0f,

			     /* Side slopes from body to cap.
			      * Right and Left side. Made from
			      * two triangles each.
			      */
			-0.2f,  0.1f, 0.6f,       0.0f, 0.0f,  // triangle one
			-0.2f, -0.1f, 0.6f,       0.5f, 1.0f,
			-0.4f,  0.1f, 0.5f,       1.0f, 0.0f,
			-0.2f, -0.1f, 0.6f,       0.0f, 0.0f,  // triangle two
			-0.4f, -0.1f, 0.5f,       0.5f, 1.0f,
			-0.4f,  0.1f,  0.5f,      1.0f, 0.0f,
			 0.2f,  0.1f, 0.6f,       0.0f, 0.0f,  // triangle one
			 0.2f, -0.1f, 0.6f,       0.5f, 1.0f,
			 0.4f,  0.1f, 0.5f,       1.0f, 0.0f,
			 0.2f, -0.1f, 0.6f,       0.0f, 0.0f,  // triangle two
			 0.4f, -0.1f, 0.5f,       0.5f, 1.0f,
			 0.4f,  0.1f,  0.5f,      1.0f, 0.0f,

			 	 /* Corner slopes connecting the body
			 	  * with the cap. Each corner has three
			 	  * triangles. Starting with upper left,
			 	  * upper right, lower right, lower left
			 	  */
			-0.2f,  0.1f,   0.6f,     0.0f, 0.0f,  // upper left one
			-0.4f,  0.1f,   0.5f,     0.5f, 1.0f,
			-0.39f, 0.15f,  0.5f,     1.0f, 0.0f,
			-0.2f,  0.1f,   0.6f,     0.0f, 0.0f,  // upper left two
			-0.39f, 0.15f,  0.5f,     0.5f, 1.0f,
			-0.35f, 0.19f,  0.5f,     1.0f, 0.0f,
			-0.2f,  0.1f,   0.6f,     0.0f, 0.0f,  // upper left three
			-0.35f, 0.19f,  0.5f,     0.5f, 1.0f,
			-0.3f,  0.2f,   0.5f,     1.0f, 0.0f,

			 0.2f,  0.1f,   0.6f,     0.0f, 0.0f,  // upper right one
			 0.4f,  0.1f,   0.5f,     0.5f, 1.0f,
			 0.39f, 0.15f,  0.5f,     1.0f, 0.0f,
			 0.2f,  0.1f,   0.6f,     0.0f, 0.0f,  // upper right two
			 0.39f, 0.15f,  0.5f,     0.5f, 1.0f,
			 0.35f, 0.19f,  0.5f,     1.0f, 0.0f,
			 0.2f,  0.1f,   0.6f,     0.0f, 0.0f,  // upper right three
			 0.35f, 0.19f,  0.5f,     0.5f, 1.0f,
			 0.3f,  0.2f,   0.5f,     1.0f, 0.0f,

			 0.2f,  -0.1f,   0.6f,    0.0f, 0.0f,  // lower right one
			 0.4f,  -0.1f,   0.5f,    0.5f, 1.0f,
			 0.39f, -0.15f,  0.5f,    1.0f, 0.0f,
			 0.2f,  -0.1f,   0.6f,    0.0f, 0.0f,  // lower right two
			 0.39f, -0.15f,  0.5f,    0.5f, 1.0f,
			 0.35f, -0.19f,  0.5f,    1.0f, 0.0f,
			 0.2f,  -0.1f,   0.6f,    0.0f, 0.0f,  // lower right three
			 0.35f, -0.19f,  0.5f,    0.5f, 1.0f,
			 0.3f,  -0.2f,   0.5f,    1.0f, 0.0f,

			-0.2f,  -0.1f,   0.6f,    0.0f, 0.0f,  // lower left one
			-0.4f,  -0.1f,   0.5f,    0.5f, 1.0f,
			-0.39f, -0.15f,  0.5f,    1.0f, 0.0f,
			-0.2f,  -0.1f,   0.6f,    0.0f, 0.0f,  // lower left two
			-0.39f, -0.15f,  0.5f,    0.5f, 1.0f,
			-0.35f, -0.19f,  0.5f,    1.0f, 0.0f,
			-0.2f,  -0.1f,   0.6f,    0.0f, 0.0f,  // lower left three
			-0.35f, -0.19f,  0.5f,    0.5f, 1.0f,
			-0.3f,  -0.2f,   0.5f,    1.0f, 0.0f,

				/* Top of bottle (cap).
				 * Made of two triangles to create a
				 * square
				 */
			-0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle one
			-0.17f, -0.1f, 0.8f,      0.5f, 1.0f,
			 0.17f,  0.1f, 0.8f,      1.0f, 0.0f,
			 0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle two
			 0.17f, -0.1f, 0.8f,      0.5f, 1.0f,
			-0.17f, -0.1f, 0.8f,      1.0f, 0.0f,

				/* Sides for connecting the top of the
				 * cap with the bottom of the cap.
				 * Four sides, each created with two
				 * triangles.
				 */
					// Front and Back
			-0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle one
			 0.17f,  0.1f, 0.8f,      0.5f, 1.0f,
			 0.2f,   0.1f, 0.6f,      1.0f, 0.0f,
			-0.2f,   0.1f, 0.6f,      0.0f, 0.0f,  // triangle two
			 0.2f,   0.1f, 0.6f,      0.5f, 1.0f,
			-0.17f,  0.1f, 0.8f,      1.0f, 0.0f,
			-0.17f, -0.1f, 0.8f,      0.0f, 0.0f,  // triangle one
			 0.17f, -0.1f, 0.8f,      0.5f, 1.0f,
			 0.2f,  -0.1f, 0.6f,      1.0f, 0.0f,
			-0.2f,  -0.1f, 0.6f,      0.0f, 0.0f,  // triangle two
			 0.2f,  -0.1f, 0.6f,      0.5f, 1.0f,
			-0.17f, -0.1f, 0.8f,      1.0f, 0.0f,

			 	 	 // Right and Left side of cap
			-0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle one
			-0.17f, -0.1f, 0.8f,      0.5f, 1.0f,
			-0.2f,  -0.1f, 0.6f,      1.0f, 0.0f,
			-0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle two
			-0.2f,   0.1f, 0.6f,      0.5f, 1.0f,
			-0.2f,  -0.1f, 0.6f,      1.0f, 0.0f,
			 0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle one
			 0.17f, -0.1f, 0.8f,      0.5f, 1.0f,
			 0.2f,  -0.1f, 0.6f,      1.0f, 0.0f,
			 0.17f,  0.1f, 0.8f,      0.0f, 0.0f,  // triangle two
			 0.2f,   0.1f, 0.6f,      0.5f, 1.0f,
			 0.2f,  -0.1f, 0.6f,      1.0f, 0.0f,

	};

	//Generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers.
	glBindVertexArray(VAO);

	//Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //copy vertices to vbo

	//Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); //Enables vertex attribute

	//Set attribute pointer 1 to hold Color data
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); //Enables vertex attribute

	glBindVertexArray(0); //Deactivates the VAO which is good practice
}


//Generate and load the texture
void UGenerateTexture() {

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("metal111.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
}

//Implements the UMouseMove
void UMouseMove(int x, int y) {

	//Immediately replaces center locked coordinates with new mouse coordinates
	if(mouseDetected) {
		lastMouseX = x;
		lastMouseY = y;
		mouseDetected = false;
	}

	//Gets the direction the mouse was moved in x and y
	mouseXOffset = x - lastMouseX;
	mouseYOffset = lastMouseY - y; //Inverted Y

	//Updates with new mouse coordinates
	lastMouseX = x;
	lastMouseY = y;

	//Applies sensitivity to mouse direction
	mouseXOffset *= sensitivity;
	mouseYOffset *= sensitivity;

	//Accumulates the yaw and pitch variables
	yaw += mouseXOffset;
	pitch += mouseYOffset;

	//Maintains a 90 degree pitch for gimbal lock
	if(pitch > 89.0f)
		pitch = 89.0f;

	if (pitch <-89.9f)
		pitch = -89.9f;

	//Converts mouse coordinates /degrees into Radians, then to vectors
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
}

//Implements the UKeyboard function
void UKeyboard(unsigned char key, GLint x, GLint y) {


	}
























