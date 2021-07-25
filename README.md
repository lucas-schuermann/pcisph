# pcisph
Predictive-Corrective smoothed particle hydrodynamics (SPH) implementation in 2D for CS4167 final project

See SPH tutorials on [my website](https://lucasschuermann.com/writing), including [SPH math](https://lucasschuermann.com/writing/particle-based-fluid-simulation) and [a simple SPH solver](https://lucasschuermann.com/writing/implementing-sph-in-2d)

See [here](https://github.com/cerrno/pcisph-rs) for a more recent implementation in Rust

## Demo video
[![Demo video](http://img.youtube.com/vi/_Kxp5dJ7HM8/0.jpg)](http://www.youtube.com/watch?v=_Kxp5dJ7HM8 "Predictive-Corrective Incompressible SPH")

## License
[MIT](https://lucasschuermann.com/license.txt)

## Note
This solver is not exactly PCISPH, but can be viewed as 1-iteration of SPH relaxation plus sub-stepping. The “prediction-relaxation” scheme of my implementation actually comes mainly from the (much easier to follow) paper "Particle-based Viscoelastic Fluid Simulation”, as opposed to "Predictive-Corrective Incompressible SPH”.
