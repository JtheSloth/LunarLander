/**
* Author: Jemima Datus
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

enum EntityType { SHIP, BACKGROUND, MOON, ASTEROID };
class Entity
{
private:

    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_accleration;
    glm::vec3 m_scale;
    float m_rotate;
    float m_rotate_speed;
    float m_height;
    float m_width;

    glm::mat4 m_model_matrix;

    float     m_speed;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ----- EntityType ----//
    EntityType m_entityType;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, float animation_time,
        int animation_frames, int animation_index, int animation_cols,
        int animation_rows, EntityType entityType, float height, float width);
    Entity(GLuint texture_id, float speed, EntityType entityType); // Simpler constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index, int cols, int rows);
    void update(float delta_time, Entity** array, int arraySize, int& gameWon);
    void render(ShaderProgram* program, int index, int cols, int rows);

    void normalise_movement() { m_movement = glm::normalize(m_movement); };

    //my new methods
    void turn_left();
    void turn_right();
    void acclerate();

    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position; }
    glm::vec3 const get_movement()   const { return m_movement; }
    glm::vec3 const get_velocity()   const { return m_velocity; }
    glm::vec3 const get_accleration()   const { return m_accleration; }
    glm::vec3 const get_scale()      const { return m_scale; }
    GLuint    const get_texture_id() const { return m_texture_id; }
    float     const get_speed()      const { return m_speed; }

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_accleration(glm::vec3 new_accleration) { m_accleration = new_accleration; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(int new_time) { m_animation_time = new_time; }
    void const set_width(int new_width) { m_width = new_width; }
    void const set_height(int new_height) { m_height = new_height; }

};