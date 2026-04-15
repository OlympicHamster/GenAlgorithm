🏎️ Intelligent cars - Using genetic algorithms

<img width="718" height="484" alt="image" src="https://github.com/user-attachments/assets/228a45b5-b3e5-4505-a67e-5c0861a7d413" />

A lightweight, machine-learning visualization project built in C++ and Raylib. This simulation uses a Genetic Algorithm (Neuroevolution of augmenting topologies) to teach a population of neural-network-driven cars how to navigate a race track.

<img width="204" height="132" alt="image" src="https://github.com/user-attachments/assets/5166be72-86ad-4b6f-bfd6-e9ebc56fd1ad" />

The program uses three distance sensors to fine tune the turning and accelearation sensitivity on the car for each mutation.

🎨 How to Create Your Own Track

Just edit the circuit.png file!
The engine uses strict color coding to understand the world.
<ul>
<li>Path (e.g., Gray, Black, White) Safe zone. Cars can drive freely here.

<li>Walls/Grass Green (R<80, G>120, B<80) Fatal. Touching this eliminates the car.CheckpointRed

<li>Checkpoint Red (R>200, G<80, B<80) Mandatory pass-through zone to validate a lap.

<li>Finish Line Blue (B>200) Triggers the win condition (only if checkpoint was cleared).
</ul>
