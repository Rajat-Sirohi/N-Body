#ifndef PARTICLE_H
#define PARTICLE_H

#include <eigen3/Eigen/Dense>

class Particle
{
public:
    double Mass;
    Eigen::Vector3d Position;
    Eigen::Vector3d Velocity;

    Particle (double mass = 1, Eigen::Vector3d position = {0,0,0}, Eigen::Vector3d velocity = {0,0,0});
};

#endif