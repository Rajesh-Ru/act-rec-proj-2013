CC = g++

CFLAGS = -w -std=c++11
LFLAGS = -lOpenNI -lpthread -lrt -lXi -lX11 -lXxf86vm -lXrandr -lGL -lglfw3

NI_DIR = /home/rajesh/Documents/computer_vision/kinect/openni
GLFW_INC = /usr/local/include
GLFW_LIB = /usr/local/lib

all:
	$(CC) main.cpp scene_drawer/SceneDrawer.cpp -o ni_recorder $(CFLAGS) -I$(NI_DIR)/Include -I$(GLFW_INC) -L$(GLFW_LIB) $(LFLAGS)
clean:
	\rm -f *.o *~ ni_recorder