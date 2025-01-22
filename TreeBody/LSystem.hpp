#ifndef LSYSTEM_HPP
#define LSYSTEM_HPP

#include <string>
#include <unordered_map>

class LSystem {
public:
    LSystem(const std::string &axiom, const std::unordered_map<char, std::string> &rules, int iterations);
    std::string generate();

private:
    std::string m_axiom;
    std::unordered_map<char, std::string> m_rules;
    int m_iterations;
};

#endif
