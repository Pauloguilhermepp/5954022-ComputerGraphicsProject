# Solar System Simulation

The current project aims to simulate the solar system using OpenGL. It provides a physically precise visualization of the Sun, Earth, and other planets, allowing the user to navigate and explore the solar system. The simulation takes into account *Newton's law of universal gravitation* to approximate the bodies orbits.

## Collaborators
This project was developed by:
* Ângelo Pilotto - NºUSP: 12542647
* Paulo Guilherme Pinheiro Pereira - NºUSP: 12542755

## Running the Simulation
If you are in a linux machine, you can run the simulation by executing the *main.sh* file. After access the project directory, you first should make sure that the execution of the file is allowed (you only need to do this step once). To do it, just need to run:

```
chmod +x main.sh
```

After it, you can execute a simulation by running:
```
./main.sh
```

The past command will compile and run the code automatically.

If you want to make your own changes in the code, you also can apply the code formatter in the end! To do it, you need to install *clang* and run the following command:
```
./main.sh format
```

It will automatically format all the files of the project in the directory.

If you are not in a linux machine, you are still able to execute the simulation by manually compiling it:
```
g++ -o main  src/main.cpp -lGL -lGLU -lglut
```

And executing it:
```
./main
```

## Visualization

The simulation renders the following elements:

1. **Stars**: A vast number of stars are scattered throughout the background, creating a realistic starry sky.

2. **Sun**: The Sun is represented as a bright, massive sphere at the center of the simulation.

3. **Planets**: The Earth and the other planets in the solar system are represented as small spheres in orbit around the Sun. They move according to *Newton's law of universal gravitation*.

4. **Moon**: Observing Earth close enough, it is possible to see the Moon orbiting it. Its movement also is totally described by *Newton's law of universal gravitation*.

5. **Comet**: A comet was created describing its orbit using a Bézier curve.

6. **Grid**: A grid is drawn on the XZ plane to provide a reference for orientation.

7. **Crosshair**: A crosshair marks the center of the screen, aiding navigation.

## Navigation

- **Camera Movement**: You can control the camera's movement using the following keys:
  - `W`: Move forward.
  - `A`: Strafe left.
  - `S`: Move backward.
  - `D`: Strafe right.
  - `Spacebar`: Move upward.
  - `\`: Move downward.

- **Planet Finder**: If you want to find a planet, press its number (1 to Mercury up to 8 to Neptune, respecting the distance to the Sun), and the path between your position and the planet will be drawn with the planet color.

- **Mouse**: You can change the camera's direction using the mouse. Move the mouse to look around. Using mouse scrolling you can make your movements faster or slower.

- **Simulation Speed**: You can adjust the simulation speed using the arrow keys. `Up` increases the speed, and `Down` decreases it. If you decrease the simulation speed enough, it actually will reverse like in a time machine!

- **Pause Simulation**: Press the `Enter` key to pause and resume the simulation.

- **Change Field of View (FOV)**: You can adjust the field of view using the `+` and `-` keys.

- **Display Reference Curves**: Press `b` to stop showing the Bézier curve and again to undo it. Do the same with the `g` key for a analogue mechanics with the grid.

- **Finish the simulation**: You can finish the simulation by pressing: `Alt + F4`.

## Observations
- **Planets Diameters**: As mentioned earlier, the simulation is physically accurate. Despite this fact, it was decided to drawn the planets diameters out of scale. This option was actually harder to implement, but it makes easier to observe the simulation for the user (the real size of the planets would be too small).

- **Comet Orbit**: The comet orbit showed in the simulation is described by a Bézier curve. Naturally, it would be possible (and easier) to make the real trajectory of the comet, but that was a necessary step due to some prerequisites of this project. If the ideia of a imprecision in the simulation upsets you, feel free to imagine that, actually, it is not a comet, but the **imperial death star**!
