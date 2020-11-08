#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include <vector>
using namespace std;

#include <eigen3/Eigen/Dense>
using namespace Eigen;

// Lagrangian particle
class Particle
{
public:
	Particle() : x(0.0f, 0.0f), v(0.0f, 0.0f), m(PARTICLE_MASS) {}
	Particle(Vector2d _x) : x(_x), v(0.0f, 0.0f), m(PARTICLE_MASS) {}
	// position, velocity, and mass
	Vector2d x;
	Vector2d v;
	double m;
	// pressure, density, and their variations
	double p;
	double pv;
	double d;
	double dv;
	// next particle in grid cell linked list
	Particle *n;

	constexpr const static double PARTICLE_MASS = 1.0f;
};

struct Neighborhood
{
public:
	Neighborhood() : particles(MAX_NEIGHBORS), r(MAX_NEIGHBORS), numNeighbors(0) {}
	vector<const Particle *> particles;
	vector<double> r;
	int numNeighbors;

	const static int MAX_NEIGHBORS = 64; // by grid definition
};

#endif