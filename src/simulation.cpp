#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>

// Define gravitational constant
const double G = 6.67430e-11;

// Define a structure to represent celestial bodies
struct Body {
    double mass;
    double x, y;
    double vx, vy;
};

// Function to calculate gravitational force between two bodies
void calculateGravity(Body& body1, Body& body2, double& fx, double& fy) {
    double dx = body2.x - body1.x;
    double dy = body2.y - body1.y;
    double r = sqrt(dx * dx + dy * dy);

    double F = (G * body1.mass * body2.mass) / (r * r);

    fx = F * (dx / r);
    fy = F * (dy / r);
}

// Function to update the position and velocity of a body based on forces
void updateBody(Body& body, double fx, double fy, double dt) {
    double ax = fx / body.mass;
    double ay = fy / body.mass;

    body.vx += ax * dt;
    body.vy += ay * dt;

    body.x += body.vx * dt;
    body.y += body.vy * dt;
}

int main() {
    // Read simulation data from a file
    Body sun, earth;
    double dt, simulationTime;
    std::ifstream inputFile("Data/data.txt");

    inputFile >> dt >> simulationTime;
    inputFile >> sun.mass >> sun.x >> sun.y >> sun.vx >> sun.vy;
    inputFile >> earth.mass >> earth.x >> earth.y >> earth.vx >> earth.vy;

    // Perform the simulation
    for (double t = 0.0; t < simulationTime; t += dt) {
        double fx, fy;

        calculateGravity(earth, sun, fx, fy);
        updateBody(earth, fx, fy, dt);
    }

    // Show results
    std::cout << "Final position of Earth: (" << earth.x << ", " << earth.y << ")" << std::endl;

    return 0;
}
