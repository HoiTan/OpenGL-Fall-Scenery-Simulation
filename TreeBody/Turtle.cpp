#include "Turtle.hpp"
#include <stdexcept>    // for std::invalid_argument
#include <cstdlib>      // for std::stof (or std::stod)
#include <iostream>
#include <cctype>       // for std::isdigit, std::isalpha
#include <cmath>
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/constants.hpp"
#include "../glm/gtc/type_ptr.hpp"

// Include OpenGL headers
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <functional>
#endif

#include "../glut.h"

// Define the static member
std::mt19937 Turtle::gGlobalRNG = std::mt19937(
    static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count())
);
// -------------------------------------
// Constructor
// -------------------------------------
Turtle::Turtle() 
    : m_angleIncrement(glm::radians(25.0f))
    , m_stepLength(1.0f)
{
    m_state.position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_state.yAxis    = glm::vec3(0.0f, 1.0f, 0.0f); // heading
    m_state.zAxis    = glm::vec3(0.0f, 0.0f, 1.0f); // up
    m_state.xAxis    = glm::vec3(1.0f, 0.0f, 0.0f); // right
}

// -------------------------------------
// Set tropism vector & coefficient
// -------------------------------------
void Turtle::setTropismVector(const glm::vec3& tropism) {
    m_tropismVector = tropism;
}
void Turtle::setTropismCoefficient(float coeff) {
    m_tropismCoefficient = coeff;
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
// ---------------------------------------------------------
// Helper: Parse a float in parentheses, e.g. if the string
// has "F(123.45)", after reading 'F' we parse "(123.45)".
// 'pos' should point to the '(' when this function is called.
// ---------------------------------------------------------
float Turtle::parseFloatParameter(const std::string &str, size_t &pos) const
{
    // We expect str[pos] == '('
    // Move past '('
    if (pos < str.size() && str[pos] == '(') {
        pos++;
    } else {
        return 0.0f; // no '(' => no param
    }

    // Collect everything until the next ')'
    std::string numberStr;
    while (pos < str.size() && str[pos] != ')') {
        numberStr.push_back(str[pos]);
        pos++;
    }

    // Move past ')'
    if (pos < str.size() && str[pos] == ')') {
        pos++;
    }

    // Convert to float
    try {
        return std::stof(numberStr); // or std::stod if you prefer double
    }
    catch (const std::invalid_argument &) {
        // If we fail, just return 0 or throw an error
        // std::cerr << "Warning: could not parse number: " << numberStr << std::endl;
        return 0.0f;
    }
}

// -------------------------------------
// After drawing a forward segment, 
// bend heading slightly toward m_tropismVector
// -------------------------------------
void Turtle::applyTropism()
{
    if (m_tropismCoefficient <= 0.0f) {
        return;  // no tropism effect
    }
    if (glm::length(m_tropismVector) < 1e-6f) {
        return;  // tropism vector is zero => no effect
    }

    // Current heading is m_state.yAxis. 
    // Cross with tropism vector T = m_tropismVector.
    glm::vec3 R = glm::cross(m_state.yAxis, m_tropismVector);

    // Angle of rotation alpha = e * |R|
    float magnitudeR = glm::length(R);
    float alpha = m_tropismCoefficient * magnitudeR;
    if (magnitudeR < 1e-6f) {
        return; // heading is nearly parallel or antiparallel to T
    }

    // Rotate the heading around R (normalized) by alpha
    glm::vec3 Rhat = glm::normalize(R);

    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), alpha, Rhat);
    glm::vec4 newHeading = rot * glm::vec4(m_state.yAxis, 0.0f);

    m_state.yAxis = glm::normalize(glm::vec3(newHeading));

    // Re-orthogonalize
    m_state.zAxis = glm::normalize(m_state.zAxis);
    m_state.xAxis = glm::normalize(glm::cross(m_state.yAxis, m_state.zAxis));
    m_state.zAxis = glm::normalize(glm::cross(m_state.xAxis, m_state.yAxis));
}

// ---------------------------------------------------------
// interpret(...) with parametric commands + TROPISM
// ---------------------------------------------------------
void Turtle::interpret(const std::string &lsystemString)
{
    static std::uniform_real_distribution<float> branchDist(0.0f, 1.0f);
    static std::uniform_real_distribution<float> anglePitchDist(-20.0f, 20.0f);
    static std::uniform_real_distribution<float> angleRollDist(-180.0f, 180.0f);

    // Clear old data
    while (!m_radiusStack.empty()) {
        m_radiusStack.pop();
    }
    m_radiusStack.push(m_initialRadius);

    std::stack<TurtleState> stateStack;
    for (size_t i = 0; i < lsystemString.size(); /* manual increment */)
    {
        unsigned int seed = generateSeed(m_state.position);
        std::mt19937 gen(seed);
        char c = lsystemString[i];

        if (c == 'F') {
            size_t lookahead = i + 1;
            float dist = m_stepLength;
            if (lookahead < lsystemString.size() && lsystemString[lookahead] == '(') {
                dist = parseFloatParameter(lsystemString, lookahead);
            }
            i = lookahead;

            // The current radius stack top is the "base" for this segment
            float baseR = m_radiusStack.top();  
            // You want the top radius to be some fraction, e.g. 0.7 * base
            float topR = baseR * m_taperFactor; // Or your desired taper ratio

            // Draw segment from baseR -> topR
            glm::vec3 start = m_state.position;
            glm::vec3 end   = start + m_state.yAxis * dist;

            float colorFactor = baseR / m_initialRadius;
            glColor3f(0.55f * colorFactor, 0.27f * colorFactor, 0.07f * colorFactor);

            drawCylinder(start, end, baseR, topR);

            // Update the radius stack with the new top radius
            m_radiusStack.top() = topR;

            // Move turtle forward
            m_state.position = end;

            // (Optional) add a leaf
            addLeaf(topR);

            // Apply tropism, etc.
            applyTropism();
        }   
        else if (c == '!') {
            // scale radius
            size_t lookahead = i + 1;
            float scale = 1.0f;
            if (lookahead < lsystemString.size() && lsystemString[lookahead] == '(') {
                scale = parseFloatParameter(lsystemString, lookahead);
            }
            i = lookahead;
            float &currentRadius = m_radiusStack.top();
            currentRadius *= scale;
        }
        else if (c == '/') {
            // / => roll
            size_t lookahead = i + 1;
            float angleDeg = 0.0f;
            if (lookahead < lsystemString.size() && lsystemString[lookahead] == '(') {
                angleDeg = parseFloatParameter(lsystemString, lookahead);
            }
            i = lookahead;
            angleDeg += angleRollDist(gen);
            float angleRad = glm::radians(angleDeg);

            rotate(m_state.xAxis, m_state.yAxis, angleRad);
            rotate(m_state.zAxis, m_state.yAxis, angleRad);
        }
        else if (c == '&') {
            // & => pitch up
            size_t lookahead = i + 1;
            float angleDeg = 0.0f;
            if (lookahead < lsystemString.size() && lsystemString[lookahead] == '(') {
                angleDeg = parseFloatParameter(lsystemString, lookahead);
            }
            i = lookahead;
            angleDeg += anglePitchDist(gen);
            float angleRad = glm::radians(angleDeg);

            rotate(m_state.yAxis, m_state.xAxis, -angleRad);
            rotate(m_state.zAxis, m_state.xAxis, -angleRad);
        }
        else if (c == '[') {
            // 1) Generate a random float in [0,1]
            float chance = branchDist(gen);

            // 2) If the chance is below threshold => skip entire bracket block
            //    e.g. 0.2 => 20% chance to remove that sub-branch

            float removeThreshold = 0.0f;  // tweak as desired
            if(i > 6)
            {
                removeThreshold = 0.1f;
            }else if(i > 8)
            {
                removeThreshold = 0.3f;
            }else if(i > 12)
            {
                removeThreshold = 0.5f;
            }
            if (chance < removeThreshold) {
                // Skip until we find the matching ']'
                int bracketDepth = 1; 
                i++; // move past '['
                while (i < lsystemString.size() && bracketDepth > 0) {
                    if (lsystemString[i] == '[') bracketDepth++;
                    else if (lsystemString[i] == ']') bracketDepth--;
                    i++;
                }
                // Now we've consumed that entire sub-branch from the input,
                // effectively removing it from the final image.
            }
            else {
                // Keep the sub-branch as usual
                // (1) push state
                stateStack.push(m_state);

                // (2) push radius stack if needed
                float newRadius = m_radiusStack.top() * m_taperFactor;
                m_radiusStack.push(newRadius);

                i++;
            }
        }

        else if (c == ']') {
            // pop state
            if (!stateStack.empty()) {
                m_state = stateStack.top();
                stateStack.pop();
            }
            if (!m_radiusStack.empty()) {
                m_radiusStack.pop();
            }
            i++;
        }
        else if (c == '+') {
            rotate(m_state.yAxis, m_state.zAxis, m_angleIncrement);
            rotate(m_state.xAxis, m_state.zAxis, m_angleIncrement);
            i++;
        }
        else if (c == '-') {
            rotate(m_state.yAxis, m_state.zAxis, -m_angleIncrement);
            rotate(m_state.xAxis, m_state.zAxis, -m_angleIncrement);
            i++;
        }
        else if (c == '<') {
            rotate(m_state.zAxis, m_state.yAxis, m_angleIncrement);
            rotate(m_state.xAxis, m_state.yAxis, m_angleIncrement);
            i++;
        }
        else if (c == '>') {
            rotate(m_state.zAxis, m_state.yAxis, -m_angleIncrement);
            rotate(m_state.xAxis, m_state.yAxis, -m_angleIncrement);
            i++;
        }
        else if (c == 'v') {
            rotate(m_state.yAxis, m_state.xAxis, m_angleIncrement);
            rotate(m_state.zAxis, m_state.xAxis, m_angleIncrement);
            i++;
        }
        else {
            // skip unknown symbols
            i++;
        }

        // Re-orthogonalize after each command
        m_state.yAxis = glm::normalize(m_state.yAxis);
        m_state.zAxis = glm::normalize(m_state.zAxis);
        m_state.xAxis = glm::normalize(glm::cross(m_state.yAxis, m_state.zAxis));
        m_state.zAxis = glm::normalize(glm::cross(m_state.xAxis, m_state.yAxis));
    }
}


void Turtle::addLeaf(float currentRadius)
{
    //  -----------------------------------
    //  (A) Setup: get random distributions
    //  -----------------------------------


    // Offsets within +/- 0.1 in the XZ plane
    static std::uniform_real_distribution<float> offsetDist(-0.1f, 0.1f);

    // Small tilt angle for orientation, e.g. +/- 15 degrees
    static std::uniform_real_distribution<float> angleDist(-15.0f, 15.0f);

    //  -----------------------------------
    //  (B) Create the leaf struct
    //  -----------------------------------
    Leaf leaf;

    // 1) Position: place the leaf at the turtle's current tip
    leaf.position = m_state.position;
    unsigned int seed = generateSeed(leaf.position);
    std::mt19937 generator(seed);
    // 3) Random offset in plane perpendicular to the branch axis (yAxis)
    float offsetX = offsetDist(generator);
    float offsetZ = offsetDist(generator);

    // Add offsets along the turtle's local X/Z axes
    leaf.position += offsetX * m_state.xAxis + offsetZ * m_state.zAxis;

    // 2) Orientation:
    //    Let's say we want the leaf's 'up' to be partly aligned with "world up"
    //    and partly with the branch axis (yAxis). We'll do a simple average:
    glm::vec3 worldUp(0.0f, 0.0f, 1.0f);
    glm::vec3 approximateNormal = glm::normalize(0.5f * worldUp + 0.5f * m_state.yAxis);

    // Slight random tilt from that approximate normal
    float tiltDeg = angleDist(generator);
    float tiltRad = glm::radians(tiltDeg);

    // We'll rotate around the cross( normal, branchAxis ) to tilt it a bit
    glm::vec3 axis = glm::cross(approximateNormal, m_state.yAxis);
    if (glm::length(axis) < 1e-6f) {
        // fallback if normal ~ branchAxis
        axis = glm::vec3(1, 0, 0);
    }
    axis = glm::normalize(axis);

    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), tiltRad, axis);
    glm::vec4 newUpVec = rot * glm::vec4(approximateNormal, 0.0f);
    glm::vec3 newUp = glm::normalize(glm::vec3(newUpVec));

    // Let's pick 'right' to be perpendicular to newUp & the branch axis
    glm::vec3 newRight = glm::normalize(glm::cross(newUp, m_state.yAxis));

    leaf.up    = newUp;
    leaf.right = newRight;

    // 5) Scale the leaf based on the current branch radius (top of the stack)
    //    If you stored that in radiusStack.top(), you can do:
    // currentRadius = 0.0f;
    if (!m_radiusStack.empty()) {
        currentRadius = m_radiusStack.top();
    }

    float radiusRatio = (m_initialRadius > 0.0f) 
                          ? (currentRadius / m_initialRadius)
                          : 1.0f;
    // Suppose your Leaf struct has a 'scale' field
    leaf.scale = 1.0f + 0.5f * radiusRatio;  
    // e.g. bigger branches => bigger leaves

    //  -----------------------------------
    //  (C) Store the leaf
    //  -----------------------------------
    leafPositions.push_back(leaf);
}

// void Turtle::interpret(const std::string &lsystemString) {
//     std::stack<TurtleState> stateStack;
//     std::stack<float> radiusStack;

//     // Define initial branch radius and taper factor
//     radiusStack.push(m_initialRadius);

//     std::uniform_real_distribution<float> posOffsetDist(-0.5f, 0.5f);
//     std::uniform_real_distribution<float> rotAngleDist(-30.0f, 30.0f); // Degrees
    
//     for (auto c : lsystemString) {
//         switch (c) {
//             case 'F': {
//                 // Current branch radius
//                 float currentRadius = radiusStack.top();

//                 // Current position and orientation
//                 glm::vec3 start = m_state.position;
//                 glm::vec3 end = start + m_state.yAxis * m_stepLength;
                
//                 // Set branch color based on radius (optional)
//                 float colorFactor = currentRadius / m_initialRadius;
//                 glColor3f(0.55f * colorFactor, 0.27f * colorFactor, 0.07f * colorFactor); // Brownish color
                
//                 // Draw the branch as a cylinder
//                 drawCylinder(start, end, currentRadius);
                
//                 // Move to the end position
//                 m_state.position = end;
//             } break;

//             case '+':
//                 // Rotate +angle about Z-axis (yaw left)
//                 rotate(m_state.yAxis, m_state.zAxis, m_angleIncrement);
//                 rotate(m_state.xAxis, m_state.zAxis, m_angleIncrement);
//                 break;

//             case '-':
//                 // Rotate -angle about Z-axis (yaw right)
//                 rotate(m_state.yAxis, m_state.zAxis, -m_angleIncrement);
//                 rotate(m_state.xAxis, m_state.zAxis, -m_angleIncrement);
//                 break;

//             case '<':
//                 // Rotate +angle about Y-axis (roll left)
//                 rotate(m_state.zAxis, m_state.yAxis, m_angleIncrement);
//                 rotate(m_state.xAxis, m_state.yAxis, m_angleIncrement);
//                 break;

//             case '>':
//                 // Rotate -angle about Y-axis (roll right)
//                 rotate(m_state.zAxis, m_state.yAxis, -m_angleIncrement);
//                 rotate(m_state.xAxis, m_state.yAxis, -m_angleIncrement);
//                 break;

//             case 'v':
//                 // Rotate +angle about X-axis (pitch down)
//                 rotate(m_state.yAxis, m_state.xAxis, m_angleIncrement);
//                 rotate(m_state.zAxis, m_state.xAxis, m_angleIncrement);
//                 break;

//             case '&':
//                 // Rotate -angle about X-axis (pitch up)
//                 rotate(m_state.yAxis, m_state.xAxis, -m_angleIncrement);
//                 rotate(m_state.zAxis, m_state.xAxis, -m_angleIncrement);
//                 break;

//             case '[':
//                 // Push current state and radius
//                 stateStack.push(m_state);
//                 radiusStack.push(radiusStack.top() * m_taperFactor); // Tapering
//                 break;

//             case ']':
//                 // Pop state and radius
//                 if (!stateStack.empty()) {
//                     m_state = stateStack.top();
//                     stateStack.pop();
//                 }
//                 if (!radiusStack.empty()) {
//                     radiusStack.pop();
//                 }
//                 break;

//             case 'L': {
//                 // Record leaf position and orientation
//                 Leaf leaf;
//                 leaf.position = m_state.position;

//                 unsigned int seed = generateSeed(leaf.position);
//                 std::mt19937 generator(seed); // Mersenne Twister PRNG
                
//                 // Apply random positional offsets
//                 float offsetX = posOffsetDist(generator);
//                 float offsetY = posOffsetDist(generator);
//                 float offsetZ = posOffsetDist(generator);
//                 leaf.position += glm::vec3(offsetX, offsetY, offsetZ);
                
//                 // Apply random rotations to the orientation vectors
//                 float rotX = rotAngleDist(generator);
//                 float rotY = rotAngleDist(generator);
//                 float rotZ = rotAngleDist(generator);
                
//                 // Create rotation matrices
//                 glm::mat4 rotMatrixX = glm::rotate(glm::mat4(1.0f), glm::radians(rotX), m_state.xAxis);
//                 glm::mat4 rotMatrixY = glm::rotate(glm::mat4(1.0f), glm::radians(rotY), m_state.yAxis);
//                 glm::mat4 rotMatrixZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotZ), m_state.zAxis);
                
//                 // Apply rotations to the orientation vectors
//                 glm::vec4 newUp = rotMatrixX * glm::vec4(m_state.zAxis, 0.0f);
//                 glm::vec4 newRight = rotMatrixY * glm::vec4(m_state.xAxis, 0.0f);
//                 glm::vec4 newForward = rotMatrixZ * glm::vec4(m_state.yAxis, 0.0f);
                
//                 leaf.up = glm::normalize(glm::vec3(newUp));
//                 leaf.right = glm::normalize(glm::vec3(newRight));
//                 leafPositions.push_back(leaf);
//                 break;
//             }

//             default:
//                 // Ignore other symbols
//                 break;
//         }

//         // Re-orthogonalize the coordinate frame to prevent drift
//         m_state.yAxis = glm::normalize(m_state.yAxis);
//         m_state.zAxis = glm::normalize(m_state.zAxis);
//         m_state.xAxis = glm::normalize(glm::cross(m_state.yAxis, m_state.zAxis));
//         m_state.zAxis = glm::normalize(glm::cross(m_state.xAxis, m_state.yAxis));
//     }
// }

void Turtle::rotate(glm::vec3 &dir, const glm::vec3 &axis, float angle) {
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, axis);
    glm::vec4 newDir = rot * glm::vec4(dir, 0.0f);
    dir = glm::normalize(glm::vec3(newDir));
}


// Function to draw a cylinder between two points
void Turtle::drawCylinder(const glm::vec3& start, 
                          const glm::vec3& end, 
                          float baseRadius, 
                          float topRadius)
{
    glm::vec3 direction = end - start;
    float height = glm::length(direction);
    if (height < 1e-6f) return; // skip if zero length

    // Default cylinder is aligned along Z, so we rotate it to match 'direction'
    glm::vec3 up(0.0f, 0.0f, 1.0f);
    glm::vec3 rotationAxis = glm::cross(up, glm::normalize(direction));
    float dotProduct = glm::dot(up, glm::normalize(direction));
    float angle = 0.0f;
    if (glm::length(rotationAxis) < 1e-6f) {
        // direction nearly parallel to 'up'
        rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        angle = (dotProduct > 0) ? 0.0f : 180.0f;
    } else {
        angle = glm::degrees(acos(dotProduct));
    }

    glPushMatrix();
      // Translate to start
      glTranslatef(start.x, start.y, start.z);
      // Rotate so cylinder aligns with direction
      glRotatef(angle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

      GLUquadric* quad = gluNewQuadric();
      gluQuadricNormals(quad, GLU_SMOOTH);

      // Draw the tapered cylinder from baseRadius to topRadius
      gluCylinder(quad, baseRadius, topRadius, height, 12, 3);

      // Optionally cap the smaller end
      gluDisk(quad, 0.0f, topRadius, 12, 1);

      gluDeleteQuadric(quad);
    glPopMatrix();
}
