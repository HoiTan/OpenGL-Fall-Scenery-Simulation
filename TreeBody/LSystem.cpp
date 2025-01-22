#include "LSystem.hpp"

LSystem::LSystem(const std::string &axiom, const std::unordered_map<char, std::string> &rules, int iterations)
    : m_axiom(axiom), m_rules(rules), m_iterations(iterations)
{}

std::string LSystem::generate() {
    std::string current = m_axiom;
    for (int i = 0; i < m_iterations; i++) {
        std::string next;
        for (auto c : current) {
            if (m_rules.find(c) != m_rules.end()) {
                next += m_rules.at(c);
            } else {
                next += c;
            }
        }
        current = next;
    }
    return current;
}
