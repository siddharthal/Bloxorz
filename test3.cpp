#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	//    exit(EXIT_SUCCESS);
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

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
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

/**************************
 * Customizable functions *
 **************************/

int left_press=0,right_press=0,up_press=0,down_press=0;
int arena[15][10];
glm::mat4 MVP,VP;

struct block_positions{
	int x1;
	int y1;
	int x2;
	int y2;
	int orientation;
	float translate_x;
	float translate_y;
	float translate_z;
	int x_axis;
	int y_axis;
	int z_axis;	
};

struct block_positions block_position;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UP:
				//				up_press=0;
				break;
			case GLFW_KEY_LEFT:
				//				left_press=0;
				break;
			case GLFW_KEY_DOWN:
				//				down_press=0;
				break;
			case GLFW_KEY_RIGHT:
				//				right_press=0;
				break;
			case GLFW_KEY_C:
				break;
			case GLFW_KEY_P:
				break;
			case GLFW_KEY_X:
				// do something ..
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_UP:
				up_press=1;
				break;
			case GLFW_KEY_LEFT:
				left_press=1;
				break;
			case GLFW_KEY_DOWN:
				down_press=1;
				break;
			case GLFW_KEY_RIGHT:
				right_press=1;
				break;
			case GLFW_KEY_ESCAPE:
				quit(window);
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
				break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = M_PI/4;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-9.0f, 9.0f, -6.0f, 6.0f, 0.1f, 500.0f);
}

VAO  *block_vertical,*block_horizontal1,*block_horizontal2,*tile;

void createBlockVertical ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0,0,0, // vertex 1
		1,0,0, // vertex 2
		0,2,0, // vertex 3
		1,2,0, // vertex 3
		1,0,0, // vertex 
		0,2,0,

		0,0,1,
		1,0,1,
		0,2,1,
		1,2,1,
		1,0,1,
		0,2,1,

		0,0,0,
		0,2,0,
		0,0,1,
		0,2,1,
		0,2,0,
		0,0,1,

		1,0,0,
		1,2,0,
		1,0,1,
		1,2,1,
		1,2,0,
		1,0,1,

		0,0,0,
		1,0,0,
		0,0,1,
		1,0,1,
		1,0,0,
		0,0,1,

		0,2,0,
		1,2,0,
		0,2,1,
		1,2,1,
		1,2,0,
		0,2,1
	};

	static const GLfloat color_buffer_data [] = {
		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	block_vertical = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBlockHorizontal1 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0,0,0, 
		2,0,0, 
		0,1,0, 
		2,1,0, 
		2,0,0, 
		0,1,0,

		0,0,1,
		2,0,1,
		0,1,1,
		2,1,1,
		2,0,1,
		0,1,1,

		0,0,0,
		0,1,0,
		0,0,1,
		0,1,1,
		0,1,0,
		0,0,1,

		2,0,0,
		2,1,0,
		2,0,1,
		2,1,1,
		2,1,0,
		2,0,1,

		0,0,0,
		2,0,0,
		0,0,1,
		2,0,1,
		2,0,0,
		0,0,1,

		0,1,0,
		2,1,0,
		0,1,1,
		2,1,1,
		2,1,0,
		0,1,1
	};

	static const GLfloat color_buffer_data [] = {
		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	block_horizontal1 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createBlockHorizontal2 ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0,0,0, 
		1,0,0, 
		0,1,0, 
		1,1,0, 
		1,0,0, 
		0,1,0,

		0,0,2,
		1,0,2,
		0,1,2,
		1,1,2,
		1,0,2,
		0,1,2,

		0,0,0,
		0,1,0,
		0,0,2,
		0,1,2,
		0,1,0,
		0,0,2,

		1,0,0,
		1,1,0,
		1,0,2,
		1,1,2,
		1,1,0,
		1,0,2,

		0,0,0,
		1,0,0,
		0,0,2,
		1,0,2,
		1,0,0,
		0,0,2,

		0,1,0,
		1,1,0,
		0,1,2,
		1,1,2,
		1,1,0,
		0,1,2
	};

	static const GLfloat color_buffer_data [] = {
		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

		0.1,0.1,0.1,
		0.3,0.2,0.1, 
		0.3,0.2,0.1, 
		0.1,0.1,0.1,
		0.3,0.2,0.1,
		0.3,0.2,0.1, 

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	block_horizontal2 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTile()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0,0, // vertex 1
		1,0,0, // vertex 2
		0,0.2,0, // vertex 3
		1,0.2,0, // vertex 3
		1,0,0, // vertex 4
		0,0.2,0,

		0,0,1,
		1,0,1,
		0,0.2,1,
		1,0.2,1,
		1,0,1,
		0,0.2,1,

		0,0,0,
		0,0.2,0,
		0,0,1,
		0,0.2,1,
		0,0.2,0,
		0,0,1,

		1,0,0,
		1,0.2,0,
		1,0,1,
		1,0.2,1,
		1,0.2,0,
		1,0,1,

		0,0,0,
		1,0,0,
		0,0,1,
		1,0,1,
		1,0,0,
		0,0,1,

		0,0.2,0,
		1,0.2,0,
		0,0.2,1,
		1,0.2,1,
		1,0.2,0,
		0,0.2,1
	};

	static const GLfloat color_buffer_data [] = {
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 

		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 

		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 

		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 

		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 

		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6, 
		0.3,0.3,0.3,
		0.6,0.6,0.6, 
		0.6,0.6,0.6

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	tile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90,block_rotation=0,tile_rotation=0;

void check_key_functions()
{
	if(left_press==1)
	{
		block_position.translate_x=0;
		block_position.translate_y=0;
		block_position.translate_z=0;
		block_position.x_axis=0;
		block_position.y_axis=0;
		block_position.z_axis=1;
		if(block_rotation < 90)
			block_rotation+=2;

		if(block_position.orientation==1)
		{
			if(block_rotation >=90)
			{
				block_rotation=0;
				block_position.orientation=0;
				block_position.x1-=2;
				left_press=0;
			}
		}

		else if(block_position.orientation==0)
		{
			if(block_rotation >=90)
			{
				block_rotation=0;
				block_position.orientation=1;
				block_position.x1-=1;
				left_press=0;
			}

		}
		else
		{
			if(block_rotation >=90)
			{
				block_rotation=0;
				block_position.x1-=1;
				left_press=0;
			}
		}
	}

	else if(right_press==1)
	{
		if(block_position.orientation==1)
		{
			block_position.translate_x=-1;
			block_position.translate_y=0;
			block_position.translate_z=0;
			block_position.x_axis=0;
			block_position.y_axis=0;
			block_position.z_axis=-1;
			if(block_rotation < 90)
				block_rotation+=2;
			else
			{
				block_rotation=0;
				block_position.orientation=0;
				block_position.x1+=1;
				right_press=0;
			}
		}
		else if(block_position.orientation==0)
		{	
			block_position.translate_x=-2;
			block_position.translate_y=0;
			block_position.translate_z=0;
			block_position.x_axis=0;
			block_position.y_axis=0;
			block_position.z_axis=-1;
			if(block_rotation<90)
				block_rotation+=2;
			else
			{
				block_rotation=0;
				block_position.orientation=1;
				block_position.x1+=2;
				right_press=0;
			}			
		}
		else
		{
			block_position.translate_x=-1;
			block_position.translate_y=0;
			block_position.translate_z=0;
			block_position.x_axis=0;
			block_position.y_axis=0;
			block_position.z_axis=-1;
			if(block_rotation<90)
				block_rotation+=2;
			else
			{
				block_rotation=0;
				block_position.x1+=1;
				right_press=0;
			}
		}
	}

	else if(down_press==1)
	{
		if(block_position.orientation==1)
		{
			block_position.translate_x=0;
			block_position.translate_y=0;
			block_position.translate_z=-1;
			block_position.x_axis=1;
			block_position.y_axis=0;
			block_position.z_axis=0;
			if(block_rotation<90)
				block_rotation+=2;
			else
			{
				block_rotation=0;
				block_position.orientation=2;
				block_position.y1-=1;
				down_press=0;
			}
		}

		else if(block_position.orientation==0)
		{
			block_position.translate_x=0;
			block_position.translate_y=0;
			block_position.translate_z=-1;
			block_position.x_axis=1;
			block_position.y_axis=0;
			block_position.z_axis=0;
			if(block_rotation<90)
				block_rotation+=2;
			else
			{
				block_rotation=0;
				block_position.orientation=0;
				block_position.y1-=1;
				down_press=0;
			}
		}
		else
		{
			block_position.translate_x=0;
			block_position.translate_y=0;
			block_position.translate_z=-2;
			block_position.x_axis=1;
			block_position.y_axis=0;
			block_position.z_axis=0;
			if(block_rotation<90)
				block_rotation+=2;
			else
			{
				block_rotation=0;
				block_position.orientation=1;
				block_position.y1-=2;
				down_press=0;
			}
		}
	}

	else if(up_press==1)
	{
			block_position.translate_x=0;
			block_position.translate_y=0;
			block_position.translate_z=0;
			block_position.x_axis=-1;
			block_position.y_axis=0;
			block_position.z_axis=0;
			if(block_rotation < 90)
				block_rotation+=2;

		if(block_position.orientation==1)
		{
			if(block_rotation>=90)
			{
				block_rotation=0;
				block_position.orientation=2;
				block_position.y1+=2;
				up_press=0;
			}

		}
		else if(block_position.orientation==0)
		{
			if(block_rotation>=90)
			{
				block_rotation=0;
				block_position.orientation=0;
				block_position.y1+=1;
				up_press=0;
			}
		}
		else
		{
			if(block_rotation>=90)
			{
				block_rotation=0;
				block_position.orientation=1;
				block_position.y1+=1;
				up_press=0;
			}
		}
	}
}

void draw_tiles()
{
	float x_start=-7.5,z_start=5;
	int i,j;
	for(i=0;i<15;i++)
	{
		z_start=5;
		for(j=0;j<10;j++)
		{
			Matrices.model = glm::mat4(1.0f);
			glm::mat4 translateTile = glm::translate (glm::vec3(x_start,0, z_start));        
			glm::mat4 rotateTile = glm::rotate((float)(tile_rotation*M_PI/180.0f), glm::vec3(1,1,0)); 
			Matrices.model *= (translateTile * rotateTile);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(tile);
			z_start-=1;
		}
		x_start+=1;
	}
}

void draw_block()
{
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle = glm::translate (glm::vec3(-7.5+block_position.x1,0.2, 5-block_position.y1));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(block_rotation*M_PI/180.0f), glm::vec3(block_position.x_axis,block_position.y_axis,block_position.z_axis));
	glm::mat4 translateRotate = glm::translate (glm::vec3(block_position.translate_x,block_position.translate_y,block_position.translate_z));
	glm::mat4 translateCancel =  glm::translate (glm::vec3(-1*block_position.translate_x,-1*block_position.translate_y,-1*block_position.translate_z)); 
	Matrices.model *= (translateRectangle * translateCancel* rotateRectangle * translateRotate);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(block_position.orientation==1)
		draw3DObject(block_vertical);
	else if(block_position.orientation==0)
		draw3DObject(block_horizontal1);
	else
		draw3DObject(block_horizontal2);

}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,15,9), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model yfou render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	// MVP = Projection * View * Model

	// Load identity to model matrix
	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	check_key_functions();
	draw_tiles();
	draw_block();

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		//        exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//        exit(EXIT_FAILURE);
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

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createBlockVertical ();
	createTile();
	createBlockHorizontal1();
	createBlockHorizontal2();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (1,1,1, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void initialiseArena()
{
	int i,j;
	for(i=0;i<15;i++)
	{
		for(j=0;j<10;j++)
		{
			arena[i][j]=1;
		}
	}
	block_position.x1=5;
	block_position.y1=5;
	block_position.orientation=1;
}

int main (int argc, char** argv)
{
	int width = 900;
	int height = 600;

	initialiseArena();
	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
