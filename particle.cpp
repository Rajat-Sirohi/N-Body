#include "particle.h"

Particle::Particle (double mass, Eigen::Vector3d position, Eigen::Vector3d velocity)
{
    Mass = mass;
    Position = position;
    Velocity = velocity;
}