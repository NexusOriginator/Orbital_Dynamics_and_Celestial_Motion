# OpenGL Solar System Simulation

This project is a 3D simulation of a solar system using OpenGL, featuring a central planet (Earth), orbiting satellites, a moon, asteroids, and a starfield background. The simulation includes interactive controls and realistic 3D rendering with textures.

## Features

- Textured 3D Earth model with rotation
- Two orbiting bodies (a satellite and a moon)
- Interactive satellite control
- Dynamic asteroid field with collision detection
- Animated starfield background
- Texture mapping and lighting effects
- Real-time 3D rendering

## Prerequisites

- macOS operating system
- Xcode Command Line Tools
- Homebrew package manager
- OpenGL 3.3+ support
- Required libraries:
  - GLFW
  - GLEW
  - GLM

## Installation

1. Clone the repository:
```bash
git clone <repository-url>
cd <repository-directory>
```

2. Install dependencies using the provided Makefile:
```bash
make deps
```

3. Verify system configuration:
```bash
make check
```

## Building the Project

1. Compile the project:
```bash
make all
```

2. Clean build files (if needed):
```bash
make clean
```

## Running the Simulation

1. Execute the compiled program:
```bash
./test_sphere
```

2. Required texture files:
- Place these texture files in the same directory as the executable:
  - `earth_texture.jpg`
  - `satellite_texture.jpg`
  - `moon_texture.jpg`

## Controls

- **W**: Increase satellite orbit radius
- **S**: Decrease satellite orbit radius
- **A**: Rotate satellite counterclockwise
- **D**: Rotate satellite clockwise
- **ESC**: Exit the application

## Project Structure

- `main.cpp`: Core implementation file containing all the simulation logic
- `stb_image.h`: Image loading library (header-only)
- Shader implementations:
  - Vertex and fragment shaders for the planet
  - Simplified shaders for asteroids
  - Texture mapping shaders

## Technical Details

- OpenGL Version: 3.3 Core Profile
- Window Resolution: 800x600 pixels
- Features:
  - Perspective projection
  - Depth testing
  - Texture mapping
  - Dynamic object generation
  - Collision detection
  - Vertex and fragment shaders

## Troubleshooting

1. If compilation fails:
- Ensure all dependencies are installed: `make deps`
- Verify system configuration: `make check`
- Check if OpenGL 3.3 is supported on your system

2. If textures don't load:
- Verify texture files exist in the correct directory
- Check file permissions
- Ensure texture file names match exactly

3. If linking fails:
- Verify Homebrew installation
- Check if library paths in Makefile match your system
- Ensure all required frameworks are available

## Notes

- The simulation is optimized for macOS systems
- Performance may vary based on hardware capabilities
- The asteroid field generation can be adjusted by modifying the spawn parameters in the code

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/3477885/aaaae3b3-d813-47c2-8a03-aa9f3916d511/main.cpp