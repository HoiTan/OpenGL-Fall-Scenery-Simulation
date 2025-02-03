import numpy as np
import matplotlib.pyplot as plt
import json

# Constants for the simulation
g = 9.81          # Gravity (m/s^2)
rho_f = 1.225     # Air density (kg/m^3)

# Object properties
class Object:
    def __init__(self, mass, width, height, drag_coeff_perp, drag_coeff_para):
        self.mass = mass
        self.width = width
        self.height = height
        self.drag_coeff_perp = drag_coeff_perp
        self.drag_coeff_para = drag_coeff_para

# Function to compute the derivatives
def compute_derivatives(state, obj):
    x, y, theta, vx, vy, omega = state

    # Velocity magnitude
    V = np.sqrt(vx**2 + vy**2) + 1e-6  # Avoid division by zero

    alpha = np.arctan2(vx, vy)
    beta = alpha + theta

    k = 1 if np.sign(vy) * np.sin(beta) >= 0 else -1

    # Drag and lift forces
    drag_force = k * np.pi * rho_f * obj.height * V**2 * np.cos(beta) * np.cos(alpha)
    lift_force = k * np.pi * rho_f * obj.width * V**2 * np.cos(beta) * np.sin(alpha)

    # Differential equations
    dx = vx
    dy = vy
    dtheta = omega

    dvx = -(obj.drag_coeff_perp * np.sin(theta)**2 + obj.drag_coeff_para * np.cos(theta)**2) * vx \
          + (obj.drag_coeff_perp - obj.drag_coeff_para) * np.sin(theta) * np.cos(theta) * vy \
          - drag_force / obj.mass

    dvy = -g + -(obj.drag_coeff_perp * np.cos(theta)**2 + obj.drag_coeff_para * np.sin(theta)**2) * vy \
          + (obj.drag_coeff_perp - obj.drag_coeff_para) * np.sin(theta) * np.cos(theta) * vx \
          + lift_force / obj.mass

    domega = (-obj.drag_coeff_perp * omega) - 3 * np.pi * rho_f * V**2 * np.cos(beta) * np.sin(beta)

    return np.array([dx, dy, dtheta, dvx, dvy, domega]) 

# Runge-Kutta 4th-order method
def runge_kutta4(state, obj, dt):
    k1 = compute_derivatives(state, obj)
    k2 = compute_derivatives(state + dt * k1 / 2, obj)
    k3 = compute_derivatives(state + dt * k2 / 2, obj)
    k4 = compute_derivatives(state + dt * k3, obj)
    return state + (dt / 6) * (k1 + 2 * k2 + 2 * k3 + k4)

# Generate a trajectory for given initial conditions
def generate_trajectory(obj, initial_state, dt, steps):
    trajectory = []
    state = np.array(initial_state)
    for _ in range(steps):
        trajectory.append(state.copy())
        state = runge_kutta4(state, obj, dt)
    return trajectory

# Plot a single trajectory
def plot_trajectory(trajectory, label):
    x = [state[0] for state in trajectory]
    y = [state[1] for state in trajectory]
    plt.plot(x, y, label=label)
    plt.xlabel("X Position (m)")
    plt.ylabel("Y Position (m)")
    plt.gca().invert_yaxis()  # Invert y-axis to match falling motion
    plt.legend()

# Generate multiple trajectories with different initial conditions
def generate_trajectory_database():
    # Object properties: mass, width, height, drag coefficients
    obj = Object(mass=0.01, width=0.1, height=0.1, drag_coeff_perp=4.1, drag_coeff_para=0.9)

    # Simulation parameters
    dt = 0.01        # Time step (s)
    steps = 1000     # Number of time steps

    # Different initial conditions: [x, y, theta, vx, vy, omega]
    initial_conditions = [
        [0, 0, 0, 1, -1, 0],
        [0, 0, np.pi / 6, 1, -1, 0.1],
        [0, 0, np.pi / 4, 0.5, -1, -0.1],
        [0, 0, np.pi / 3, 1.5, -2, 0],
    ]

    # Dictionary to store trajectories
    trajectory_database = {}

    plt.figure(figsize=(10, 6))

    for i, initial_state in enumerate(initial_conditions):
        trajectory = generate_trajectory(obj, initial_state, dt, steps)
        label = f"Trajectory {i+1}"
        plot_trajectory(trajectory, label)
        trajectory_database[label] = trajectory

    plt.title("Precomputed Trajectories")
    plt.grid(True)
    plt.show()

    # Save the trajectory database to a JSON file
    with open("precomputed_trajectory_database.json", "w") as f:
        json.dump({label: [state.tolist() for state in traj] for label, traj in trajectory_database.items()}, f, indent=4)
    print("Trajectory database saved to 'precomputed_trajectory_database.json'")

# Main function to execute the program
if __name__ == "__main__":
    generate_trajectory_database()
