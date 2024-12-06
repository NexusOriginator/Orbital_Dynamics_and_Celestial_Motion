#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime>

// Structure for asteroid
struct Asteroid
{
    glm::vec3 position;
    glm::vec3 velocity;
    float size;
    float rotation;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    size_t indexCount;
};

// Global variables
std::vector<Asteroid> asteroids;
const float SPAWN_RADIUS = 8.0f;
const float MIN_ASTEROID_SIZE = 0.4f;
const float MAX_ASTEROID_SIZE = 0.6f;

// Vertex Shader Source
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 fragTexCoord;
uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(position, 1.0);
    fragTexCoord = texCoord; // Pass texture coordinates to fragment shader
}
)";

// Fragment Shader Source
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 color;

in vec2 fragTexCoord;
uniform sampler2D texture1;

void main() {
    color = texture(texture1, fragTexCoord); // Use the texture color
}
)";

// Simplified asteroid vertex shader
const char *asteroidVertexShader = R"(
    #version 330 core
    layout(location = 0) in vec3 position;
    uniform mat4 mvp;
    void main() {
        gl_Position = mvp * vec4(position, 1.0);
    }
)";

// Simplified asteroid fragment shader with just brown color
const char *asteroidFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.6, 0.4, 0.2, 1.0); // Brown color
    }
)";

float satelliteOrbitRadius = 1.75f;
float satelliteAngle = 0.0f;
float satelliteX = satelliteOrbitRadius;
float satelliteY = 0.0f;

float satelliteOrbitRadius2 = 1.75f;
float satelliteAngle2 = 0.0f;
float satelliteX2 = satelliteOrbitRadius2;
float satelliteY2 = 0.0f;
float satelliteZ2 = 0.0f;
GLuint satelliteVAO2, satelliteVBO2;

// Function to generate sphere vertices and texture coordinates
void generateSphere(float radius, int segments, int rings, std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    for (int y = 0; y <= rings; ++y)
    {
        for (int x = 0; x <= segments; ++x)
        {
            float xSegment = static_cast<float>(x) / segments;
            float ySegment = static_cast<float>(y) / rings;
            float xPos = radius * cos(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);
            float yPos = radius * cos(ySegment * M_PI);
            float zPos = radius * sin(xSegment * 2.0f * M_PI) * sin(ySegment * M_PI);

            // Position
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);

            // Texture coordinates (flip y coordinate)
            vertices.push_back(xSegment);        // S
            vertices.push_back(1.0f - ySegment); // T
        }
    }

    for (int y = 0; y < rings; ++y)
    {
        for (int x = 0; x < segments; ++x)
        {
            int first = (y * (segments + 1)) + x;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

bool checkCollision(const glm::vec3 &satellitePos, float satelliteRadius, const Asteroid &asteroid)
{
    // Calculate distance between centers
    float distance = glm::length(satellitePos - asteroid.position);

    // If distance is less than satellite radius + asteroid size, collision occurred
    return distance < (satelliteRadius + asteroid.size);
}

// Function to generate random star positions
void generateStars(int numStars, std::vector<glm::vec3> &stars)
{
    for (int i = 0; i < numStars; ++i)
    {
        // Random distance from the center (you can adjust the range)
        float distance = 4.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f; // Range from 8 to 10
        // Random initial angle
        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * M_PI; // Random angle in radians

        // Push the generated star position into the vector
        stars.push_back(glm::vec3(distance * cos(angle), distance * sin(angle), 0.0f)); // Using the proper constructor
    }
}

void generateAsteroid(float radius, int sectors, int stacks,
                      std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.15f, 0.15f);

    // Generate vertices
    for (int i = 0; i <= stacks; ++i)
    {
        float phi = M_PI * float(i) / float(stacks);
        for (int j = 0; j <= sectors; ++j)
        {
            float theta = 2.0f * M_PI * float(j) / float(sectors);

            // Base sphere coordinates
            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            // Add random displacement
            float noise = 1.0f + dis(gen);
            x *= radius * noise;
            y *= radius * noise;
            z *= radius * noise;

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal vector for lighting
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            int first = i * (sectors + 1) + j;
            int second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void generateAsteroidMesh(float radius, int sectors, int stacks,
                          std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.15f, 0.15f);

    // Generate vertices
    for (int i = 0; i <= stacks; ++i)
    {
        float phi = M_PI * float(i) / float(stacks);
        for (int j = 0; j <= sectors; ++j)
        {
            float theta = 2.0f * M_PI * float(j) / float(sectors);

            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            // Add random displacement
            float noise = 1.0f + dis(gen);
            x *= radius * noise;
            y *= radius * noise;
            z *= radius * noise;

            // Only position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            int first = i * (sectors + 1) + j;
            int second = first + sectors + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

Asteroid createAsteroid()
{
    Asteroid asteroid;

    // Random spawn position
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
    asteroid.position = glm::vec3(
        SPAWN_RADIUS * cos(angle),
        SPAWN_RADIUS * sin(angle),
        0.0f);

    // Calculate velocity towards center
    glm::vec3 direction = glm::normalize(-asteroid.position);
    float speed = 1.0f + static_cast<float>(rand()) / RAND_MAX * 1.0f;
    asteroid.velocity = direction * speed;

    // Random size and rotation
    asteroid.size = MIN_ASTEROID_SIZE + static_cast<float>(rand()) / RAND_MAX * (MAX_ASTEROID_SIZE - MIN_ASTEROID_SIZE);
    asteroid.rotation = static_cast<float>(rand()) / RAND_MAX * 360.0f;

    // Generate mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateAsteroidMesh(asteroid.size, 16, 8, vertices, indices);
    asteroid.indexCount = indices.size();

    // Create OpenGL buffers
    glGenVertexArrays(1, &asteroid.VAO);
    glGenBuffers(1, &asteroid.VBO);
    glGenBuffers(1, &asteroid.EBO);

    glBindVertexArray(asteroid.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, asteroid.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asteroid.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute only
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    return asteroid;
}
// Function to update asteroids
void updateAsteroids(float deltaTime, const glm::vec3 &satellitePos, float satelliteRadius)
{
    auto it = asteroids.begin();
    while (it != asteroids.end())
    {
        // Update position
        it->position += it->velocity * deltaTime;

        // Update rotation
        it->rotation += 45.0f * deltaTime;

        // Check for collision with satellite
        if (checkCollision(satellitePos, satelliteRadius, *it))
        {
            // Delete asteroid buffers
            glDeleteVertexArrays(1, &it->VAO);
            glDeleteBuffers(1, &it->VBO);
            glDeleteBuffers(1, &it->EBO);
            // Remove asteroid
            it = asteroids.erase(it);
            continue;
        }

        // Check if asteroid should be removed (too close to center or too far)
        float distance = glm::length(it->position);
        if (distance < 1.0f || distance > SPAWN_RADIUS + 2.0f)
        {
            glDeleteVertexArrays(1, &it->VAO);
            glDeleteBuffers(1, &it->VBO);
            glDeleteBuffers(1, &it->EBO);
            it = asteroids.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void renderAsteroid(GLuint shaderProgram, const Asteroid &asteroid,
                    const glm::mat4 &view, const glm::mat4 &projection)
{
    static std::vector<float> asteroidVertices;
    static std::vector<unsigned int> asteroidIndices;
    static GLuint asteroidVAO = 0;
    static GLuint asteroidVBO = 0;
    static GLuint asteroidEBO = 0;

    // Initialize the asteroid mesh only once
    if (asteroidVAO == 0)
    {
        generateAsteroid(1.0f, 16, 8, asteroidVertices, asteroidIndices);

        glGenVertexArrays(1, &asteroidVAO);
        glGenBuffers(1, &asteroidVBO);
        glGenBuffers(1, &asteroidEBO);

        glBindVertexArray(asteroidVAO);

        glBindBuffer(GL_ARRAY_BUFFER, asteroidVBO);
        glBufferData(GL_ARRAY_BUFFER, asteroidVertices.size() * sizeof(float),
                     asteroidVertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asteroidEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, asteroidIndices.size() * sizeof(unsigned int),
                     asteroidIndices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    glUseProgram(shaderProgram);

    // Create transformation matrices
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, asteroid.position);
    model = glm::rotate(model, glm::radians(asteroid.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(asteroid.size / 2));

    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

    // Draw asteroid
    glBindVertexArray(asteroid.VAO);
    glDrawElements(GL_TRIANGLES, asteroid.indexCount, GL_UNSIGNED_INT, 0);
}

// Function to spawn a new asteroid
void spawnAsteroid()
{
    Asteroid asteroid;

    // Random angle for spawn position
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;

    // Set random position on circle
    asteroid.position = glm::vec3(
        SPAWN_RADIUS * cos(angle),
        SPAWN_RADIUS * sin(angle),
        0.0f);

    // Calculate velocity vector towards center
    glm::vec3 direction = glm::normalize(-asteroid.position);
    float speed = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
    asteroid.velocity = direction * speed;

    // Random size and rotation
    asteroid.size = MIN_ASTEROID_SIZE +
                    static_cast<float>(rand()) / RAND_MAX * (MAX_ASTEROID_SIZE - MIN_ASTEROID_SIZE);
    asteroid.rotation = static_cast<float>(rand()) / RAND_MAX * 360.0f;

    // Generate mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateAsteroidMesh(asteroid.size, 16, 8, vertices, indices);
    asteroid.indexCount = indices.size();

    // Create OpenGL buffers
    glGenVertexArrays(1, &asteroid.VAO);
    glGenBuffers(1, &asteroid.VBO);
    glGenBuffers(1, &asteroid.EBO);

    glBindVertexArray(asteroid.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, asteroid.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asteroid.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Add asteroid to the vector
    asteroids.push_back(asteroid);
}

// Function to compile shaders
GLuint compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    return shader;
}

// Function to create sphere vertices
std::vector<float> createSphereVertices(float radius, int sectors, int stacks)
{
    std::vector<float> vertices;

    for (int i = 0; i <= stacks; ++i)
    {
        float stackAngle = M_PI / 2 - i * M_PI / stacks; // from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);            // radius at current stack
        float z = radius * sinf(stackAngle);             // z coordinate

        for (int j = 0; j <= sectors; ++j)
        {
            float sectorAngle = j * 2 * M_PI / sectors; // from 0 to 2pi
            float x = xy * cosf(sectorAngle);           // x coordinate
            float y = xy * sinf(sectorAngle);           // y coordinate
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Indices for drawing triangles
    std::vector<unsigned int> indices;
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < sectors; ++j)
        {
            unsigned int first = (i * (sectors + 1)) + j;
            unsigned int second = first + sectors + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Convert indices to a flat vertex list
    std::vector<float> indexedVertices;
    for (const auto &index : indices)
    {
        indexedVertices.push_back(vertices[index * 3]);
        indexedVertices.push_back(vertices[index * 3 + 1]);
        indexedVertices.push_back(vertices[index * 3 + 2]);
    }
    return indexedVertices;
}

// Function to load texture
GLuint loadTexture(const char *path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Load image using stb_image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        return -1;
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    // Set GLFW context version (OpenGL 3.3)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Textured Sphere with Stars", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        return -1;
    }

    // Set viewport
    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST); // Enable depth testing

    // Generate sphere vertices and indices
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateSphere(1.0f, 64, 32, vertices, indices);

    // Create Vertex Array Object and Vertex Buffer Objects
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Define the vertex data layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0); // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float))); // Texture coordinates
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    std::vector<float> satelliteVertices = createSphereVertices(0.08f, 32, 16);

    // Cleanup shaders as they are now linked
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint asteroidVertShader = compileShader(GL_VERTEX_SHADER, asteroidVertexShader);
    GLuint asteroidFragShader = compileShader(GL_FRAGMENT_SHADER, asteroidFragmentShader);

    GLuint asteroidShaderProgram = glCreateProgram();
    glAttachShader(asteroidShaderProgram, asteroidVertShader);
    glAttachShader(asteroidShaderProgram, asteroidFragShader);
    glLinkProgram(asteroidShaderProgram);

    glDeleteShader(asteroidVertShader);
    glDeleteShader(asteroidFragShader);

    // Load texture
    GLuint satelliteTexture = loadTexture("satellite_texture.jpg");
    GLuint moonTexture = loadTexture("moon_texture.jpg");
    GLuint earthTexture = loadTexture("earth_texture.jpg"); // Ensure you have the Earth texture image in the same directory

    unsigned int satelliteVAO1, satelliteVBO1;
    glGenVertexArrays(1, &satelliteVAO1);
    glGenBuffers(1, &satelliteVBO1);
    glBindVertexArray(satelliteVAO1);
    glBindBuffer(GL_ARRAY_BUFFER, satelliteVBO1);
    glBufferData(GL_ARRAY_BUFFER, satelliteVertices.size() * sizeof(float), satelliteVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // After creating first satellite's VAO and VBO
    glGenVertexArrays(1, &satelliteVAO2);
    glGenBuffers(1, &satelliteVBO2);

    glBindVertexArray(satelliteVAO2);

    glBindBuffer(GL_ARRAY_BUFFER, satelliteVBO2);
    glBufferData(GL_ARRAY_BUFFER, satelliteVertices.size() * sizeof(float), satelliteVertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize timing variables
    float currentTime = glfwGetTime();
    float lastTime = currentTime;
    float lastSpawnTime = currentTime;
    float spawnInterval = 2.0f;

    // Generate stars with their original positions
    std::vector<glm::vec3> stars;
    glPointSize(8.0f);
    generateStars(1000, stars); // Generate 300 stars within a range of 10.0 units

    // Main loop
    while (!glfwWindowShouldClose(window))
    {

        currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Update spawn timer
        if (currentTime - lastSpawnTime >= spawnInterval)
        {
            spawnAsteroid();
            lastSpawnTime = currentTime;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(shaderProgram);

        // Create transformation matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 mvp = projection * view * model;

        // Set the MVP uniform
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // Use texture unit 0

        // Draw the sphere
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Satellite
        model = glm::mat4(1.0f);

        // Satellite movement logic
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            satelliteAngle += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            satelliteAngle -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            satelliteOrbitRadius += 0.01f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            satelliteOrbitRadius -= 0.01f;

        // Clamp the orbit radius
        if (satelliteOrbitRadius < 0.1f)
            satelliteOrbitRadius = 0.1f;

        // Position of the satellite
        glm::mat4 satelliteModel = glm::mat4(1.0f);
        satelliteX = satelliteOrbitRadius * cos(satelliteAngle);
        satelliteY = satelliteOrbitRadius * sin(satelliteAngle);
        // satelliteModel = glm::translate(satelliteModel, glm::vec3(0.0f, satelliteY, satelliteX));

        // Position of the satellite
        // model = glm::translate(model, glm::vec3(
        //                                   satelliteOrbitRadius * cos(satelliteAngle),
        //                                   0.0f,
        //                                   satelliteOrbitRadius * sin(satelliteAngle)));
        model = glm::translate(glm::mat4(1.0f), glm::vec3(satelliteX, satelliteY, 0.0f));
        float satelliteScale = 2.0f; // Makes satellite 3x larger
        // satelliteModel = glm::scale(satelliteModel, glm::vec3(satelliteScale));
        model = glm::scale(model, glm::vec3(satelliteScale));

        // Set the MVP matrix for the satellite (projection * view * model)
        mvp = projection * view * model;

        // Set the MVP uniform for the satellite
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f);
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, satelliteTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // Use texture unit 0
        // Draw the satellite (bind its VAO and draw)
        glBindVertexArray(satelliteVAO1);
        glDrawArrays(GL_TRIANGLES, 0, satelliteVertices.size());

        glm::vec3 satellitePos(satelliteX, satelliteY, 0.0f);
        float satelliteRadius = 0.08f;

        updateAsteroids(deltaTime, satellitePos, satelliteRadius);

        // Reduced speed for tilt oscillation (from 10.0f to 2.0f)
        float tiltAngle = glm::radians(45.0f + 2.0f * sin(glfwGetTime()));

        // Calculate position with tilt
        satelliteX2 = satelliteOrbitRadius2 * cos(satelliteAngle2);
        satelliteY2 = satelliteOrbitRadius2 * sin(satelliteAngle2) * sin(tiltAngle);
        satelliteZ2 = satelliteOrbitRadius2 * sin(satelliteAngle2) * cos(tiltAngle);

        // Reduced orbital rotation speed (from 0.01f to 0.003f)
        satelliteAngle2 += 0.01f;

        // Create moon satellite's model matrix with tilted orbit
        glm::mat4 satelliteModel2 = glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(satelliteX2, satelliteY2, satelliteZ2));
        float moonScale = 4.0f; // Makes the moon 3 times larger
        satelliteModel2 = glm::scale(satelliteModel2,
                                     glm::vec3(moonScale)); // Uniform scaling in all directions
        mvp = projection * view * satelliteModel2;

        // Update MVP uniform for moon
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"),
                           1, GL_FALSE, glm::value_ptr(mvp));
        // Bind Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // Use texture unit 0

        // Draw moon satellite
        glBindVertexArray(satelliteVAO2);
        glDrawArrays(GL_TRIANGLES, 0, satelliteVertices.size());

        float starDistance = 3.0f; // Adjust star distance if necessary
        for (size_t i = 0; i < stars.size(); ++i)
        {
            // Calculate angle for revolution
            float angle = glfwGetTime() + (i * (2.0f * M_PI / stars.size())); // Offset each star's angle

            // Position stars in a circular path around the sphere
            float x = starDistance * cos(angle);      // Circular motion on x-axis
            float z = starDistance * sin(angle);      // Circular motion on z-axis
            glm::vec3 starPosition(x, stars[i].y, z); // Keep the original y position of the star

            // Create the star model matrix
            glm::mat4 starModel = glm::translate(glm::mat4(5.0f), starPosition);
            starModel = glm::scale(starModel, glm::vec3(10.0f)); // Scale stars down
            glm::mat4 starMVP = projection * view * starModel;

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvp"), 1, GL_FALSE, glm::value_ptr(starMVP));

            // Draw a point for the star
            glDrawArrays(GL_POINTS, 0, 1); // Drawing a single point
        }

        for (const auto &asteroid : asteroids)
        {
            renderAsteroid(asteroidShaderProgram, asteroid, view, projection);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &satelliteVBO1);
    glDeleteBuffers(1, &satelliteVBO2);
    glDeleteVertexArrays(1, &satelliteVAO1);
    glDeleteVertexArrays(1, &satelliteVAO2);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}