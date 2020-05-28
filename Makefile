#######################################################################################################


# Mac OS X
PCISPH_INCLUDE_PATH      = -I/usr/local/include/
PCISPH_LIBRARY_PATH      = -L/usr/local/lib/
PCISPH_OPENGL_LIBS       = -framework OpenGL -framework GLUT

# # Linux
# PCISPH_INCLUDE_PATH      =
# PCISPH_LIBRARY_PATH      =
# PCISPH_OPENGL_LIBS       = -lglut -lGL -lGLU -lX11

# # Windows / Cygwin
# PCISPH_INCLUDE_PATH      = -I/usr/include/opengl
# PCISPH_LIBRARY_PATH      = -L/usr/lib/w32api
# PCISPH_OPENGL_LIBS       = -lglut32 -lglu32 -lopengl32

#######################################################################################################

TARGET = pcisph
CC = g++
LD = g++
CFLAGS = -std=c++11 -O3 -Wall -Wno-deprecated -pedantic -Wno-vla-extension $(PCISPH_INCLUDE_PATH) -I./include -I./src -DNDEBUG
LFLAGS = -std=c++11 -O3 -Wall -Wno-deprecated -Werror -pedantic $(PCISPH_LIBRARY_PATH) -DNDEBUG
LIBS = $(PCISPH_OPENGL_LIBS)

OBJS = obj/main.o

default: $(TARGET)

all: clean $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

obj/main.o: src/main.cpp include/Particle.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/main.cpp -o obj/main.o

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
	rm -f $(TARGET).exe