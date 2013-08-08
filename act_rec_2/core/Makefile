CC = g++
CFLAGS = -w -std=c++11
LFLAGS = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lOpenNI -lpthread -lrt -lXi -lX11 -lXxf86vm -lXrandr -lGL -lglfw3

OPENNI_DIR = /home/rajesh/Documents/computer_vision/kinect/openni
GLFW_INC = /usr/local/include
GLFW_LIB = /usr/local/lib
BIN_DIR = bin

all: act_rec
	\rm -f *.o
act_rec: clean
	$(CC) *.cpp ../binary_classifier/*.cpp -o $(BIN_DIR)/act_rec -I$(OPENNI_DIR)/Include -I$(GLFW_INC) $(CFLAGS) -L$(GLFW_LIB) $(LFLAGS)
clean:
	\rm -fR *.o *~ $(BIN_DIR); mkdir -p $(BIN_DIR)