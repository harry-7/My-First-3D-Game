#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>
#include <unistd.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <FTGL/ftgl.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <pthread.h>
#include <unistd.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <irrKlang.h>



#pragma comment(lib, "irrKlang.lib")

#define out1(x)cout<<#x<<" is "<<x<<endl
#define out2(x,y)cout<<#x<<" is "<<x<<" | "<<#y <<" is "<<y<<endl
#define out3(x,y,z)cout<<#x<<" is "<<x<<" | "<<#y<<" is "<<y<<" | "<<#z<<" is "<<z<<endl;
#define out4(a,b,c,d)cout<<#a<<" is "<<a<<" | "<<#b<<"  is "<<b<<" | "<<#c<<" is "<<c<<" | "<<#d<<" is "<<d<<endl;


#define XMAX 290
#define XMIN -220
#define ZMAX 390
#define ZMIN -140
#define BITS 8

#define YMAX 260
#define YMIN -260


using namespace std;
using namespace irrklang;
struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;


typedef struct COLOR{
	float r;
	float g;
	float b;
}COLOR;

typedef struct object{
	float x;
	float y;
	float z;
	VAO* obj;
	float l;
	float b;
	float h;
	float ol;
	float ob;
	float oh;
	float gravity;
	float angle ;
	float scalevalue;
	float delta;
	float val;

}object;

vector<object> floors[10];
map<string,object> playerobj;
vector<object> obstacles,goalobj,coins;
int to_draw = 1;
int injump = 0;
int gamest = 0;
float sfac = 0;
int took = 0;
bool mute  = 0;
int lives = 3;
float sp_fac = 0;
int game_end = 0;
float score = 0;
float ptime;
float max_time;
int levels = 0;
float pscore = 0;
bool in_pause = 0;
float paused_time;
ISoundEngine* sfxengine = createIrrKlangDevice();
ISoundEngine* musicengine = createIrrKlangDevice();

int GameMap[10][10]={
	1,1,1,1,1,3,1,2,1,1,
	1,1,1,2,1,1,1,5,1,1,
	1,2,5,1,1,5,1,3,1,1,
	1,1,1,2,2,2,1,1,1,1,
	1,5,1,2,1,2,1,3,1,1,
	1,1,1,2,4,2,1,1,1,1,
	1,2,1,2,2,2,1,2,1,1,
	1,5,1,1,1,1,1,1,1,1,
	1,1,1,2,1,1,5,3,1,1,
	1,3,1,1,1,3,1,1,1,1,
};


/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path,  std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream,  Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path,  std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream,  Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n",  vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID,  1,  &VertexSourcePointer ,  NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID,  GL_COMPILE_STATUS,  &Result);
	glGetShaderiv(VertexShaderID,  GL_INFO_LOG_LENGTH,  &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID,  InfoLogLength,  NULL,  &VertexShaderErrorMessage[0]);
	fprintf(stdout,  "%s\n",  &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n",  fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID,  1,  &FragmentSourcePointer ,  NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID,  GL_COMPILE_STATUS,  &Result);
	glGetShaderiv(FragmentShaderID,  GL_INFO_LOG_LENGTH,  &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID,  InfoLogLength,  NULL,  &FragmentShaderErrorMessage[0]);
	fprintf(stdout,  "%s\n",  &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout,  "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID,  VertexShaderID);
	glAttachShader(ProgramID,  FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID,  GL_LINK_STATUS,  &Result);
	glGetProgramiv(ProgramID,  GL_INFO_LOG_LENGTH,  &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength,  int(1)) );
	glGetProgramInfoLog(ProgramID,  InfoLogLength,  NULL,  &ProgramErrorMessage[0]);
	fprintf(stdout,  "%s\n",  &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error,  const char* description)
{
	fprintf(stderr,  "Error: %s\n",  description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	musicengine->drop();
	sfxengine->drop();
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			2,                  // attribute 2. Textures
			2,                  // size (s,t)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	if(vao == NULL)return;
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
	if(vao == NULL)return;
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}


/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float angle = 0;
int rotst = 0;
int view = 0;
float xspeed = 0, zspeed = 0;
float rot = 0;
float val = 1;
float val1 = 1;
float prot = 0;
float delta = 0;
float gravity = 1;
float y_vel = 0;
char disp_str[50];

void play_sfx(int fl){
	if(mute == 1)return ;
	if(fl == 1){
		//jump
		sfxengine->play2D("./sfx/jump.wav",false);
	}
	else if (fl == 2){
		sfxengine->play2D("./sfx/fall.wav",false);
	}
	else if (fl == 3){
		ISound *music = sfxengine->play2D("./music/win.mp3");		
		return ;
	}
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{

	if (action == GLFW_RELEASE) {
		if(in_pause == 1){
			if(key == GLFW_KEY_P)in_pause  = 0;
			//cout << "Before " << ptime << endl;
			ptime+=(glfwGetTime() - paused_time);
			//cout << ptime << endl;
			return;
		}

		switch (key) {
			case GLFW_KEY_M:
				mute = !mute;
				break;
			case GLFW_KEY_I:
					sp_fac+=0.1;
					sp_fac = min(sp_fac,1.5f);
				break;
			case GLFW_KEY_D:
				sp_fac -= 0.1;
				sp_fac = max(sp_fac,-0.5f);
			case GLFW_KEY_LEFT:
				zspeed = 0;
				break;
			case GLFW_KEY_RIGHT:
				zspeed = 0;
				break;
			case GLFW_KEY_DOWN:
				xspeed = 0;
				val1 = 1;
				break;
			case GLFW_KEY_UP:
				xspeed = 0;
				val1 = -1;
				break;
			case GLFW_KEY_SPACE:
				if(injump == 0){
					y_vel = 20;
					injump = 1;
					play_sfx(1);
				}
				break;
			case GLFW_KEY_V:
				view = 1;
				break;
			case GLFW_KEY_H:
				view = 2;
				break;
			case GLFW_KEY_O:
				view = 0;
				break;
			case GLFW_KEY_F:
				view = 4;
				break;
			case GLFW_KEY_P:
				in_pause = !in_pause;
				paused_time = glfwGetTime();
				break;
			case GLFW_KEY_R:
				to_draw = 2;
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_UP:
				xspeed = -2-sp_fac;
				break;
			case GLFW_KEY_DOWN:
				xspeed = 2 + sp_fac;
				break;
			case GLFW_KEY_LEFT:
				zspeed = 4;
				break;
			case GLFW_KEY_RIGHT:
				zspeed = -4;
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				rotst = 0;
			else
				rotst = 1;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				rotst = 0;
			}
			else
				rotst = -1;
			break;
		default:
			break;
	}
}


void mousescroll(GLFWwindow* window, double xoffset, double yoffset){
	if (yoffset==-1) { 
		sfac+=0.1;
		sfac = min(sfac,1.0f);
	}
	else if(yoffset==1){
		sfac-=0.1;
		sfac = max(sfac,-0.4f);
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;

	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 0.9f+sfac;

	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.01f, 9000.0f);

	//Matrices.projection = glm::ortho(-600.0f, 600.0f, -300.0f, 300.0f, -5000.0f, 5000.0f);
}


struct VAO*  generate_cube(float l, float b, float h,COLOR L,COLOR R,COLOR T,GLenum fill_mode=GL_FILL){
	GLfloat a[]={

		/* Rectangle 1 */

		-l, -b, -h, 
		-l, -b, h, 
		l, -b, h, 
		l, -b, h, 
		l, -b, -h, 
		-l, -b, -h, 

		/* Rectangle 2 */

		l, b, -h, 
		l, b, h, 
		l, -b, h, 
		l, -b, h, 
		l, -b, -h, 
		l, b, -h, 

		/* Rectangle 3 */

		-l, b, -h, 
		-l, b, h, 
		l, b, h, 
		l, b, h, 
		l, b, -h, 
		-l, b, -h, 		

		/* Rectangle 4 */

		-l, b, -h, 
		-l, b, h, 
		-l, -b, h, 
		-l, -b, h, 
		-l, -b, -h, 
		-l, b, -h, 

		/* Rectangle 5 */

		-l, -b, -h, 
		-l, b, -h, 
		l, b, -h, 
		l, b, -h, 
		l, -b, -h, 
		-l, -b, -h, 

		/* Rectangle 6 */
		-l, -b, h, 
		-l, b, h, 
		l, b, h, 
		l, b, h, 
		l, -b, h, 
		-l, -b, h, 

	};
	GLfloat C[]={
		/* Rectangle 1 (Bottom)*/
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,

		/* Rectangle 2 (left)*/
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,



		/* Rectangle 3  (Top layer)*/

		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,
		T.r, T.g, T.b,

		/* Rectangle 4 */
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,
		L.r, L.g, L.b,

		/* Rectangle 5 (right)*/

		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,

		/* Rectangle 6 */
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,
		R.r, R.g, R.b,


	};
	return create3DObject(GL_TRIANGLES,  6*6,  a,  C,  fill_mode);

}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;


bool check_collision(object a,object b){
	if((abs(a.x - b.x )<=a.l+b.l) && (abs(a.y-b.y)<=a.b+b.b) && (abs(a.z-b.z)<=a.h + b.h)) return true;
	return false;

}

int isontop(object a){
	for(int i = 0; i < 10; i++){
		for(int j = 0; j < 10; j++){
			object b = floors[i][j];
			if(GameMap[i][j]!= 3){
				if(abs(a.x - b.x ) < a.l+b.l-4 && (abs(a.z-b.z)<a.h + b.h-4)){
					if(a.y-a.b < b.y +b.b && a.y> b.y){

						return GameMap[i][j];
					}
				}
			}
		}
	}
	return 0;
}

VAO *rectangle;

void createBackground(GLuint textureID)
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-600,-300,0, // vertex 1
		600,-300,0, // vertex 2
		600, 300,0, // vertex 3

		600, 300,0, // vertex 3
		-600, 300,0, // vertex 4
		-600,-300,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	static const GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};

	// create3DTexturedObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}

void make_game(){

	obstacles.clear();
	playerobj.clear();
	
	for(int i = 0; i < 10; i++) floors[i].clear();

	COLOR grass,wood,black,danger,blue,green,white,grey,gold,skin,sh;
	
	black.r = black.g = black.b = 0;
	white.r = white.g = white.b = 1;
	
	grass.r = 26/255.0;
	grass.g = 161/255.0;
	grass.b = 48/255.0;
	
	green.r = 20/255.0;
	green.g = 128/255.0;
	green.b = 38/255.0;
	
	wood.r = 93/255.0;
	wood.g = 75/255.0;
	wood.b = 47/255.0;
	
	danger.r =174/255.0 ;
	danger.g = 42/255.0;
	danger.b =  42/255.0;
	
	grey.r = grey.g= grey.b = 0.2;
	
	gold.r = 255/255.0;;
	gold.g = 226/255.0;
	gold.b = 25/255.0;

	/* Creating Board */

	float length = 25;
	float breadth = 25;
	float height = 25;
	for(int  i = 0;i<10;i++){
		for(int j = 0; j<10;j++){
			object temp;
			temp.x = -200+i*2*(length+1);
			temp.y = -50;
			temp.l = length;
			temp.b = breadth;
			temp.h = height;
			temp.z = -100+j*(2*height+1);
			temp.scalevalue = 1.0;
			temp.delta = 0;
			temp.val = -1 + rand()%2;
			if(temp.val == 0)temp.val++;
			if(GameMap[i][j] == 2){
				temp.y+=breadth;
				temp.l*=0.8;
				temp.h*=0.8;
				temp.b*=2;
				temp.obj=generate_cube(length*0.8,2*breadth,height*0.8,green,grass,danger,GL_FILL);
			}
			else if(GameMap[i][j] == 3){
				temp.y-=0.5*temp.b;
				temp.b*=0.5;
				temp.obj=generate_cube(length,breadth*0.5,height,green,grass,danger,GL_FILL);
			}
			else if((i == 9 && j == 9 )|| (i == 0 && j == 0))
				temp.obj=generate_cube(length,breadth,height,grass,grass,gold,GL_FILL);
			else
				temp.obj=generate_cube(length,breadth,height,green,grass,wood);

			if(GameMap[i][j] == 3){
				temp.scalevalue = 0.5;
				temp.obj = NULL;
			}
			temp.ol = temp.l;
			temp.ob = temp.b;
			temp.oh = temp.h;
			floors[i].push_back(temp);
			if(GameMap[i][j] == 2){
				obstacles.push_back(temp);
			}
			if(GameMap[i][j] == 4){
				object t;
				t.x = temp.x;
				t.y = temp.y+temp.b*3;
				t.z = temp.z;
				t.obj = generate_cube(10,temp.b*2,10,gold,gold,gold);
				t.l = t.h = 10;
				t.b = temp.b*2;
				t.angle = 0;
				goalobj.push_back(t);
			}
			if(GameMap[i][j] == 5){
				object t;
				t.x = temp.x;
				t.y = temp.y+temp.b+10;
				t.z = temp.z;
				t.obj = generate_cube(10,10,10,gold,gold,gold);
				t.l = t.b = t.h = 10;
				t.angle = 0;
				coins.push_back(t);
			}

		}
	}

	/* Creating Player */
	
	float sidex = 10;
	float sidey = 25;
	float sidez = 10;
	float x = -200+9*2*(sidey+1)+sidey-20;
	float y = 0;
	float z = -100+8*2*(sidey+1)+sidey+10;
	COLOR skind;
	skind.r = 229/255.0;
	skind.g = 205/255.0;
	skind.b = 155/255.0;
	skin.r =  255/255.0;
	skin.g =  181/255.0;
	skin.b =  111/255.0;
	blue.r = 74/255.0;
	blue.g = 74/255.0;
	blue.b = 181/255.0;
	sh.r = 227/255.0;
	sh.g = 111/255.0;
	sh.b = 73/255.0;
	/* legs */
	for(int i = 0;i<2;i++){
		object p;
		string s = "legs";
		s+=('0'+i+1);
		p.x = x;
		p.y = y;
		p.z = z;
		p.obj = generate_cube(sidex,sidey,sidez*(0.8),skin,skind,skin,GL_FILL);
		p.l = sidex;
		p.b = sidey;
		p.h = sidez*(0.8);
		p.angle = 0;
		p.ol = p.l;
		p.ob  = p.b;
		p.oh = p.h;
		playerobj[s]= p;

		if(i%2 == 0){
			z-=2*sidez;
		}
		else{
			z+=2*sidez;
			y+=2*sidey;
		}
	}
	z = -100+8*2*(sidey+1)+sidey;

	/* body */

	object p;
	p.x = x;
	p.y = y;
	p.z = z;
	p.obj = generate_cube(sidex,sidey,2*sidez,blue,blue,sh,GL_FILL);
	p.l = sidex;
	p.b = sidey;
	p.h = 2*sidez;
	p.angle = 0;
	p.ol = p.l;
	p.ob  = p.b;
	p.oh = p.h;
	playerobj["body"]= p;


	/* Hands */

	object h1,h2;
	h1.x = h2.x = x;
	h1.z = z-sidez*2.5;
	h2.z = z+sidez*2.5;
	h1.y = h2.y = y;
	h1.l = h2.l = sidex;
	h1.ol = h2.ol = h1.l;
	h1.ob = h2.ob = h1.b;
	h1.oh = h2.oh = h1.h;
	h1.b = h2.b = sidey;
	h1.h = h2.h = sidez/2;
	h1.obj = generate_cube(sidex,sidey,sidez/2,skin,skind,sh);
	h2.obj = generate_cube(sidex,sidey,sidez/2,skin,skind,sh);
	h1.angle = h2.angle = 0;
	playerobj["hands1"] = h1;
	playerobj["hands2"] = h2;
	y+=sidey;

	/* neck */

	y+=sidey/8;
	object n;
	n.x = x;
	n.y = y;
	n.z = z;
	n.obj = generate_cube(sidex,sidey/8,sidez,skind,skind,skind);
	n.l = sidex;
	n.b = sidey/8;
	n.h = sidez;
	y+=sidey/8;
	n.angle = 0;
	playerobj["neck"] = n;	

	/* head */

	y+=sidey/2;
	object h;
	h.x  = x;
	h.y = y;
	h.z = z;
	h.obj = generate_cube(sidex,sidey/2.5,sidez,skin,skin,black);
	h.l = sidex;
	h.b = sidey/2.5;
	h.h = sidez;
	h.angle = 0;
	playerobj["head"] = h;

	/* Eyes */

	object e1,e2,no;
	no.x  = e1.x = e2.x = x-sidex;
	e1.z = z+sidez/2;
	e2.z = z-sidez/2;
	no.z = z;
	e1.y = e2.y = y+sidey/8;
	no.y = y-2;
	no.l = e1.l = e2.l = sidex/4;
	no.b = e1.b = e2.b = sidey/16;
	no.h = e1.h = e2.h = sidez/4;
	e1.obj = generate_cube(sidex/4,sidey/16,sidez/4,white,white,white);
	e2.obj = generate_cube(sidex/4,sidey/16,sidez/4,white,white,white);
	no.obj = generate_cube(sidex/4,sidey/16,sidez/4,white,white,white);
	e1.angle = e2.angle = 0;
	no.angle = 0;
	playerobj["nose"] = no;
	playerobj["eye1"] = e1;
	playerobj["eye2"] = e2;

	/* Hair */

	object l;
	l.x = x;
	l.y = y+sidey*(0.25+1/2.5);
	l.z = z;
	l.obj = generate_cube(sidex,sidey/4,sidez,grey,grey,grey);
	l.l = sidex;
	l.b = sidey/4;
	l.h = sidez;
	l.angle = 0;
	playerobj["fhair"] = l;

}
void display_string(float x,float y,float z,char *str,float fontScaleValue = 30){
	glUseProgram(fontProgramID);
	glm::vec3 fontColor = glm::vec3(1,1,1);
	float r = 500;
	Matrices.view = glm::lookAt(glm::vec3(60+r*cos(0), 300, 130+r*sin(0)),  glm::vec3(60, 0, 130),  glm::vec3(0, 1, 0));
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(x,y,z));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	glm::mat4 rot1 = glm::rotate((float)(M_PI/2), glm::vec3(0,1,0));
	Matrices.model *= (translateText * scaleText*rot1);
	glm::mat4 MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(str);
}
void init_game(){
	angle = 0;
	rotst = 0;
	view = 0;
	if(gamest == 1){
		score = pscore;
	}
	else if(gamest == 2){
		pscore = score;
	}
	xspeed = 0, zspeed = 0;
	rot = 0;
	val = 1;
	val1 = 1;
	prot = 0;
	delta = 0;
	gravity = 1;
	y_vel = 0;
	to_draw = 1;
	injump = 0;
	gamest = 0;
	sfac = 0;
	took = 0;
	sp_fac = 0;
	score = pscore;
	make_game();
	ptime = glfwGetTime();
	max_time = 20-levels*2;
	musicengine->setAllSoundsPaused(false);
	sfxengine->stopAllSounds();

}

/* Render the scene with openGL */
/* Edit this function according to your assignment */


void draw ()
{
	// clear the color and depth in the frame buffer

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int fbwidth=1200,fbheight=600;
	GLfloat fov = 0.9f+sfac;
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.01f, 9000.0f);

	// use the loaded shader program
	// Don't change unless you know what you are doing


	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f),  0,  5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0,  0,  0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0,  1,  0);
	float r = 500;
	Matrices.view = glm::lookAt(glm::vec3(60+r*cos(M_PI*angle/180), 300, 130+r*sin(M_PI*angle/180)),  glm::vec3(60, 0, 130),  glm::vec3(0, 1, 0)); // Fixed camera for 2D (ortho) in XY plane
	if(view == 1){
		Matrices.view = glm::lookAt(glm::vec3(200, 600, 0),  glm::vec3(0, 0, 0),  glm::vec3(0, 1, 0));
	}
	else if (view == 2){
		/* Head View */
		float x = playerobj["fhair"].x-(playerobj["fhair"].l*1.25)*cos(M_PI*prot/180);
		float y = playerobj["fhair"].y+playerobj["fhair"].b;
		float z = playerobj["fhair"].z+(playerobj["fhair"].l*1.25)*sin(M_PI*prot/180);
		float r = 100;
		Matrices.view = glm::lookAt(glm::vec3(x, y+10, z),  glm::vec3(x-r*cos(M_PI*prot/180),y/1.5, z+r*sin(M_PI*prot/180)),  glm::vec3(0, 1, 0));
	}
	else if (view == 4){
		/* Follow Cam View */
		float x = playerobj["fhair"].x-(playerobj["fhair"].l*1.25)*cos(M_PI*prot/180);
		float y = playerobj["fhair"].y+playerobj["fhair"].b;
		float z = playerobj["fhair"].z+(playerobj["fhair"].l*1.25)*sin(M_PI*prot/180);
		float r = 200;
		Matrices.view = glm::lookAt(glm::vec3(x+r*cos(M_PI*prot/180),y+100, z-r*sin(M_PI*prot/180)),  glm::vec3(x-r*cos(M_PI*prot/180),y/4, z+r*sin(M_PI*prot/180)),  glm::vec3(0, 1, 0));		
	}

	glm::mat4 VP = Matrices.projection * Matrices.view;

	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix

	/* Render your scene */
	int fl = 0;
	char level_str[30],speed_str[20],score_str[20],timer[20],lifes_str[30],displ_str[40];
	float sp = sp_fac+2;
	sp*=10;
	sp =abs(sp);
	sprintf(lifes_str,"Lifes %d",lives);
	sprintf(level_str,"Level %d",levels+1);
	sprintf(score_str,"Your Score: %0.lf",score);
	sprintf(speed_str,"Speed %.0lf",sp);
	sprintf(displ_str,"Game is in Pause . Press P to unpause.");
	if(in_pause){
		display_string(100,100,300,displ_str,40);
		return ;
	}
	if(game_end == 1){
		char str[40];
		sprintf(str,"Game Over Thanks for Playing");
		display_string(200,10,-100,str,40);
		display_string(200,200,-10,score_str,20);
		sleep(1);
		game_end = 2;
		return ;

	}
	if(to_draw == 0){

		display_string(100,100,300,disp_str,40);
		display_string(200,300,-20,score_str,60);
		sleep(1);
		to_draw = 2;
		sleep(2);
		return;
	}
	float time1;
	time1 = glfwGetTime() - ptime;
	sprintf(timer,"Timer %0.lf",max_time - time1);
	if(time1 > max_time){
		to_draw = 0;
		view = 0;
		lives--;
		if(lives>0)
			strcpy(disp_str ,"Life Lost . Time Over");
		else{
			strcpy(disp_str ,"Game Over. You exhausted all lifes");
			game_end = 1;
		}
		gamest = 1;
		play_sfx(2);
		return ;
	}
	display_string(300,250,150,timer,20);
	display_string(200,250,50,level_str);
	display_string(200,200,20,score_str);
	display_string(200,150,-50,speed_str);
	display_string(300,300,-50,level_str);
	glUseProgram(textureProgramID);

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(M_PI/2.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;

	// Copy MVP to texture shaders
	glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Set the texture sampler to access Texture0 memory
	glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DTexturedObject(rectangle);

	glUseProgram (programID);
	map<string,object>::iterator itr;
	map<string,float> orig_x,orig_z,orig_y;
	for(int i = 0;i<goalobj.size();i++){
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 transobj = glm::translate (glm::vec3(goalobj[i].x, goalobj[i].y, goalobj[i].z)); 
		Matrices.model *= transobj; 
		MVP = VP * Matrices.model; 
		glUniformMatrix4fv(Matrices.MatrixID,  1,  GL_FALSE,  &MVP[0][0]);
		draw3DObject(goalobj[i].obj);
	}
	for(int i = 0;i<coins.size();i++){
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 transobj = glm::translate (glm::vec3(coins[i].x, coins[i].y, coins[i].z)); 
		Matrices.model *= transobj; 
		MVP = VP * Matrices.model; 
		glUniformMatrix4fv(Matrices.MatrixID,  1,  GL_FALSE,  &MVP[0][0]);
		draw3DObject(coins[i].obj);
	}
	if(xspeed!=0){
		rot-=val1;
		if(rot<-20)val1 = -1;
		else if(rot>20) val1 = 1;
	}
	else if (rot!=0){
		if(rot>0)rot -= 1;
		else if (rot<0)rot+=1;
	}
	//out2(delta,val);		
	for(int i = 0;i<10;i++){
		for(int j = 0;j<floors[i].size();j++){
			Matrices.model = glm::mat4(1.0f);
			if(GameMap[i][j] == 2){
				if(rand()%2 == 1){
					floors[i][j].y+=floors[i][j].val;
					floors[i][j].delta+=floors[i][j].val;
					float del = floors[i][j].delta; 
					if(del < -40 ||del > 40){
						floors[i][j].val = -floors[i][j].val;
					}
				}
			}
			glm::mat4 transobj = glm::translate (glm::vec3(floors[i][j].x, floors[i][j].y, floors[i][j].z)); 
			Matrices.model *= transobj; 
			MVP = VP * Matrices.model; 
			glUniformMatrix4fv(Matrices.MatrixID,  1,  GL_FALSE,  &MVP[0][0]);
			draw3DObject(floors[i][j].obj);
		}	
	}
	bool yfl = 0,fall = 0;
	float am = 0,xam = 0,zam = 0;
	y_vel-= gravity;
	if(y_vel < 0 && isontop(playerobj["legs1"])!=0 && isontop(playerobj["legs2"])!=0){
		y_vel = 0;
	}
	prot+=zspeed;

	if(playerobj["legs1"].y < -25){
		to_draw = 0;
		view = 0;
		lives--;
		if(lives>0)
			strcpy(disp_str ,"You Fell Down,Life Lost");
		else{
			strcpy(disp_str ,"Game Over.You exhausted all lifes");
			game_end = 1;
		}
		gamest = 1;
		play_sfx(2);
		return ;
	}
	for(itr = playerobj.begin();itr!=playerobj.end();itr++){
		string i = itr->first;
		orig_x[i] = playerobj[i].x;
		orig_z[i] = playerobj[i].z;
		orig_y[i] = playerobj[i].y;
		float vel = xspeed;
		if(vel>0)vel+=sp_fac;
		else if(vel!=0)vel-=sp_fac;

		playerobj[i].y+=y_vel;
		playerobj[i].x += vel*cos(prot*M_PI/180.0);
		playerobj[i].z -= vel*sin(prot*M_PI/180.0);

		float side1,side2;
		side1 = floors[0][0].l;
		side2 = floors[0][0].h;
		int x = (playerobj[i].x - XMAX+10)/(2*side1);
		int y = (playerobj[i].z - ZMAX)/(2*side2);
		for(int j = 0;j < 10; j++){
			for(int k = 0; k < 10; k++){
				if(GameMap[j][k] == 2 && check_collision(playerobj[i],floors[j][k])){
					if(i[0] == 'l' && GameMap[9+x][9+y] == 2 && isontop(playerobj[i]) == 2 && playerobj[i].y > 0){
						yfl = 1;
						am = abs(orig_y[i]- playerobj[i].b-(floors[j][k].y + floors[j][k].b));
					}
					else {
						fl = 1;
					}
				}
			}
		}
		if(i[0] == 'l' && GameMap[9+x][9+y] == 5){
			for(int j = 0; j< coins.size(); j++){
				if(check_collision(playerobj[i],coins[j])){
					coins.erase(coins.begin()+j);
					score += 10;
					break;
				}
			}
		}
		if(took == 1 )goalobj.clear();
		if(took == 0){
			if(check_collision(playerobj[i],goalobj[0])){
				took = 1;
				goalobj.erase(goalobj.begin());
			}
		}
		if(took == 1 && 9+x == 0  && 9+y == 0){
			to_draw = 0;
			view = 0;
			gamest = 2;
			musicengine->setAllSoundsPaused(true);
			play_sfx(3);
			score+=100;
			score+=(20-time1)*2;
			strcpy(disp_str ,"You Won. Level Completed");
			levels++;
			levels = min(levels,5);
			return ;
		}

		if(playerobj[i].x > XMAX || playerobj[i].x < XMIN){
			fl = 2;
		}
		if(playerobj[i].z < ZMIN || playerobj[i].z> ZMAX){
			fl = 2;
		}
		if(i[0] == 'l'){
			if(playerobj[i].y - playerobj[i].b < -25 && isontop(playerobj["legs1"])!=0){
				fall = 1;
			}
		}
	}
	if(fl == 1 && playerobj["legs1"].y>0 && playerobj["legs2"].y>0){
		fl = 0;
	}

	if(y_vel < 0 && isontop(playerobj["legs1"])!=0){
		y_vel = 0;
		if(injump == 1)injump = 0;
	}
	if(fl !=0){
		for(itr = playerobj.begin();itr!=playerobj.end();itr++){
			string i = itr->first;
			playerobj[i].x = orig_x[i]+xam;
			playerobj[i].z = orig_z[i]+zam;
		}
	}
	if(yfl == 1){
		for(itr = playerobj.begin();itr!=playerobj.end();itr++){
			string i = itr->first;
			playerobj[i].y = orig_y[i]+am;
		}
	}
	if(fall == 1){
		for(itr = playerobj.begin();itr!=playerobj.end();itr++){
			string i = itr->first;
			playerobj[i].y = orig_y[i];
		}
	}
	if(to_draw == 1){
		for(itr = playerobj.begin();itr!=playerobj.end();itr++){
			string i = itr->first;
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 transobj = glm::translate (glm::vec3(playerobj["body"].x, playerobj["body"].y, playerobj["body"].z));
			Matrices.model *= transobj;
			glm::mat4 rotobj = glm::rotate((float)(prot*M_PI/180.0f), glm::vec3(0,1,0));
			Matrices.model *= rotobj;
			transobj = glm::translate (glm::vec3(-playerobj["body"].x, -playerobj["body"].y, -playerobj["body"].z));
			Matrices.model *= transobj;
			transobj = glm::translate (glm::vec3(playerobj[i].x, playerobj[i].y, playerobj[i].z));
			Matrices.model *= transobj;
			if(i[0] == 'l' || i[1] == 'a'){
				transobj = glm::translate(glm::vec3(0,playerobj[i].b,0));
				Matrices.model *= transobj;
				if(i[1] == 'a' && injump == 1){
					rotobj = glm::rotate((float)(M_PI), glm::vec3(0,0,-1));
				}
				else
					rotobj = glm::rotate((float)(rot*M_PI/180.0f), glm::vec3(0,0,-1));
				Matrices.model *= rotobj;
				transobj = glm::translate(glm::vec3(0,-playerobj[i].b,0));
				Matrices.model *= transobj;
				rot = -rot;
			}
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID,  1,  GL_FALSE,  &MVP[0][0]);
			draw3DObject(playerobj[i].obj);
		}
	}
	angle+=rotst;
}

/* Initialise glfw window,  I/O callbacks and the renderer to use */
/* Nothing to Edit here */


GLFWwindow* initGLFW (int width,  int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,  GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE,  GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window,mousescroll);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models

	/* Standard Colors */

	init_game();
	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	GLuint textureID = createTexture("background.png");
	// check for an error during the load process
	if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");
	//createBackground(textureID);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (76/255.0,132/255.0,183/255.0, 0.5f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	const char* fontfile = "Monaco.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
	GL3Font.font->CharMap(ft_encoding_unicode);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1200;
	int height = 600;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	musicengine->play2D("./music/1.mp3",true);
	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {
		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		current_time = glfwGetTime();
		if ((current_time - last_update_time) >= 0.5) {
			last_update_time = current_time;
		}
		if(game_end == 2){
			sleep(1);
			quit(window);
		}
		if(to_draw == 2){
			sleep(1);
			init_game();
			to_draw = 1;
		}

	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
