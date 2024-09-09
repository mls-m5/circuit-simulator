## circuit-simulator

This circuit simulator will (if ever finished) do calculations similar to other simulators like SPICE.
But since many SPICE simulations only allows for fixed simulations that is scripted from the start, this simulator will allow for relatime interaction with the user
(As well of of course automated interaction through scripting for automatic testing.)

This way you will be able to use it to test for example design for mechanical keyboards and similar.

## Current features

 - [ ] Basic static analysis
 
## Todo

 - [ ] Time dependent components (inductors and capacitors)
 - [ ] GUI based elements (buttons and onscreen voltage and ampere meters)
 - [ ] Standard analysis with sweep and impulse response

## Design

Each time frame is set up as a linear equation. You have two major options.
Either you set up your equation system and solve it using some normal matrix
solver method, or you use gradient descent to solve each step. Currently the
simulator uses gradient descent, because of its flexibility for handling any
type of component (for example, transistors and diods is not solvable this way).
The downside is that the solutions is not mathematically exact, a level of
accuracy I will probably never need.

I will probably do some implementation using exact mathematical solutions
in the future, but that will only be for the components that is somewhat linear.

A interesting sidenote is that the way to solve a equation set up by a electric
circuit (gradient descent) is the same type of algorithm you use for training
machine learning models (informally called AI).

## Licence

Currently private: Feel free to browse and test run the program at small scale.
See licence file for more information.

