#include "LSystem.hpp"
#include <iostream>
#include <cctype>    // for std::isalpha
#include <stdexcept> // for std::invalid_argument, std::runtime_error

// Optionally keep your constants as #defines or switch them to constexpr
#define d1 94.74
#define d2 132.63
#define a 18.95
#define lr 1.109
#define vr 1.01
#define l  5.0

LSystem::LSystem(const std::string& axiom,
                 const std::unordered_map<std::string, std::string>& rules,
                 int iterations)
    : axiom_(axiom)
    , rules_(rules)
    , iterations_(iterations)
{
}

// --------------------------------------------------------------------------
// 1) parseParameter: Converts a string to a double, handling placeholders
// --------------------------------------------------------------------------
double LSystem::parseParameter(const std::string& paramStr) const
{
    // Attempt to parse a numeric value
    try {
        return std::stod(paramStr); // e.g. "123.45" => 123.45
    }
    catch (const std::invalid_argument&) {
        // If we failed, maybe paramStr is "vr", "l", "d1", etc.
        // Replace with actual numeric constants as needed:
        if (paramStr == "vr")  return vr; 
        if (paramStr == "lr")  return lr;
        if (paramStr == "l")   return l;
        if (paramStr == "a")   return a;
        if (paramStr == "d1")  return d1;
        if (paramStr == "d2")  return d2;
        
        // If unknown placeholder, you can throw an error or return 0.
        throw std::runtime_error("Unknown L-system parameter: " + paramStr);
    }
}

// --------------------------------------------------------------------------
// 2) rewriteParamToken: e.g. F( x ) => F( x * lr )
// --------------------------------------------------------------------------
std::string LSystem::rewriteParamToken(const std::string &symbol, double param) const
{
    if (symbol == "F") {
        // e.g., multiply length
        param *= lr;
        return "F(" + std::to_string(param) + ")";
    }
    else if (symbol == "!") {
        // e.g., multiply width
        param *= vr;
        return "!(" + std::to_string(param) + ")";
    }
    else if (symbol == "/") {
        // e.g., angle symbol => maybe param += some base angle, or multiply, etc.
        // param += 10; // or do nothing
        return "/(" + std::to_string(param) + ")";
    }
    else if (symbol == "&") {
        // Another angle symbol: could do param += a
        // param += a; 
        return "&(" + std::to_string(param) + ")";
    }

    // fallback: just pass it through
    return symbol + "(" + std::to_string(param) + ")";
}

// --------------------------------------------------------------------------
// 3) expandSymbol: For non-param symbols like "A"
//    - Return expansions from rules_ or a hard-coded fallback
// --------------------------------------------------------------------------
std::string LSystem::expandSymbol(const std::string &symbol) const
{
    // If we find the symbol in the rules_ map, return the mapped expansion
    auto it = rules_.find(symbol);
    if (it != rules_.end()) {
        return it->second;
    }
    
    // For example, if "A" wasn't in the map, we could do a manual expansion
    if (symbol == "A") {
        // This expansion still includes placeholders like (vr), (l), (d1), etc.
        // parseParameter(...) will handle them in subsequent passes.
        return "!(vr)F(l)[&(a)F(l)A]/(d1)[&(a)F(l)A]/(d2)[&(a)F(l)A]";
    }

    // If we don't have any rule for this symbol, return empty => no expansion
    return "";
}

// --------------------------------------------------------------------------
// 4) generate: Main rewriting loop for param + expansions
// --------------------------------------------------------------------------
std::string LSystem::generate()
{
    std::string current = axiom_;

    // Do N iterations
    for (int iter = 0; iter < iterations_; ++iter)
    {
        std::string next;
        size_t pos = 0;

        while (pos < current.size())
        {
            char c = current[pos];

            // Check if it's a symbol that might have parentheses afterward
            if (std::isalpha(static_cast<unsigned char>(c)) ||
                c == '!' || c == '/' || c == '&' || c == '+' || c == '-')
            {
                // Gather symbol. (Many L-systems just use single-char commands.)
                std::string symbol(1, c);
                pos++;

                // If next char is '(', parse the parameter
                if (pos < current.size() && current[pos] == '(')
                {
                    pos++; // skip '('
                    std::string paramStr;
                    while (pos < current.size() && current[pos] != ')')
                    {
                        paramStr.push_back(current[pos]);
                        pos++;
                    }
                    if (pos < current.size()) {
                        pos++; // skip ')'
                    }

                    // Convert paramStr (e.g. "200" or "vr") => double
                    double paramVal = parseParameter(paramStr);

                    // Rewrite param-based token
                    next += rewriteParamToken(symbol, paramVal);
                }
                else
                {
                    // No parentheses => treat it as a plain symbol. Maybe 'A'?
                    std::string expansion = expandSymbol(symbol);
                    if (!expansion.empty()) {
                        next += expansion;
                    } else {
                        // If no expansion, copy symbol as-is
                        next += symbol;
                    }
                }
            }
            else
            {
                // For brackets '[', ']', whitespace, etc., just copy
                next.push_back(c);
                pos++;
            }
        }

        current = next;
    }

    return current;
}
