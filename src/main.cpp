#define GL_SILENCE_DEPRECATION
#if __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <iostream>
#include <vector>
using namespace std;

#include "Particle.h"
typedef Matrix<unsigned int, 2, 1> Vector2ui;

// rendering projection parameters
const static unsigned int WINDOW_WIDTH = 800;
const static unsigned int WINDOW_HEIGHT = 600;
const static double VIEW_WIDTH = 12.5f;
const static double VIEW_HEIGHT = WINDOW_HEIGHT * VIEW_WIDTH / WINDOW_WIDTH;

// global parameters
const static unsigned int MAX_PARTICLES = 50 * 50;
const static unsigned int fps = 40;
const static Vector2d g(0.0f, -9.81f);
static vector<Vector3d> boundaries = vector<Vector3d>();
const static double EPS = 0.0000001f;
const static double EPS2 = EPS * EPS;

// solver parameters
const static unsigned int SOLVER_STEPS = 10;
const static double REST_DENSITY = 45.0f; // 82.0f;
const static double STIFFNESS = 0.08f;
const static double STIFF_APPROX = 0.1f;
const static double SURFACE_TENSION = 0.0001f;
const static double LINEAR_VISC = 0.25f;
const static double QUAD_VISC = 0.5f;
const static double PARTICLE_RADIUS = 0.03f;
const static double H = 6.0f * PARTICLE_RADIUS; // smoothing radius
const static double DT = ((1.0f / fps) / SOLVER_STEPS);
const static double DT2 = DT * DT;
const static double KERN = 20. / (2. * M_PI * H * H);
const static double KERN_NORM = 30. / (2. * M_PI * H * H);

// global memory allocation
static unsigned int numParticles = MAX_PARTICLES;
static vector<Particle> particles = vector<Particle>();
static vector<Neighborhood> nh(numParticles);
static vector<Vector2d> xlast(numParticles);
static vector<Vector2d> xprojected(numParticles);

// gridding parameters
static const double CELL_SIZE = H; // set to smoothing radius
static const unsigned int GRID_WIDTH = (unsigned int)(VIEW_WIDTH / CELL_SIZE);
static const unsigned int GRID_HEIGHT = (unsigned int)(VIEW_HEIGHT / CELL_SIZE);
static const unsigned int NUM_CELLS = GRID_WIDTH * GRID_HEIGHT;

// gridding memory allocation
static vector<Particle *> grid(NUM_CELLS);
static vector<Vector2i> gridIndices(MAX_PARTICLES);

void GridInsert(void);

void InitSPH(void)
{
	boundaries.push_back(Vector3d(1, 0, 0));
	boundaries.push_back(Vector3d(0, 1, 0));
	boundaries.push_back(Vector3d(-1, 0, -VIEW_WIDTH));
	boundaries.push_back(Vector3d(0, -1, -VIEW_HEIGHT));

	cout << "grid width: " << GRID_WIDTH << endl;
	cout << "grid height: " << GRID_HEIGHT << endl;
	cout << "cell size: " << CELL_SIZE << endl;
	cout << "num cells: " << NUM_CELLS << endl;

	Vector2d start(0.25f * VIEW_WIDTH, 0.95f * VIEW_HEIGHT);
	double x0 = start(0);
	unsigned int num = sqrt(numParticles);
	double spacing = PARTICLE_RADIUS;
	cout << "initializing with " << num << " particles per row for " << num * num << " overall" << endl;
	for (unsigned int i = 0; i < num; i++)
	{
		for (unsigned int j = 0; j < num; j++)
		{
			particles.push_back(Particle(start));
			start(0) += 2.0f * PARTICLE_RADIUS + spacing;
		}
		start(0) = x0;
		start(1) -= 2.0f * PARTICLE_RADIUS + spacing;
	}
	cout << "inserted " << particles.size() << " particles" << endl;

	GridInsert();
}

void GridInsert(void)
{
	for (auto &elem : grid)
		elem = NULL;
	for (auto &p : particles)
	{
		auto i = &p - &particles[0];
		unsigned int xind = p.x(0) / CELL_SIZE;
		unsigned int yind = p.x(1) / CELL_SIZE;
		xind = std::max(1U, std::min(GRID_WIDTH - 2, xind));
		yind = std::max(1U, std::min(GRID_HEIGHT - 2, yind));
		p.n = grid[xind + yind * GRID_WIDTH];
		grid[xind + yind * GRID_WIDTH] = &p;
		gridIndices[i] = Vector2i(xind, yind);
	}
}

void ApplyExternalForces(void)
{
	for (auto &p : particles)
		p.v += g * DT;
}

void Integrate(void)
{
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		auto &p = particles[i];
		xlast[i] = p.x;
		p.x += DT * p.v;
	}
}

void PressureStep(void)
{
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		auto &pi = particles[i];

		Vector2ui ind = Vector2ui(gridIndices[i](0), gridIndices[i](1) * GRID_WIDTH);
		nh[i].numNeighbors = 0;

		double dens = 0.0f;
		double dens_proj = 0.0f;
		for (unsigned int ii = ind(0) - 1; ii <= ind(0) + 1; ii++)
			for (unsigned int jj = ind(1) - GRID_WIDTH; jj <= ind(1) + GRID_WIDTH; jj += GRID_WIDTH)
				for (Particle *pgrid = grid[ii + jj]; pgrid != NULL; pgrid = pgrid->n)
				{
					const Particle &pj = *pgrid;
					Vector2d dx = pj.x - pi.x;
					double r2 = dx.squaredNorm();
					if (r2 < EPS2 || r2 > H * H)
						continue;
					double r = sqrt(r2);
					double a = 1. - r / H;
					dens += pj.m * a * a * a * KERN;
					dens_proj += pj.m * a * a * a * a * KERN_NORM;
					if (nh[i].numNeighbors < Neighborhood::MAX_NEIGHBORS)
					{
						nh[i].particles[nh[i].numNeighbors] = &pj;
						nh[i].r[nh[i].numNeighbors] = r;
						++nh[i].numNeighbors;
					}
				}

		pi.d = dens;
		pi.dv = dens_proj;
		pi.p = STIFFNESS * (dens - pi.m * REST_DENSITY);
		pi.pv = STIFF_APPROX * dens_proj;
	}
}

void Project(void)
{
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		auto &pi = particles[i];

		Vector2d xx = pi.x;
		for (unsigned int j = 0; j < nh[i].numNeighbors; j++)
		{
			const Particle &pj = *nh[i].particles[j];
			double r = nh[i].r[j];
			Vector2d dx = pj.x - pi.x;

			double a = 1. - r / H;
			double d = DT2 * ((pi.pv + pj.pv) * a * a * a * KERN_NORM + (pi.p + pj.p) * a * a * KERN) / 2.;

			// relaxation
			xx -= d * dx / (r * pi.m);

			// surface tension applies if the particles are of the same material
			// this would allow for extensibility of multi-phase
			if (pi.m == pj.m)
				xx += (SURFACE_TENSION / pi.m) * pj.m * a * a * KERN * dx;

			// linear and quadratic visc
			Vector2d dv = pi.v - pj.v;
			double u = dv.dot(dx);
			if (u > 0)
			{
				u /= r;
				double a = 1 - r / H;
				double I = 0.5f * DT * a * (LINEAR_VISC * u + QUAD_VISC * u * u);
				xx -= I * dx * DT;
			}
		}
		xprojected[i] = xx;
	}
}

void Correct(void)
{
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		auto &p = particles[i];
		p.x = xprojected[i];
		p.v = (p.x - xlast[i]) / DT;
	}
}

void EnforceBoundary(void)
{
	for (auto &p : particles)
		for (auto b : boundaries)
		{
			double d = p.x(0) * b(0) + p.x(1) * b(1) - b(2);
			if ((d = std::max(0., d)) < PARTICLE_RADIUS)
				p.v += (PARTICLE_RADIUS - d) * b.segment<2>(0) / DT;
		}
}

void InitGL(void)
{
	glClearColor(0.9f, 0.9f, 0.9f, 1);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(2.5f * PARTICLE_RADIUS * WINDOW_WIDTH / VIEW_WIDTH);
	glMatrixMode(GL_PROJECTION);
}

void Render(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glOrtho(0, VIEW_WIDTH, 0, VIEW_HEIGHT, 0, 1);

	glColor4f(0.2f, 0.6f, 1.0f, 1);
	glBegin(GL_POINTS);
	for (auto &p : particles)
		glVertex2f(p.x(0), p.x(1));
	glEnd();

	glutSwapBuffers();
}

void PrintPositions(void)
{
	for (const auto &p : particles)
		cout << p.x << endl;
}

void Update(void)
{
	for (unsigned int i = 0; i < SOLVER_STEPS; i++)
	{
		ApplyExternalForces();
		Integrate();
		GridInsert();
		PressureStep();
		Project();
		Correct();
		GridInsert();
		EnforceBoundary();
	}

	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInit(&argc, argv);
	glutCreateWindow("PCISPH");
	glutDisplayFunc(Render);
	glutIdleFunc(Update);

	InitGL();
	InitSPH();

	glutMainLoop();
	return 0;
}