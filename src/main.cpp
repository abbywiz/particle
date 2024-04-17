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

// Define window dimensions
const GLint WIDTH = 800, HEIGHT = 600;

bool simulation_running = false;


// Define a structure to represent a particle
struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec3 color;
};

// Define the number of particles
const int NUM_PARTICLES = 100;

int main(int argc, char** argv) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create an SDL window
    SDL_Window* window = SDL_CreateWindow("Particles Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create an OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Load OpenGL function pointers with GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Set up OpenGL viewport
    glViewport(0, 0, WIDTH, HEIGHT);

    // Set up perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    // Initialize particles
    Particle particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        particles[i].position = glm::vec2((rand() % WIDTH) / (float)WIDTH * 2 - 1, (rand() % HEIGHT) / (float)HEIGHT * 2 - 1);
        particles[i].velocity = glm::vec2((rand() % 200 - 100) / 1000.0f, (rand() % 200 - 100) / 1000.0f);
        particles[i].color = glm::vec3((rand() % 255) / 255.0f, (rand() % 255) / 255.0f, (rand() % 255) / 255.0f);
    }

    // Game loop
    bool running = true;
    while (running) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
switch (event.type) {
    case SDL_QUIT:
        running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_SPACE) {
            // Start or stop simulation
            simulation_running = !simulation_running;
        } else if (event.key.keysym.sym == SDLK_UP) {
            // Change particle color based on up arrow key
            for (int i = 0; i < NUM_PARTICLES; ++i) {
                particles[i].color.r += 0.1f;
                particles[i].color.r = glm::clamp(particles[i].color.r, 0.0f, 1.0f);
            }
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            // Change particle color based on down arrow key
            for (int i = 0; i < NUM_PARTICLES; ++i) {
                particles[i].color.r -= 0.1f;
                particles[i].color.r = glm::clamp(particles[i].color.r, 0.0f, 1.0f);
            }
        } else if (event.key.keysym.sym == SDLK_LEFT) {
            // Change particle color based on left arrow key
            for (int i = 0; i < NUM_PARTICLES; ++i) {
                particles[i].color.g -= 0.1f;
                particles[i].color.g = glm::clamp(particles[i].color.g, 0.0f, 1.0f);
            }
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            // Change particle color based on right arrow key
            for (int i = 0; i < NUM_PARTICLES; ++i) {
                particles[i].color.g += 0.1f;
                particles[i].color.g = glm::clamp(particles[i].color.g, 0.0f, 1.0f);
            }
        }
        break;
}

        }

        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render particles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < NUM_PARTICLES; ++i) {
            // Update particle position
            particles[i].position += particles[i].velocity;

            // Set particle color
            glColor3f(particles[i].color.r, particles[i].color.g, particles[i].color.b);

            // Draw triangle representing the particle
            glVertex2f(particles[i].position.x, particles[i].position.y);
            glVertex2f(particles[i].position.x + 0.02f, particles[i].position.y);
            glVertex2f(particles[i].position.x, particles[i].position.y + 0.02f);
        }
        glEnd();

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Clean up
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}