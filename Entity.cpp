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
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_velocity(0.0f), m_accleration(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_rotate(0.0f), m_rotate_speed(0.05f), m_entityType(BACKGROUND), m_height(0.0f), m_width(0.0f){}

// Parameterized constructor
Entity::Entity(GLuint texture_id, float speed, float animation_time,
    int animation_frames, int animation_index, int animation_cols,
    int animation_rows, EntityType entityType, float height, float width)
    : m_position(0.0f), m_movement(0.0f), m_velocity(0.0f), m_accleration(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(animation_cols),
    m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr),
    m_animation_time(animation_time), m_texture_id(texture_id), m_rotate(0.0f), m_rotate_speed(0.05f), m_entityType(entityType), m_height(height), m_width(width) {}

// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed, EntityType entityType)
    : m_position(0.0f), m_movement(0.0f), m_velocity(0.0f), m_accleration(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_rotate(0.0f), m_rotate_speed(0.05f), m_entityType(entityType), m_height(0.0f), m_width(0.0f) {}

Entity::~Entity() { }

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index, int cols, int rows)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % cols) / (float)cols;
    float v_coord = (float)(index / cols) / (float)rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)cols;
    float height = 1.0f / (float)rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::update(float delta_time, Entity** array, int arraySize, int& gameWon)
{
    if (m_entityType == SHIP) {
        //if the game is over just stop moving
        if (gameWon != 0) {
            m_velocity.x = 0.0f;
            m_velocity.y = 0.0f;
            m_model_matrix = glm::mat4(1.0f);
            m_model_matrix = glm::translate(m_model_matrix, m_position);
            m_model_matrix = glm::rotate(m_model_matrix, m_rotate * m_rotate_speed * delta_time, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate on z axis
            m_model_matrix = glm::scale(m_model_matrix, m_scale);
        }
        else {
            //reset velocity
            m_velocity.x = 0.0f;
            m_velocity.y = 0.0f;

            //add in accleration
            float gravity = -9.8 * 0.01f;
            m_accleration.y += gravity; //add gravity to vertical accleration
            m_velocity += m_accleration * delta_time;

            //change position based on velocity and delta time
            m_position += m_velocity * delta_time;

            m_model_matrix = glm::mat4(1.0f);
            m_model_matrix = glm::translate(m_model_matrix, m_position);
            m_model_matrix = glm::rotate(m_model_matrix, m_rotate * m_rotate_speed * delta_time, glm::vec3(0.0f, 0.0f, 1.0f)); //rotate on z axis
            m_model_matrix = glm::scale(m_model_matrix, m_scale);

            //check for collision
            for (int ind = 0; ind < arraySize; ind++) {
                float x_distance = fabs(m_position.x - array[ind]->m_position.x) - ((m_width + array[ind]->m_width) / 2.0f);
                float y_distance = fabs(m_position.y - array[ind]->m_position.y) - ((m_height + array[ind]->m_height) / 2.0f);

                if (x_distance < 0.0f && y_distance < 0.0f) { //we have a collision
                    m_velocity.x = 0;
                    m_velocity.y = 0;
                    //check what type of entity we are colliding with
                    if (array[ind]->m_entityType == MOON) {//we won
                        gameWon = 1;
                    }
                    else {//we lost
                        gameWon = 2;
                    }
                }
            }
            //check if we are out of bounds of the screen
            if (m_position.x > 5.0f || m_position.x < -5.0f || m_position.y > 5.0f || m_position.y < -5.0f) {
                gameWon = 2;
            }
        }
        
    }
    else if (m_entityType == BACKGROUND){
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        m_model_matrix = glm::scale(m_model_matrix, m_scale);
    }
    else if (m_entityType == MOON) { 
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        m_model_matrix = glm::scale(m_model_matrix, m_scale);
    }
    else if (m_entityType == ASTEROID) { 
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        m_model_matrix = glm::scale(m_model_matrix, m_scale);
    }

   

}

void Entity::render(ShaderProgram* program, int index, int cols, int rows)
{
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id,
            m_animation_indices[m_animation_index], m_animation_cols, m_animation_rows);
        return;
    }
    else {
        draw_sprite_from_texture_atlas(program, m_texture_id, index, cols, rows);
    }


}

void Entity::turn_right() {
    //only change rotate if within range
    if (m_rotate < 1880.0f) {
        m_rotate += 1.0f;
    }
}
void Entity::turn_left() {
    //only change rotate if within range
    if (m_rotate > -1880.0f) {
        m_rotate -= 1.0f;

    }

}
void Entity::acclerate() {
    //change our accleration based on our current rotation
    if (m_rotate >= 1880.0f) { //we are completely horizontal 
        //only change horizontal accleration
        m_accleration.x = -9.0f;

    }
    else if (m_rotate <= -1880.0f) { //we are completely horizontal
        //only change horizontal accleration
        //m_accleration.x = 9.0f;
        m_accleration.x = 9.0f;

    }
    else if (m_rotate == 0) {//we are completely vertical 
        //only change vertical accleration
        m_accleration.y = 9.0f;
    }
    else if (m_rotate <0) { 
        m_accleration.x = 9.0f * fabs(cos(m_rotate)); 
        m_accleration.y = 9.0f * fabs(sin(m_rotate)); //the player can only acclerate positively
    }
    else { //m_rotate > 0
        m_accleration.x = -9.0f * fabs(cos(m_rotate));
        m_accleration.y = 9.0f * fabs(sin(m_rotate)); //the player can only acclerate 
    }
}
