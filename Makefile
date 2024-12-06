# # Compiler and flags
# CC = g++
# CFLAGS = -Wall -Wextra -std=c++11

# # Include paths for libraries
# INCLUDES = -I/usr/include -I/usr/local/include

# # Libraries to link with (GLFW, GLEW, OpenGL)
# LIBS = -lglfw -lGLEW -lGL -lm

# # Source files and object files
# SRCS = main.cpp
# OBJS = $(SRCS:.cpp=.o)

# # Name of the output executable
# MAIN = sphere

# # Rule to build the executable
# $(MAIN): $(OBJS)
# 	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

# # Rule to build object files from source files
# .cpp.o:
# 	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# # Clean rule to remove object files and the executable
# clean:
# 	$(RM) *.o *~ $(MAIN)





# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Include paths for macOS
INCLUDES = -I. \
          -I/opt/homebrew/include \
          -I/usr/local/include \
          -Iglm

# Library paths and frameworks for macOS
LDFLAGS = -L/opt/homebrew/lib \
          -L/usr/local/lib \
          -lglfw \
          -lGLEW \
          -framework OpenGL \
          -framework Cocoa \
          -framework IOKit \
          -framework CoreVideo

# Source files
SOURCES = main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
TARGET = test_sphere

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -f $(TARGET) $(OBJECTS)

# Install dependencies using Homebrew
deps:
	brew install glfw
	brew install glew
	brew install glm

# System configuration check
check:
	@echo "Checking system configuration..."
	@echo "GLFW:"
	@ls -l /opt/homebrew/lib/libglfw* || echo "GLFW not found!"
	@echo "\nGLEW:"
	@ls -l /opt/homebrew/lib/libGLEW* || echo "GLEW not found!"
	@echo "\nGLM:"
	@ls -l /opt/homebrew/include/glm || echo "GLM not found!"

.PHONY: all clean deps check
