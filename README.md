# Solar System Simulation

The current project aims to simulate the solar system using OpenGL. It provides a real-time visualization of the Sun, Earth, and other planets, allowing the user to navigate and explore the solar system. The simulation takes into account *Newton's law of universal gravitation* to approximate thr bodies orbits.

## Collaborators
This project was made by:
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

If you are not in a linux machine, you are still able to execute the simulation by manually compiling it:
```
g++ -o simulation  src/simulation.cpp -lGL -lGLU -lglut
```

And executing it:
```
./simulation
```
## Visualization

The simulation renders the following elements:

1. **Stars**: A vast number of stars are scattered throughout the background, creating a realistic starry sky.

2. **Sun**: The Sun is represented as a bright, massive sphere at the center of the simulation.

3. **Earth**: Earth is depicted as a smaller sphere in orbit around the Sun. It moves according to gravitational forces.

4. **Grid**: A grid is drawn on the XZ plane to provide a reference for orientation.

5. **Crosshair**: A crosshair marks the center of the screen, aiding navigation.

## Navigation

- **Camera Movement**: You can control the camera's movement using the following keys:
  - `W`: Move forward.
  - `A`: Strafe left.
  - `S`: Move backward.
  - `D`: Strafe right.
  - `Spacebar`: Move upward.
  - `\`: Move downward.
  - `Alt + F4`: Close the simulation.

- **Mouse**: You can change the camera's direction using the mouse. Move the mouse to look around. Clicking the left mouse button allows you to interact with objects in the simulation.

- **Simulation Speed**: You can adjust the simulation speed using the arrow keys. `Up` increases the speed, and `Down` decreases it.

- **Pause Simulation**: Press the `Enter` key to pause and resume the simulation.

- **Change Field of View (FOV)**: You can adjust the field of view using the `+` and `-` keys.
