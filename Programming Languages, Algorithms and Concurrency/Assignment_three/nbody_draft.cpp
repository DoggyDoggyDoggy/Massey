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

// Global threading buffers
static int g_num_threads = 0;
static std::vector<std::vector<vec2>> g_local_acc;
static vec2 g_acc[N];

// Update Nbody Simulation (optimized multithreaded)
void update() {
    // Initialize once
    if (!g_num_threads) {
        g_num_threads = std::thread::hardware_concurrency();
        if (g_num_threads <= 0) g_num_threads = 4;
        g_local_acc.assign(g_num_threads, std::vector<vec2>(N, vec2(0,0)));
    }

    // Zero local buffers
    for (int t = 0; t < g_num_threads; ++t)
        std::fill_n(g_local_acc[t].begin(), N, vec2(0,0));

    int chunk = (N + g_num_threads - 1) / g_num_threads;
    std::vector<std::thread> threads;

    // Phase 1: compute forces
    auto worker_force = [&](int tid, int start, int end) {
        for (int i = start; i < end; ++i) {
            for (int j = i + 1; j < N; ++j) {
                vec2 dx = bodies[i].pos - bodies[j].pos;
                double d2 = length2(dx);
                if (d2 > min2) {
                    vec2 u = normalise(dx);
                    double x = smoothstep(min2, 2 * min2, d2);
                    double f = -G * bodies[i].mass * bodies[j].mass / d2;
                    vec2 ai = (u * f / bodies[i].mass) * x;
                    vec2 aj = (u * -f / bodies[j].mass) * x;
                    g_local_acc[tid][i] += ai;
                    g_local_acc[tid][j] += aj;
                }
            }
        }
    };

    for (int t = 0; t < g_num_threads; ++t) {
        int s = t * chunk;
        int e = std::min(s + chunk, N);
        threads.emplace_back(worker_force, t, s, e);
    }
    for (auto &th : threads) th.join();

    // Phase 2: merge and update bodies
    threads.clear();
    auto worker_update = [&](int start, int end) {
        for (int i = start; i < end; ++i) {
            // Merge accelerations
            vec2 a(0,0);
            for (int t = 0; t < g_num_threads; ++t)
                a += g_local_acc[t][i];
            g_acc[i] = a;
            // Update state
            bodies[i].pos += bodies[i].vel * dt;
            bodies[i].vel += g_acc[i] * dt;
        }
    };

    for (int t = 0; t < g_num_threads; ++t) {
        int s = t * chunk;
        int e = std::min(s + chunk, N);
        threads.emplace_back(worker_update, s, e);
    }
    for (auto &th : threads) th.join();
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
