// System Headers
#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <vector>
#include <omp.h>

// Project Headers
#include "nbody.h"

// #define GRAPHICS
#ifdef GRAPHICS
    #include <SFML/Window.hpp>
    #include <SFML/Graphics.hpp>
#endif

// Number of particles
// #define SMALL
// #define LARGE
#define LARGE

#if defined(SMALL)
    const int N = 1000;
#elif defined(LARGE)
    const int N = 5000;
#elif defined(MASSIVE)
    const int N = 10000;
#endif

// Constants
const double min2 = 2.0;
const double G    = 1 * 10e-10;
const double dt   = 0.01;
const int NO_STEPS = 500;

// Size of Window/Output image
const int width  = 1920;
const int height = 1080;

// Bodies
body *bodies = new body[N];

// Tile size for workload division
const int TILE = 64;
const int numBlocks = (N + TILE - 1) / TILE;

// N-Body update with tiled OpenMP
void update() {
    static std::vector<vec2> acc(N);
    std::fill(acc.begin(), acc.end(), vec2(0,0));

    #pragma omp parallel
    {
        // Local buffer per thread
        std::vector<vec2> local_acc(N, vec2(0,0));

        // Process blocks dynamically
        #pragma omp for schedule(dynamic,1)
        for(int bi = 0; bi < numBlocks; ++bi) {
            for(int bj = bi; bj < numBlocks; ++bj) {
                int i0 = bi * TILE;
                int i1 = std::min(i0 + TILE, N);
                int j0 = bj * TILE;
                int j1 = std::min(j0 + TILE, N);
                for(int i = i0; i < i1; ++i) {
                    int startJ = (bi == bj ? std::max(i+1, j0) : j0);
                    for(int j = startJ; j < j1; ++j) {
                        vec2 dx = bodies[i].pos - bodies[j].pos;
                        double d2 = length2(dx);
                        if(d2 <= min2) continue;
                        vec2 u = normalise(dx);
                        double x = smoothstep(min2, 2*min2, d2);
                        double f = -G * bodies[i].mass * bodies[j].mass / d2;
                        vec2 ai = (u * f / bodies[i].mass) * x;
                        vec2 aj = (u * -f / bodies[j].mass) * x;
                        local_acc[i] += ai;
                        local_acc[j] += aj;
                    }
                }
                // Merge local to global
                #pragma omp critical
                {
                    for(int i = i0; i < i1; ++i) acc[i] += local_acc[i];
                    if(bi != bj) for(int j = j0; j < j1; ++j) acc[j] += local_acc[j];
                }
                // Reset local_acc for block
                for(int i = i0; i < i1; ++i) local_acc[i] = vec2(0,0);
                if(bi != bj) for(int j = j0; j < j1; ++j) local_acc[j] = vec2(0,0);
            }
        }
    }

    // Update positions and velocities
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < N; ++i) {
        bodies[i].pos += bodies[i].vel * dt;
        bodies[i].vel += acc[i] * dt;
    }
}

// Initialise NBody Simulation
void initialise() {
    // Create a central heavy body (sun)
    bodies[0] = body(width/2, height/2, 0, 0, 1e15, 5);

    // For each other body
    for(int i = 1; i < N; ++i) {
        // Pick a random radius, angle and calculate velocity
        double r = (uniform() + 0.1) * height/2;
        double theta = uniform() * 2 * M_PI;
        double v = sqrt(G * (bodies[0].mass + bodies[i].mass) / r);
        // Create orbiting body
        bodies[i] = body(
            width/2 + r * cos(theta),
            height/2 + r * sin(theta),
            -sin(theta) * v,
            cos(theta) * v,
            1e9,
            2
        );
    }
}

#ifdef GRAPHICS
    // Main Function - Graphical Display
    int main() {
        sf::ContextSettings settings;
        settings.antialiasingLevel = 1;
        sf::RenderWindow window(
            sf::VideoMode(width, height),
            "NBody Simulator",
            sf::Style::Default,
            settings
        );
        initialise();
        int step = 0;
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            if (step < NO_STEPS) {
                update();
                ++step;
            }
            window.clear(sf::Color::Black);
            for (int i = 0; i < N; ++i) {
                sf::CircleShape shape(bodies[i].radius);
                shape.setFillColor(sf::Color::Red);
                shape.setPosition(bodies[i].pos.x, bodies[i].pos.y);
                window.draw(shape);
            }
            window.display();
        }
        return 0;
    }
#else
    // Main Function - Benchmark
    int main() {
        initialise();
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < NO_STEPS; ++i)
            update();
        auto end = std::chrono::high_resolution_clock::now();
        // Write output
        write_data("output.dat", bodies, N);
        write_image("output.png", bodies, N, width, height);
        calculate_maximum_difference("reference.dat", bodies, N);
        double secs =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count() / 1e6;
        std::cout << "Time Taken: " << secs << " seconds\n";
        return 0;
    }
#endif
