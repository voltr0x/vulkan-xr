# vulkan-xr
Learn the essentials of creating an XR game using OpenXR and Vulkan in C++, following @tdbe's tutorial.

![Teaser](teaser.gif)

## Key Features Implemented:

### XR Locomotion:
- Enables rotating and accelerated panning of the scene by grabbing with both hands, providing a 'tunnel vision' external scene effect.
- Implemented using state machines for movement and visual effects.

### Gameplay Mechanics
- Implements a mechanics system based on a list of GameBehaviour objects processed by the main loop.
- Each behavior is created, updated, and destroyed during runtime.
- Includes sample mechanics for locomotion, hand interactions, XR input testing, and world objects.

### 'GameData.h'
- Defines 'GameObject' attributes such as materials, models, and properties like world matrix and visibility.
- Introduces 'PlayerObject' with its associated game objects and active states.
- Defines materials with shaders, descriptor-set uniform data, and optional shared pipelines for operations like blend modes.

### Rendering
- Leveraged Vulkan's multiview capabilities to optimize rendering performance and enhance the immersive experience in the XR environment.
- Modified the rendering pipeline by addressing the complexities of Khronos Vulkan and OpenXR implementation.
- Supports per-material, per-model, and per-pipeline properties, facilitating easy creation of materials with unique attributes like transparency and double-sidedness.
- The render pipeline dynamically adapts if default material properties are modified.

### 'Input' Class and 'InputData' in 'Inputspace'
- Implements proper XR input handling, supporting various controllers and headsets with customizable binding paths and action sets.
- Provides easy access to input data and haptic feedback functionalities through 'InputData' and 'InputHaptics'.
- Offers controller and head poses, action states (e.g., buttons, sticks, triggers), user presence detection, and haptic feedback output.
- Exposes matrices and XR action state data for further customization.

### 'Utils' Class
- Includes utility functions for XR, input handling, and additional game development math and operations.
