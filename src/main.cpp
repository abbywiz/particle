#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib> // For rand()


/* ------------------------ GLOBALS  --------------------------- */


// Screen Dimensions
const int gScreenWidth 						= 800;
const int gScreenHeight 					= 600;
SDL_Window* gGraphicsApplicationWindow 	= nullptr;
SDL_GLContext gOpenGLContext			= nullptr;


// Main loop flag
bool gQuit = false; // If this is quit = 'true' then the program terminates.


GLuint gGraphicsPipelineShaderProgram	= 0;
GLuint gVertexArrayObject					= 0;
GLuint 	gVertexBufferObject					= 0;
GLuint 	gIndexBufferObject                  = 0;


// Whether we will keep drawing pixels or not
// Toggled by the space bar
bool simulation_running = false;

// Define the number of particles
int NUM_PARTICLES = 50;

// Define the Velocity of particles
float velocity = 0.5f;

float particleSizeValue = 5.0f;  // Change this value to adjust particle size

glm::vec3 centralDotPosition = glm::vec3(0.0f);

std::vector<GLfloat> vertexData;

// Define which shape we want the particles to be
// Let 0 be triangle, 1 be rhombus, 2 be hexagon
//int shape = 0;

// Define a structure to represent a particle
struct Particle
{
    glm::vec3 position;
    glm::vec3 color;
    int shape;
    float lifespan;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};


//Will hold all the particles we want to draw
std::vector<Particle> particles;



// vvvvvvvvvvvvvvvvvvv Error Handling Routines vvvvvvvvvvvvvvv
static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){
    }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
    while(GLenum error = glGetError()){
        std::cout << "OpenGL Error:" << error 
                  << "\tLine: " << line 
                  << "\tfunction: " << function << std::endl;
        return true;
    }
    return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);
// ^^^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^


/**
* LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       LoadShaderAsString("./shaders/filepath");
*
* @param filename Path to the shader file
* @return Entire file stored as a single string 
*/
std::string LoadShaderAsString(const std::string& filename){
    // Resulting shader program loaded as a single string
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if(myFile.is_open()){
        while(std::getline(myFile, line)){
            result += line + '\n';
        }
        myFile.close();

    }

    return result;
}


/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*	    Compile a vertex shader: 	CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*       Compile a fragment shader: 	CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
*
* @param type We use the 'type' field to determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile our shaders
	GLuint shaderObject;

	// Based on the type passed in, we create a shader object specifically for that
	// type.
	if(type == GL_VERTEX_SHADER){
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}else if(type == GL_FRAGMENT_SHADER){
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of our shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// Now compile our shader
	glCompileShader(shaderObject);

	// Retrieve the result of our compilation
	int result;
	// Our goal with glGetShaderiv is to retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE){
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length]; // Could also use alloca here.
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if(type == GL_VERTEX_SHADER){
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
		}else if(type == GL_FRAGMENT_SHADER){
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
		}
		// Reclaim our memory
		delete[] errorMessages;

		// Delete our broken shader
		glDeleteShader(shaderObject);

		return 0;
	}

  return shaderObject;
}



/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    // Create a new program object
    GLuint programObject = glCreateProgram();

    // Compile our shaders
    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link our two shader programs together.
	// Consider this the equivalent of taking two .cpp files, and linking them into
	// one executable file.
    glAttachShader(programObject,myVertexShader);
    glAttachShader(programObject,myFragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);

    // Once our final program Object has been created, we can
	// detach and then delete our individual shaders.
    glDetachShader(programObject,myVertexShader);
    glDetachShader(programObject,myFragmentShader);
	// Delete the individual shaders once we are done
    glDeleteShader(myVertexShader);
    return programObject;
}

/**
* Create the graphics pipeline
*
* @return void
*/
void CreateGraphicsPipeline(){

    std::string vertexShaderSource      = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource    = LoadShaderAsString("./shaders/frag.glsl");

	gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource,fragmentShaderSource);
}


/**
* Initialization of the graphics application. Typically this will involve setting up a window
* and the OpenGL Context (with the appropriate version)
*
* @return void
*/
void InitializeProgram(){
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)< 0){
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}
	
	// Setup the OpenGL Context
	// Use OpenGL 4.1 core or greater
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	// We want to request a double buffer for smooth updating.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create an application window using OpenGL that supports SDL
	gGraphicsApplicationWindow = SDL_CreateWindow( "Particle Simulator",
													SDL_WINDOWPOS_UNDEFINED,
													SDL_WINDOWPOS_UNDEFINED,
													gScreenWidth,
													gScreenHeight,
													SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	// Check if Window did not create.
	if( gGraphicsApplicationWindow == nullptr ){
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Create an OpenGL Graphics Context
	gOpenGLContext = SDL_GL_CreateContext( gGraphicsApplicationWindow );
	if( gOpenGLContext == nullptr){
		std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Initialize GLAD Library
	if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
		std::cout << "glad did not initialize" << std::endl;
		exit(1);
	}
	
}


void VertexSpecification(){

	// Geometry Data
	// Here we are going to store x,y, and z position attributes within vertexPositons for the data.
	// For now, this information is just stored in the CPU, and we are going to store this data
	// on the GPU shortly, in a call to glBufferData which will store this information into a
	// vertex buffer object.
	// Note: That I have segregated the data from the OpenGL calls which follow in this function.
	//       It is not strictly necessary, but I find the code is cleaner if OpenGL (GPU) related
	//       functions are packed closer together versus CPU operations.



    // Append cursorVertexData to vertexData
    vertexData.push_back(centralDotPosition.x); // Center
    vertexData.push_back(centralDotPosition.y); // Top
    vertexData.push_back(-0.9);
    vertexData.push_back(1.0f); // Red
    vertexData.push_back(0.0f); // Green
    vertexData.push_back(0.0f); // Blue

    Particle p;
    p.position =  glm::vec3(centralDotPosition.x, centralDotPosition.y, -0.9);
    p.lifespan = 1.0f;
    p.velocity = glm::vec3(0.0f,0.0f,0.0f);
    p.color = glm::vec3(1.0f, 0.0f, 0.0f);
    p.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    p.shape = 0;

    particles.push_back(p);

    // Initialize particles and vertexData
    for (int i = 0; i < NUM_PARTICLES; ++i)
    {

        glm::vec3 position = glm::vec3((rand() % gScreenWidth) / (float)gScreenWidth * 2 - 1, 
                                       (rand() % gScreenHeight) / (float)gScreenHeight * 2 - 1, 
                                       -0.9);
        glm::vec3 color = glm::vec3((rand() % 255) / 255.0f, 
                                     (rand() % 255) / 255.0f, 
                                     (rand() % 255) / 255.0f);
        int shape = rand() % 3;

        Particle p;
        p.position =  position;
        p.lifespan = 1.0f;
        p.velocity = glm::vec3((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, 0.0f);
        p.color = color;
        p.acceleration = glm::vec3(0.0f, -0.5f, 0.0f);
        p.shape = shape;

        particles.push_back(p);

        vertexData.push_back(position.x);
        vertexData.push_back(position.y);
        vertexData.push_back(position.z);

        vertexData.push_back(color.r);
        vertexData.push_back(color.g);
        vertexData.push_back(color.b);
    }


	// Vertex Arrays Object (VAO) Setup
	// Note: We can think of the VAO as a 'wrapper around' all of the Vertex Buffer Objects,
	//       in the sense that it encapsulates all VBO state that we are setting up.
	//       Thus, it is also important that we glBindVertexArray (i.e. select the VAO we want to use)
	//       before our vertex buffer object operations.
	glGenVertexArrays(1, &gVertexArrayObject);
	// We bind (i.e. select) to the Vertex Array Object (VAO) that we want to work withn.
	glBindVertexArray(gVertexArrayObject);

	// Vertex Buffer Object (VBO) creation
	// Create a new vertex buffer object
	// Note:  We’ll see this pattern of code often in OpenGL of creating and binding to a buffer.
	glGenBuffers(1, &gVertexBufferObject);
	// Next we will do glBindBuffer.
	// Bind is equivalent to 'selecting the active buffer object' that we want to
	// work with in OpenGL.
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	// Now, in our currently binded buffer, we populate the data from our
	// 'vertexPositions' (which is on the CPU), onto a buffer that will live
	// on the GPU.
	glBufferData(GL_ARRAY_BUFFER, 						// Kind of buffer we are working with 
														// (e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
				 vertexData.size() * sizeof(GL_FLOAT), 	// Size of data in bytes
				 vertexData.data(), 					// Raw array of data
				 GL_STATIC_DRAW);		

	// For our Given Vertex Array Object, we need to tell OpenGL
	// 'how' the information in our buffer will be used.
	glEnableVertexAttribArray(0);
	// For the specific attribute in our vertex specification, we use
	// 'glVertexAttribPointer' to figure out how we are going to move
	// through the data.
    glVertexAttribPointer(0,  		// Attribute 0 corresponds to the enabled glEnableVertexAttribArray
							  		// In the future, you'll see in our vertex shader this also correspond
							  		// to (layout=0) which selects these attributes.
                          3,  		// The number of components (e.g. x,y,z = 3 components)
                          GL_FLOAT, // Type
                          GL_FALSE, // Is the data normalized
                          sizeof(GL_FLOAT)*6, 		// Stride
                         (void*)0	// Offset
    );


    // Now linking up the attributes in our VAO
    // Color information
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3, // r,g,b
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*6,
                          (GLvoid*)(sizeof(GL_FLOAT)*3)
    );


	// Unbind our currently bound Vertex Array Object
	glBindVertexArray(0);
	// Disable any attributes we opened in our Vertex Attribute Arrray,
	// as we do not want to leave them open. 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}


/**
* PreDraw
* Typically we will use this for setting some sort of 'state'
* Note: some of the calls may take place at different stages (post-processing) of the
* 		 pipeline.
* @return void
*/
void PreDraw(){
	// Disable depth test and face culling.
    glEnable(GL_DEPTH_TEST);                    // NOTE: Need to enable DEPTH Test
    glDisable(GL_CULL_FACE);

    // Initialize clear color
    // This is the background of the screen.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, gScreenWidth, gScreenHeight);

    // Use our shader
	glUseProgram(gGraphicsPipelineShaderProgram);

    // Model transformation by translating our object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,-5.0f)); 

	
    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)gScreenWidth/(float)gScreenHeight,
                                             0.1f,
											 200.0f);

	glm::mat4 mp = perspective * model;

    // Send data to GPU
	GLint MatrixID = glGetUniformLocation(gGraphicsPipelineShaderProgram,"MP");
    if(MatrixID>=0){
        glUniformMatrix4fv(MatrixID,1,GL_FALSE,&mp[0][0]);
    }else{
        std::cout << "Could not find mp, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

    //  // Update centralDotPosition uniform
    // GLint centralDotPosID = glGetUniformLocation(gGraphicsPipelineShaderProgram, "centralDotPosition");
    // if(centralDotPosID >= 0){
    //     glUniform3fv(centralDotPosID, 1, &centralDotPosition[0]);
    // }else{
    //     std::cout << "Could not find centralDotPosition uniform\n";
    // }

    GLint particleSizeLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "particleSize");
    glUniform1f(particleSizeLocation, particleSizeValue);

}

/**
* Draw
* The render function gets called once per loop.
* Typically this includes 'glDraw' related calls, and the relevant setup of buffers
* for those calls.
*
* @return void
*/
void Draw(){
    // Enable our attributes
	glBindVertexArray(gVertexArrayObject);

	// Select the vertex buffer object we want to enable
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

    //Draw the Cursor
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, 1);


    //Render it all as points
    glPointSize(particleSizeValue);
    glDrawArrays(GL_POINTS, 1, particles.size()-1);

    //Render data
    //TODO: Make Hexagon and Rhombus Work
    // for (int i = 1; i < NUM_PARTICLES + 1; ++i) {
    //     switch (particles[i].shape)
    //     {
    //     case 0:  // Triangle
    //         glDrawArrays(GL_TRIANGLES, i * 3, 3);  // Assuming each particle has 3 vertices for triangle
    //         break;
        
    //     case 1:  // Rhombus
    //         glDrawArrays(GL_TRIANGLE_FAN, NUM_PARTICLES * 3 + i * 4, 4);  // Assuming each particle has 4 vertices for rhombus
    //         break;

    //     case 2:  // Hexagon
    //         glDrawArrays(GL_TRIANGLE_FAN, NUM_PARTICLES * 3 + NUM_PARTICLES * 4 + i * 6, 6);  // Assuming each particle has 6 vertices for hexagon
    //         break;
        
    //     default:
    //         break;
    //     }
    // }



	// Stop using our current graphics pipeline
	// Note: This is not necessary if we only have one graphics pipeline.
    glUseProgram(0);
}


/**
* Helper Function to get OpenGL Version Information
*
* @return void
*/
void getOpenGLVersionInfo(){
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "Version: " << glGetString(GL_VERSION) << "\n";
  std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}


/**
* Function called in the Main application loop to handle user input
*
* @return void
*/
void Input() {

	// Event handler that handles various events in SDL
	// that are related to input and output
	SDL_Event e;
	//Handle events on queue
	while(SDL_PollEvent( &e ) != 0){
		// If users posts an event to quit
		// An example is hitting the "x" in the corner of the window.
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			gQuit = true;
		}
        if(e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_ESCAPE)){
			std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            gQuit = true;
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE){
            // Start or stop simulation
            simulation_running = !simulation_running;
        }


        // Retrieve keyboard state
        
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_UP]) {
            // Change particle color based on down arrow key
            // for (int i = 0; i < NUM_PARTICLES; ++i) {
            //     particles[i].color.r -= 0.1f;
            //     particles[i].color.r = glm::clamp(particles[i].color.r, 0.0f, 1.0f);
            // }
            centralDotPosition.y += velocity; // Move up
        }
        if (state[SDL_SCANCODE_DOWN]) {
            // Change particle color based on down arrow key
            // for (int i = 0; i < NUM_PARTICLES; ++i) {
            //     particles[i].color.r -= 0.1f;
            //     particles[i].color.r = glm::clamp(particles[i].color.r, 0.0f, 1.0f);
            // }
            centralDotPosition.y -= velocity; // Move down
        }
        if (state[SDL_SCANCODE_LEFT]) {
            // Change particle color based on left arrow key
            // for (int i = 0; i < NUM_PARTICLES; ++i) {
            //     particles[i].color.g -= 0.1f;
            //     particles[i].color.g = glm::clamp(particles[i].color.g, 0.0f, 1.0f);
            // }
            centralDotPosition.x -= velocity; // Move left
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            // Change particle color based on right arrow key
            // for (int i = 0; i < NUM_PARTICLES; ++i) {
            //     particles[i].color.g += 0.1f;
            //     particles[i].color.g = glm::clamp(particles[i].color.g, 0.0f, 1.0f);
            // }
            centralDotPosition.x += velocity; // Move right
        }
        


	}


}

void updateParticles(float deltaTime) {

//Update Curosr
vertexData[0] = centralDotPosition.x; // Update central dot x position
vertexData[1] = centralDotPosition.y; // Update central dot y position
vertexData[2] = -0.9f;                // Update central dot z position

// Update particle movement
for (int i = 1; i <= particles.size(); i++) {
    Particle &p = particles[i];

    // Update velocity
    p.velocity += p.acceleration * deltaTime;

    // Update position
    p.position += p.velocity * deltaTime;
}

// Update vertex data
for (int i = 1; i <= particles.size(); i++) {
    Particle &p = particles[i];
    int index = 6 * (i);

    vertexData[index] = p.position.x;
    vertexData[index + 1] = p.position.y;
    vertexData[index + 2] = p.position.z;

    // Assuming you have color data in your Particle struct
    vertexData[index + 3] = p.color.r;
    vertexData[index + 4] = p.color.g;
    vertexData[index + 5] = p.color.b;
}

// Upload updated vertex data to GPU
//IDK if this should really be done here
glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);
}


void MainLoop(){
    // Variables to keep track of time
    Uint64 currentTime = SDL_GetPerformanceCounter();
    Uint64 lastTime = 0;
    float deltaTime = 0.0f;

	// While application is running
	while(!gQuit){


        // Calculate deltaTime
        lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();

        deltaTime = (float)((currentTime - lastTime) * 1000 / (float)SDL_GetPerformanceFrequency()); // deltaTime in milliseconds

        // Convert deltaTime to seconds (optional)
        deltaTime /= 1000.0f;

		// Handle Input
		Input();
		// Setup anything (i.e. OpenGL State) that needs to take
		// place before draw calls

        updateParticles(deltaTime);

		PreDraw();
		// Draw Calls in OpenGL
        // When we 'draw' in OpenGL, this activates the graphics pipeline.
        // i.e. when we use glDrawElements or glDrawArrays,
        //      The pipeline that is utilized is whatever 'glUseProgram' is
        //      currently binded.
		Draw();

		//Update screen of our specified window
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}



/**
* The last function called in the program
* This functions responsibility is to destroy any global
* objects in which we have create dmemory.
*
* @return void
*/
void CleanUp(){
	//Destroy our SDL2 Window
	SDL_DestroyWindow(gGraphicsApplicationWindow );
	gGraphicsApplicationWindow = nullptr;

    // Delete our OpenGL Objects
    glDeleteBuffers(1, &gVertexBufferObject);
    glDeleteVertexArrays(1, &gVertexArrayObject);

	// Delete our Graphics pipeline
    glDeleteProgram(gGraphicsPipelineShaderProgram);

	//Quit SDL subsystems
	SDL_Quit();
}


/**
* The entry point into our C++ programs.
*
* @return program status
*/
int main( int argc, char* args[] ){
    std::cout << "Use arrow keys to move the particle\n";
    std::cout << "Press ESC to quit\n";

	// 1. Setup the graphics program
	InitializeProgram();
	
	// 2. Setup our geometry
	VertexSpecification();
	
	// 3. Create our graphics pipeline
	// 	- At a minimum, this means the vertex and fragment shader
	CreateGraphicsPipeline();
	
	// 4. Call the main application loop
	MainLoop();	

	// 5. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}


