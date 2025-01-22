#ifndef TURTLE_HPP
#define TURTLE_HPP

#include <string>
#include <vector>
#include "/Users/hoimautan/Desktop/CS/CS450/SampleMac/glm/glm.hpp"

class Turtle {
public:
    // Public struct to store leaf data
    struct Leaf {
        glm::vec3 position;
        glm::vec3 up;
        glm::vec3 right;
    };
    
    Turtle();
    void setAngle(float angleDegrees);
    void setStep(float stepLength);
    void setRadius(float radius);
    void setTaperFactor(float taperFactor);
    void setInitialFactor(float angleDegrees, float stepLength, float radius, float taperFactor);
    void interpret(const std::string &lsystemString);

    // Public method to retrieve leaf data
    std::vector<Leaf> GetLeaves() const;

private:
    struct TurtleState {
        glm::vec3 position;
        glm::vec3 yAxis; // Heading direction
        glm::vec3 zAxis; // Up direction
        glm::vec3 xAxis; // Right direction
    };

    TurtleState m_state;
    float m_angleIncrement;
    float m_stepLength;
    float m_initialRadius = 0.5f;
    float m_taperFactor = 0.7f;

    void rotate(glm::vec3 &dir, const glm::vec3 &axis, float angle);
    void drawCylinder(const glm::vec3& start, const glm::vec3& end, float radius);
    unsigned int generateSeed(const glm::vec3& position);
    int quantize(float value, float scale);
    // Store leaf positions and orientations
    std::vector<Leaf> leafPositions;
};

#endif // TURTLE_HPP
