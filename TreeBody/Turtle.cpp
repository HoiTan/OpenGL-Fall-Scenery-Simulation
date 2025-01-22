#include "Turtle.hpp"
#include <stack>
#include "/Users/hoimautan/Desktop/CS/CS450/SampleMac/glm/gtc/matrix_transform.hpp"
#include "/Users/hoimautan/Desktop/CS/CS450/SampleMac/glm/gtc/constants.hpp"
#include "/Users/hoimautan/Desktop/CS/CS450/SampleMac/glm/gtc/type_ptr.hpp"

// Include OpenGL headers
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <random>
#include <functional>
#endif

#include "/Users/hoimautan/Desktop/CS/CS450/SampleMac/glut.h"

Turtle::Turtle() 
    : m_angleIncrement(glm::radians(25.0f)), m_stepLength(1.0f)
{
    m_state.position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_state.yAxis = glm::vec3(0.0f, 1.0f, 0.0f); // Y-axis as heading
    m_state.zAxis = glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis as up
    m_state.xAxis = glm::vec3(1.0f, 0.0f, 0.0f); // X-axis as right
}

void Turtle::setAngle(float angleDegrees) {
    m_angleIncrement = glm::radians(angleDegrees);
}

void Turtle::setStep(float stepLength) {
    m_stepLength = stepLength;
}

void Turtle::setRadius(float radius) {
    m_initialRadius = radius;
}

void Turtle::setTaperFactor(float taperFactor) {
    m_taperFactor = taperFactor;
}

void Turtle::setInitialFactor(float angleDegrees, float stepLength, float radius, float taperFactor) {
    m_angleIncrement = glm::radians(angleDegrees);
    m_stepLength = stepLength;
    m_initialRadius = radius;
    m_taperFactor = taperFactor;
}

std::vector<Turtle::Leaf> Turtle::GetLeaves() const {
    return leafPositions; // Returns a copy of the leafPositions vector
}

int Turtle::quantize(float value, float scale = 100.0f) {
    return static_cast<int>(std::round(value * scale));
}


unsigned int Turtle::generateSeed(const glm::vec3& position) {
    int qx = quantize(position.x);
    int qy = quantize(position.y);
    int qz = quantize(position.z);
    
    // Combine the quantized components using a hash function
    std::hash<int> hasher;
    size_t seed = hasher(qx);
    seed ^= hasher(qy) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hasher(qz) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    
    return static_cast<unsigned int>(seed);
}

void Turtle::interpret(const std::string &lsystemString) {
    std::stack<TurtleState> stateStack;
    std::stack<float> radiusStack;

    // Define initial branch radius and taper factor
    float initialRadius = 0.5f;
    float taperFactor = 0.7f;
    radiusStack.push(initialRadius);

    std::uniform_real_distribution<float> posOffsetDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> rotAngleDist(-30.0f, 30.0f); // Degrees
    
    for (auto c : lsystemString) {
        switch (c) {
            case 'F': {
                // Current branch radius
                float currentRadius = radiusStack.top();

                // Current position and orientation
                glm::vec3 start = m_state.position;
                glm::vec3 end = start + m_state.yAxis * m_stepLength;
                
                // Set branch color based on radius (optional)
                float colorFactor = currentRadius / initialRadius;
                glColor3f(0.55f * colorFactor, 0.27f * colorFactor, 0.07f * colorFactor); // Brownish color
                
                // Draw the branch as a cylinder
                drawCylinder(start, end, currentRadius);
                
                // Move to the end position
                m_state.position = end;
            } break;

            case '+':
                // Rotate +angle about Z-axis (yaw left)
                rotate(m_state.yAxis, m_state.zAxis, m_angleIncrement);
                rotate(m_state.xAxis, m_state.zAxis, m_angleIncrement);
                break;

            case '-':
                // Rotate -angle about Z-axis (yaw right)
                rotate(m_state.yAxis, m_state.zAxis, -m_angleIncrement);
                rotate(m_state.xAxis, m_state.zAxis, -m_angleIncrement);
                break;

            case '<':
                // Rotate +angle about Y-axis (roll left)
                rotate(m_state.zAxis, m_state.yAxis, m_angleIncrement);
                rotate(m_state.xAxis, m_state.yAxis, m_angleIncrement);
                break;

            case '>':
                // Rotate -angle about Y-axis (roll right)
                rotate(m_state.zAxis, m_state.yAxis, -m_angleIncrement);
                rotate(m_state.xAxis, m_state.yAxis, -m_angleIncrement);
                break;

            case 'v':
                // Rotate +angle about X-axis (pitch down)
                rotate(m_state.yAxis, m_state.xAxis, m_angleIncrement);
                rotate(m_state.zAxis, m_state.xAxis, m_angleIncrement);
                break;

            case '^':
                // Rotate -angle about X-axis (pitch up)
                rotate(m_state.yAxis, m_state.xAxis, -m_angleIncrement);
                rotate(m_state.zAxis, m_state.xAxis, -m_angleIncrement);
                break;

            case '[':
                // Push current state and radius
                stateStack.push(m_state);
                radiusStack.push(radiusStack.top() * taperFactor); // Tapering
                break;

            case ']':
                // Pop state and radius
                if (!stateStack.empty()) {
                    m_state = stateStack.top();
                    stateStack.pop();
                }
                if (!radiusStack.empty()) {
                    radiusStack.pop();
                }
                break;

            case 'L': {
                // Record leaf position and orientation
                Leaf leaf;
                leaf.position = m_state.position;

                unsigned int seed = generateSeed(leaf.position);
                std::mt19937 generator(seed); // Mersenne Twister PRNG
                
                // Apply random positional offsets
                float offsetX = posOffsetDist(generator);
                float offsetY = posOffsetDist(generator);
                float offsetZ = posOffsetDist(generator);
                leaf.position += glm::vec3(offsetX, offsetY, offsetZ);
                
                // Apply random rotations to the orientation vectors
                float rotX = rotAngleDist(generator);
                float rotY = rotAngleDist(generator);
                float rotZ = rotAngleDist(generator);
                
                // Create rotation matrices
                glm::mat4 rotMatrixX = glm::rotate(glm::mat4(1.0f), glm::radians(rotX), m_state.xAxis);
                glm::mat4 rotMatrixY = glm::rotate(glm::mat4(1.0f), glm::radians(rotY), m_state.yAxis);
                glm::mat4 rotMatrixZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotZ), m_state.zAxis);
                
                // Apply rotations to the orientation vectors
                glm::vec4 newUp = rotMatrixX * glm::vec4(m_state.zAxis, 0.0f);
                glm::vec4 newRight = rotMatrixY * glm::vec4(m_state.xAxis, 0.0f);
                glm::vec4 newForward = rotMatrixZ * glm::vec4(m_state.yAxis, 0.0f);
                
                leaf.up = glm::normalize(glm::vec3(newUp));
                leaf.right = glm::normalize(glm::vec3(newRight));
                leafPositions.push_back(leaf);
                break;
            }

            default:
                // Ignore other symbols
                break;
        }

        // Re-orthogonalize the coordinate frame to prevent drift
        m_state.yAxis = glm::normalize(m_state.yAxis);
        m_state.zAxis = glm::normalize(m_state.zAxis);
        m_state.xAxis = glm::normalize(glm::cross(m_state.yAxis, m_state.zAxis));
        m_state.zAxis = glm::normalize(glm::cross(m_state.xAxis, m_state.yAxis));
    }
}

void Turtle::rotate(glm::vec3 &dir, const glm::vec3 &axis, float angle) {
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, axis);
    glm::vec4 newDir = rot * glm::vec4(dir, 0.0f);
    dir = glm::normalize(glm::vec3(newDir));
}


// Function to draw a cylinder between two points
void Turtle::drawCylinder(const glm::vec3& start, const glm::vec3& end, float radius) {
    // Calculate the direction vector from start to end
    glm::vec3 direction = end - start;
    float height = glm::length(direction);
    if (height == 0.0f) return; // Prevent division by zero
    
    // Normalize the direction
    glm::vec3 normDirection = glm::normalize(direction);
    
    // Default cylinder is aligned along the Z-axis
    glm::vec3 up(0.0f, 0.0f, 1.0f);
    
    // Calculate the rotation axis and angle
    glm::vec3 rotationAxis = glm::cross(up, normDirection);
    float dotProduct = glm::dot(up, normDirection);
    
    // Handle the case when direction is parallel to up vector
    float angle;
    if (glm::length(rotationAxis) < 1e-6) {
        rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f); // Arbitrary axis
        angle = (dotProduct > 0) ? 0.0f : 180.0f;
    } else {
        angle = glm::degrees(acos(dotProduct));
    }
    
    // Save the current matrix
    glPushMatrix();
    
    // Translate to the start position
    glTranslatef(start.x, start.y, start.z);
    
    // Rotate the cylinder to align with the direction vector
    glRotatef(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
    
    // Create a quadric object for the cylinder
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    
    // Draw the cylinder (base radius, top radius, height, slices, stacks)
    gluCylinder(quad, radius, radius * 0.7f, height, 12, 3); // Tapered cylinder
    
    // Optional: Cap the cylinder at the end
    gluDisk(quad, 0.0f, radius * 0.7f, 12, 1);
    
    // Delete the quadric object
    gluDeleteQuadric(quad);
    
    // Restore the previous matrix
    glPopMatrix();
}
