#pragma once

#include <string>
#include <unordered_map>

class LSystem
{
public:
    // Constructor: store axiom, rules, and iteration count
    LSystem(const std::string& axiom,
            const std::unordered_map<std::string, std::string>& rules,
            int iterations);

    // Generate the L-system string after all iterations
    std::string generate();

private:
    // Parse a string like "123" or "vr" into a double value
    // "123" => 123.0, "vr" => 1.732, "l" => 50, etc.
    double parseParameter(const std::string& paramStr) const;

    // Given a symbol + numeric parameter (e.g. F(123)), apply rewriting logic
    // e.g. F(x) => F(x * lr)
    std::string rewriteParamToken(const std::string &symbol, double param) const;

    // For a **non-param** symbol like "A", expand it if there's a rule for it
    std::string expandSymbol(const std::string &symbol) const;

private:
    std::string axiom_;
    std::unordered_map<std::string, std::string> rules_;
    int iterations_;
};
