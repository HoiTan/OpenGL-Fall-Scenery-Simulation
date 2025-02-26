#ifndef TURTLE_HPP
#define TURTLE_HPP
#include <stack> 
#include <string>
#include <vector>
#include <random>
#include "../glm/glm.hpp"

class Turtle {
public:
    // Public struct to store leaf data
    struct Leaf {
        glm::vec3 position;
        glm::vec3 up;
        glm::vec3 right;
        float scale;
    };
    
    Turtle();
    void setAngle(float angleDegrees);
    void setStep(float stepLength);
    void setRadius(float radius);
    void setTaperFactor(float taperFactor);
    void setInitialFactor(float angleDegrees, float stepLength, float radius, float taperFactor);
    void interpret(const std::string &lsystemString);
    // Call these to set tropism (T) and coefficient (e)
    void setTropismVector(const glm::vec3& tropism);
    void setTropismCoefficient(float coeff);
    // Public method to retrieve leaf data
    std::vector<Leaf> GetLeaves() const;
    static void setGlobalSeed(unsigned int seedVal);

private:
    struct TurtleState {
        glm::vec3 position;
        glm::vec3 yAxis; // Heading direction
        glm::vec3 zAxis; // Up direction
        glm::vec3 xAxis; // Right direction
        float currentRadius;
    };

    TurtleState m_state;
    float m_angleIncrement;
    float m_stepLength;
    float m_initialRadius = 0.5f;
    float m_taperFactor = 0.7f;
    std::stack<float> m_radiusStack;
    glm::vec3 m_tropismVector = glm::vec3(0.0f, 0.0f, 0.0f);
    float     m_tropismCoefficient = 0.0f;

    float parseFloatParameter(const std::string &str, size_t &pos) const;
    void rotate(glm::vec3 &dir, const glm::vec3 &axis, float angle);
    void drawCylinder(const glm::vec3& start, 
                  const glm::vec3& end, 
                  float baseRadius, 
                  float topRadius);
    unsigned int generateSeed(const glm::vec3& position);
    int quantize(float value, float scale);
    void addLeaf(float currentRadius);
    void applyTropism();
    static std::mt19937 gGlobalRNG;


    // Store leaf positions and orientations
    std::vector<Leaf> leafPositions;
};

#endif // TURTLE_HPP
