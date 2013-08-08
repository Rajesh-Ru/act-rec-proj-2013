CC = g++
CFLAGS = -w -std=c++11
LFLAGS = -lpthread -lrt -lXi -lX11 -lXxf86vm -lXrandr -lGL -lglfw3

GLFW_INC = /usr/local/include
GLFW_LIB = /usr/local/lib

all:
	$(CC) weapon_driver.cpp Fireball.cpp test_fireball.cpp -o test_fireball -I$(GLFW_INC) $(CFLAGS) -L$(GLFW_LIB) $(LFLAGS)
clean:
	\rm -f test_fireball *~