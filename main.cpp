/**
* Author: Jemima Datus
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION //need for textures


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               // We'll talk about these later in the course
//include for loading textures
#include "stb_image.h"
#include "Entity.h"
#include <vector>
#include <string>

enum AppStatus { RUNNING, TERMINATED };

// Our window dimensions
constexpr int WINDOW_WIDTH = 640 * 1.5f,
WINDOW_HEIGHT = 480 * 1.5f;

// Background color components
constexpr float BG_RED = 1.0f,
BG_BLUE = 1.0f,
BG_GREEN = 1.0f,
BG_OPACITY = 1.0f;

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture Shaders
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

//variables for matrices
glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix;  

//texture id variable
GLuint g_fontSheetTextureID;

//variables for sprite textures
constexpr char g_ship[] = "ship.png";
constexpr char g_fontSheet[] = "font1.png";
constexpr char g_background[] = "background.png";
constexpr char g_moon[] = "moon.png";
constexpr char g_asteroid[] = "asteroid.png";

//accerlation flag for changing the picture
bool g_accerlation = false;

//flag for winning the game
int g_won = 0; // 0 means game is still going, 1 means we won gamee, 2 means we lost game

//fuel value
int g_fuel = 60000;

//variables for load_texture
constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

//variable for drawing text
constexpr int fontBankSize = 16;

//number of colliables
constexpr int NUM_COLLIABLES = 5;

//Game state Struct
struct GameState{
    Entity* ship;
    Entity* background;
    Entity* colliables[NUM_COLLIABLES];
};

//game state variable 
GameState g_gameState;

//variable for delta time and fixed time step stuff
float g_prevTicks = 0.0f;
float g_fixed_time_step = 1 / 60.0f;
float g_time_accumulator = 0.0f;

//load texture function
GLuint loadTexture(const char* filePath) {

    int width, height, numberOfComponents;
    unsigned char* image = stbi_load(filePath, &width, &height, &numberOfComponents, STBI_rgb_alpha);

    //show error message if the image was not loaded
    if (image == NULL) {
        std::cerr << "Unable to load image. Make sure the path is correct.";
        assert(false);
    }

    //generating a textureID and binding it to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //free up memory allocated for image file data
    stbi_image_free(image);

    return textureID;

}
//draw text function
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / fontBankSize;
    float height = 1.0f / fontBankSize;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % fontBankSize) / fontBankSize;
        float v_coordinate = (float)(spritesheet_index / fontBankSize) / fontBankSize;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Land on Moons and Avoid Asteroids",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    if (g_display_window == nullptr)
    {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;

        SDL_Quit();
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Initialise our camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Load up our shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialise our view, model, and projection matrices
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_model_matrix = glm::mat4(1.0f);  // Defines every translation, rotations, or scaling applied to an object
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    //create ship entity using partial constructor 
    g_gameState.ship = new Entity(loadTexture(g_ship), 1.0f, SHIP);
    g_gameState.ship->set_scale(glm::vec3(0.78f, 0.78f, 0.0f));
    g_gameState.ship->set_height(0.78);
    g_gameState.ship->set_width(0.78);

    //load font sheet texture id
    g_fontSheetTextureID = loadTexture(g_fontSheet);

    //create mountain entity using partial constructor
    g_gameState.background = new Entity(loadTexture(g_background), 1.0f, BACKGROUND);
    g_gameState.background->set_scale(glm::vec3(10.0f, 7.5f, 0.0f));

    //create moon entities
    g_gameState.colliables[0] = new Entity(loadTexture(g_moon), 1.0f, MOON);
    g_gameState.colliables[0]->set_position(glm::vec3(-2.0f, 0.0f, 0.0f));
    g_gameState.colliables[0]->set_height(1.0f);
    g_gameState.colliables[0]->set_width(1.0f);
    g_gameState.colliables[1] = new Entity(loadTexture(g_moon), 1.0f, MOON);
    g_gameState.colliables[1]->set_position(glm::vec3(3.0f, 2.0f, 0.0f));
    g_gameState.colliables[1]->set_height(1.0f);
    g_gameState.colliables[1]->set_width(1.0f);

    //create asteroid entities
    g_gameState.colliables[2] = new Entity(loadTexture(g_asteroid), 1.0f, ASTEROID);
    g_gameState.colliables[2]->set_position(glm::vec3(-1.0f, 1.0f, 0.0f));
    g_gameState.colliables[2]->set_height(1.0f);
    g_gameState.colliables[2]->set_width(1.0f);
    g_gameState.colliables[3] = new Entity(loadTexture(g_asteroid), 1.0f, ASTEROID);
    g_gameState.colliables[3]->set_position(glm::vec3(1.0f, -2.0f, 0.0f));
    g_gameState.colliables[3]->set_height(1.0f);
    g_gameState.colliables[3]->set_width(1.0f);

    g_gameState.colliables[4] = new Entity(loadTexture(g_asteroid), 1.0f, ASTEROID);
    g_gameState.colliables[4]->set_position(glm::vec3(2.0f, 1.0f, 0.0f));
    g_gameState.colliables[4]->set_height(1.0f);
    g_gameState.colliables[4]->set_width(1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    // Each object has its own unique ID
    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }

    //get keyboard state
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    if (keyState[SDL_SCANCODE_LEFT] && g_won == 0) { //if they press left rotate left
        g_gameState.ship->turn_left();
        //if they still have fuel accerlate 
        if (g_fuel > 0) {
            //set accerlation flag to true
            g_accerlation = true;
            //modify fuel value
            g_fuel -= 1;
            if (g_fuel < 0) {
                g_fuel = 0;
            }
            //acclerate
            g_gameState.ship->acclerate();
        }
    }
    if (keyState[SDL_SCANCODE_RIGHT] && g_won == 0) { //if they press right rotate right
        g_gameState.ship->turn_right();
        //if they still have fuel accerlate 
        if (g_fuel > 0) {
            //set accerlation flag to true
            g_accerlation = true;
            //modify fuel value
            g_fuel -= 1;
            if (g_fuel < 0) {
                g_fuel = 0;
            }
            //acclerate
            g_gameState.ship->acclerate();
        }
    }
    if (!keyState[SDL_SCANCODE_RIGHT] && !keyState[SDL_SCANCODE_LEFT]) { //if they aren't pressing right or left
        g_accerlation = false;
    }
}

void update() { 
    //delta time calculations
    float newTick = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = newTick - g_prevTicks;
    g_prevTicks = newTick;

    deltaTime += g_time_accumulator;

    if (deltaTime < g_fixed_time_step) {
        g_time_accumulator = deltaTime;
        return;
    }
    while (deltaTime >= g_fixed_time_step) {

        g_gameState.ship->update(g_fixed_time_step, g_gameState.colliables, NUM_COLLIABLES, g_won);
        g_gameState.background->update(g_fixed_time_step, g_gameState.colliables, NUM_COLLIABLES, g_won);
        for (int ind = 0; ind < NUM_COLLIABLES; ind++) {
            g_gameState.colliables[ind]->update(g_fixed_time_step, g_gameState.colliables, NUM_COLLIABLES, g_won);
        }
        deltaTime -= g_fixed_time_step;
    }
    g_time_accumulator = deltaTime;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    g_shader_program.set_model_matrix(g_model_matrix);

    //render mountains
    g_gameState.background->render(&g_shader_program, 0, 1, 1);

    //render the ship
    if (g_accerlation) { //render ship with flame
        g_gameState.ship->render(&g_shader_program, 1, 4, 1);
    }
    else { //render ship without flame
        g_gameState.ship->render(&g_shader_program, 0, 4, 1);
    }

    //render the fuel amount
    std::string fuelMessage = "Fuel:"+ std::to_string(g_fuel);
    draw_text(&g_shader_program, g_fontSheetTextureID, fuelMessage, 0.5f, 0.001f, glm::vec3(-4.0f, 3.0f, 0.0f));

    //render colliables
    for (int ind = 0; ind < NUM_COLLIABLES; ind++) {
        g_gameState.colliables[ind]->render(&g_shader_program, 0, 1, 1);
    }
    if (g_won == 1) {
        draw_text(&g_shader_program, g_fontSheetTextureID, "YOU WON", 0.5f, 0.001f, glm::vec3(0.0f, 0.0f, 0.0f));
    }
    if (g_won == 2) {
        draw_text(&g_shader_program, g_fontSheetTextureID, "YOU LOST", 0.5f, 0.001f, glm::vec3(0.0f, 0.0f, 0.0f));
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { 
    SDL_Quit(); 
    delete g_gameState.ship; //clear ship from heap
    delete g_gameState.background; //clear background from heap
    //delete all colliable objects from the heap
    for (int ind = 0; ind < NUM_COLLIABLES; ind++) {
        delete g_gameState.colliables[ind];
    }
}

int main(int argc, char* argv[])
{
    // Initialise our program—whatever that means
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();  // If the player did anything—press a button, move the joystick—process it
        update();         // Using the game's previous state, and whatever new input we have, update the game's state
        render();         // Once updated, render those changes onto the screen
    }

    shutdown();  // The game is over, so let's perform any shutdown protocols
    return 0;
}