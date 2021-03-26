#include "particle.h"
#include "universe.h"

void Universe::update(double dt)
{
    Eigen::Vector3d accels[numParticles];
    std::fill_n(accels, numParticles, Eigen::Vector3d {0,0,0});
    
    #pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        #pragma unroll
        for (int j = i+1; j < numParticles; j++) {
            Eigen::Vector3d dr = Particles[j].Position - Particles[i].Position;
            double dist = dr.norm();
            accels[i] += G * Particles[j].Mass / (dist * dist * dist) * dr;
            accels[j] -= G * Particles[i].Mass / (dist * dist * dist) * dr;
        }

        Particles[i].Velocity += accels[i] * dt;
        Particles[i].Position += Particles[i].Velocity * dt;
    }
}

void Universe::toVertices(float vertices[])
{
    #pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        #pragma unroll
        for (int j=0; j<3; j++) {
            vertices[3*i + j] = Particles[i].Position[j];
        }
    }
}

void Universe::addParticle (Particle p)
{
    Particles.push_back(p);
    numParticles++;
}