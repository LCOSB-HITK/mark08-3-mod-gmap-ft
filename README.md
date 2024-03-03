# mark08-3-mod-gmap-ft
(simulations and h/w-testing related to MOD_GMAP module based on mark08)

HLD at [#1](https://github.com/LCOSB-HITK/mark08-3-mod-gmap-ft/issues)

GENERAL DESC:
This repo aims to develop the rover system capable of autonomous navigation, mapping, and obstacle detection. The system is designed to operate with multiple rovers, each equipped with sensors for environmental perception and basic movement controls.

## Hardware Specifications:

- **Units:** Two rovers with shared global maps (`gmap`) and equipped with left and right echo sensors.
- **Sensors:** Echo sensors are utilized for detecting edges of obstacles in the environment.
- **Controls:** Basic movement controls implemented for rover maneuvering.

## Software Specifications:

- **Common Modules:**
  - `gmap`: Maintains the global map and handles coordinate error correction.
  - `repo`: Manages the distributed database and networking functionalities.
  - `quorum`: Ensures consistent and reliable data among rovers.

- **Programs:**
  - **`echo`:** Records echoes from the environment, creating `echo_bundles` based on sensor data.
  - **`gmap`:** Responsible for global mapping, including the maintenance of `frag_tree`, `env_tree`, and control states.
  - **`repo`:** Handles the database, storing and distributing `pl` (Point-Line) objects, `pl_bundles`, and `ua_pl_b_reg`.

- **Functionality:**
  - **`obj_table` Handling:** Basic creation and maintenance of tables representing detected objects in the environment.
  - **`recompose()`:** Functionality for object composition in the global map.

## Testing Considerations:

- **Multi-Unit Testing:**
  - Two different rovers sharing a common but initially empty `gmap`.
  - Echo sensors employed for environment perception during testing.
  - Simple off-chip commands used for basic movement controls.

- **Program Focused Testing:**
  - `repo` for database handling.
  - `echo` for echo recording and processing.
  - `gmap` for global mapping and coordinate error correction.

- **Modular Development:**
  - Encourages the development and testing of individual programs for easier debugging and maintenance.

## Chronological Observation (Desired Output):

1. **`echo_bundles` in RAM:**
   - Record and validate echoes based on sensor data.

2. **`pl` and `pl_bundle` in RAM:**
   - Create `pl` objects representing environment edges.
   - Bundle multiple `pl` objects into `pl_bundles`.

3. **Repository (`repo`):**
   - Store and handle:
     - `pl` objects.
     - `pl_bundles`.
     - Unassigned `ua_pl_b_reg`.

4. **System `gmap`:**
   - Maintain the global map.
   - Execute coordinate error correction code.

5. **Repository/Gmap (`repo/gmap`):**
   - Distribute map fragments (`map_frags`) based on received payload bundles (`pl_b`).
   - Create and update objects (`objs`), specifically `obj_table`.

6. **Local `lame` State of Units (`lcosb_lame.h`):**
   - Maintain global coordinate system and rover positions (`gpos`).

## Getting Started:

1. Clone the repository to your local machine.
   ```bash
   git clone https://github.com/your-username/multi-unit-rover-system.git
   ```

2. Follow the installation instructions in the individual program directories (`echo`, `gmap`, `repo`) to set up dependencies.

3. Run the system and monitor the logs for the desired outputs and test results.

## Contributing:

Feel free to contribute by opening issues, suggesting improvements, or submitting pull requests. Your contributions will help enhance the capabilities and reliability of the self-driving multi-unit rover system.

Happy coding and exploring the world of autonomous rovers! 🚀
