🏎️ Intelligent cars - Using genetic algorithms

A lightweight, machine-learning visualization project built in C++ and Raylib. This simulation uses a Genetic Algorithm (Neuroevolution of augmenting topologies) to teach a population of neural-network-driven cars how to navigate a race track.

🎨 How to Create Your Own Track

Just edit the circuit.png file!
The engine uses strict color coding to understand the world.

Path (e.g., Gray, Black, White) Safe zone. Cars can drive freely here.

Walls/Grass Green (R<80, G>120, B<80) Fatal. Touching this eliminates the car.CheckpointRed

Checkpoint Red (R>200, G<80, B<80) Mandatory pass-through zone to validate a lap.

Finish Line Blue (B>200) Triggers the win condition (only if checkpoint was cleared).
