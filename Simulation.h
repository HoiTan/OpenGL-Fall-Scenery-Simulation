#include <iostream>
#include <cmath>
#include <vector>
#include <functional>
#include <fstream>

struct Object {
    double mass;         // Mass of the object (kg)
    double width;        // Width (cross-section) (m)
    double height;       // Height (cross-section) (m)
    double density;      // Density of the object (kg/m^3)
    double dragCoeffPerp;    // Drag coefficient (perpendicular) 
    double dragCoeffPara;    // Drag coefficient (parallel)
    
    Object(double m, double w, double h, double d, double dPerp, double dPara)
        : mass(m), width(w), height(h), density(d), dragCoeffPerp(dPerp), dragCoeffPara(dPara) {}
};

// ///////////

// Function to represent the state [x, y, theta, vx, vy, omega]
struct State {
    double x, y, theta;
    double vx, vy, omega;
};

// Differential equations for the falling object
State derivatives(const State &state, const Object &obj, double rho_f, double g) {
    double A_perp = obj.dragCoeffPerp;
    double A_para = obj.dragCoeffPara;
    double V = std::sqrt(state.vx * state.vx + state.vy * state.vy);
    double liftForce = 0.5 * rho_f * obj.width * V * V;
    double dragForce = 0.5 * rho_f * obj.height * V * V;

    State dState;
    dState.x = state.vx;
    dState.y = state.vy;
    dState.theta = state.omega;

    dState.vx = -(A_perp * std::sin(state.theta) * state.vx + dragForce / obj.mass);
    dState.vy = -g + -(A_para * std::cos(state.theta) * state.vy + liftForce / obj.mass);
    dState.omega = -(A_perp * state.omega);

    return dState;
}

// Runge-Kutta 4th order method
State rungeKutta4(const State &initial, const Object &obj, double rho_f, double g, double dt) {
    State k1 = derivatives(initial, obj, rho_f, g);
    State k2 = derivatives({initial.x + k1.x * dt / 2, initial.y + k1.y * dt / 2, initial.theta + k1.theta * dt / 2,
                            initial.vx + k1.vx * dt / 2, initial.vy + k1.vy * dt / 2, initial.omega + k1.omega * dt / 2}, obj, rho_f, g);
    State k3 = derivatives({initial.x + k2.x * dt / 2, initial.y + k2.y * dt / 2, initial.theta + k2.theta * dt / 2,
                            initial.vx + k2.vx * dt / 2, initial.vy + k2.vy * dt / 2, initial.omega + k2.omega * dt / 2}, obj, rho_f, g);
    State k4 = derivatives({initial.x + k3.x * dt, initial.y + k3.y * dt, initial.theta + k3.theta * dt,
                            initial.vx + k3.vx * dt, initial.vy + k3.vy * dt, initial.omega + k3.omega * dt}, obj, rho_f, g);
    
    State next;
    next.x = initial.x + (dt / 6.0) * (k1.x + 2 * k2.x + 2 * k3.x + k4.x);
    next.y = initial.y + (dt / 6.0) * (k1.y + 2 * k2.y + 2 * k3.y + k4.y);
    next.theta = initial.theta + (dt / 6.0) * (k1.theta + 2 * k2.theta + 2 * k3.theta + k4.theta);
    next.vx = initial.vx + (dt / 6.0) * (k1.vx + 2 * k2.vx + 2 * k3.vx + k4.vx);
    next.vy = initial.vy + (dt / 6.0) * (k1.vy + 2 * k2.vy + 2 * k3.vy + k4.vy);
    next.omega = initial.omega + (dt / 6.0) * (k1.omega + 2 * k2.omega + 2 * k3.omega + k4.omega);

    return next;
}

// / //////////

// Save trajectory segments to a file
void saveTrajectorySegment(const std::vector<State> &trajectory, const std::string &filename) {
    std::ofstream outFile(filename);
    for (const auto &state : trajectory) {
        outFile << state.x << " " << state.y << " " << state.theta << "\n";
    }
    outFile.close();
}

int main() {
    // Initialize object properties
    //Object(double m, double w, double h, double d, double dPerp, double dPara)
    Object obj(0.0001, 0.1, 0.1, 1000, 1.0, 0.5); // Example values
    double rho_f = 1.225;  // Air density (kg/m^3)
    double g = 9.81;       // Gravity (m/s^2)
    double dt = 0.001;      // Time step (s)

    // Initial state: position (0, 0), angle 0, velocities (0, -1), and rotation rate 0
    State state = {0.0, 0.0, 0.0, 0.0, -1.0, 0.0};

    std::vector<State> trajectory;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Iteration | x         | y         | theta     | vx        | vy        | omega\n";
    std::cout << "-------------------------------------------------------------------------------\n";


    for (int i = 0; i < 1000; ++i) {  // 10 seconds == 1000 * 0.01dt (sec)
        trajectory.push_back(state);
        // Print the current state
        std::cout << std::setw(8) << i << " | "
                  << std::setw(9) << state.x << " | "
                  << std::setw(9) << state.y << " | "
                  << std::setw(9) << state.theta << " | "
                  << std::setw(9) << state.vx << " | "
                  << std::setw(9) << state.vy << " | "
                  << std::setw(9) << state.omega << "\n";

        state = rungeKutta4(state, obj, rho_f, g, dt);

        // Check for NaN or extreme values
        if (std::isnan(state.x) || std::isnan(state.y)) {
            std::cerr << "Error: NaN detected at iteration " << i << "\n";
            break;
        }
    }

    // Save the trajectory segment
    saveTrajectorySegment(trajectory, "fluttering_trajectory.txt");

    std::cout << "Trajectory saved!" << std::endl;

    return 0;
}
