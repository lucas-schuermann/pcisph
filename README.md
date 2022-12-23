`pcisph` is a predictive-corrective smoothed particle hydrodynamics (SPH) implementation in 2D for CS4167 final project. See [here](https://github.com/cerrno/pcisph-wasm) for a more recent implementation in Rust including a parallelized solver.

For further information, please see SPH tutorials on [my website](https://lucasschuermann.com/writing), including an introduction to [SPH math](https://lucasschuermann.com/writing/particle-based-fluid-simulation) and a [simple SPH solver](https://lucasschuermann.com/writing/implementing-sph-in-2d).

## Running
```bash
# install dependencies (debian/ubuntu)
apt install libopengl-dev freeglut3-dev libeigen3-dev

# uncomment header in `Makefile` depending on platform
make

# launch demo
./pcisph
```

## Demo video
[![Demo video](http://img.youtube.com/vi/_Kxp5dJ7HM8/0.jpg)](http://www.youtube.com/watch?v=_Kxp5dJ7HM8 "Predictive-Corrective Incompressible SPH")

## License
This project is distributed under the [MIT license](LICENSE.md).

## Note
This solver is not exactly PCISPH, but can be viewed as 1-iteration of SPH relaxation plus sub-stepping. The “prediction-relaxation” scheme of my implementation actually comes mainly from the (much easier to follow) paper ["Particle-based Viscoelastic Fluid Simulation”](https://dl.acm.org/doi/10.1145/1073368.1073400), as opposed to ["Predictive-Corrective Incompressible SPH”](https://dl.acm.org/doi/10.1145/1576246.1531346).
