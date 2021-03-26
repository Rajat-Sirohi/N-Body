#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "particle.h"

#include <vector>
#include <eigen3/Eigen/Dense>
#include <cmath>
#include <iostream>

class Universe
{
public:
    std::vector <Particle> Particles;
    double G = 1;
    int numParticles = 0;
    
    Universe () { }
    void update(double dt);
    void toVertices(float vertices[]);
    void addParticle (Particle p);
};

#endif