CC = g++
CFLAGS = -w -std=c++11
LFLAGS = -lOpenNI -lpthread -lrt -lXi -lX11 -lXxf86vm -lXrandr -lGL -lglfw3

OPENNI_DIR = /home/rajesh/Documents/computer_vision/kinect/openni
GLFW_INC = /usr/local/include
GLFW_LIB = /usr/local/lib
BIN_DIR = bin

O_FILES = SceneDrawer.o FireBallAction.o KinectDataPub.o RADFactory.o main.o

all: act_rec
	\rm -f *.o
act_rec: scenedrawer.o fireballaction.o kinectdatapub.o radfactory.o main.o
	$(CC) $(O_FILES) -o $(BIN_DIR)/act_rec -I$(OPENNI_DIR)/Include -I$(GLFW_INC) $(CFLAGS) -L$(GLFW_LIB) $(LFLAGS)
scenedrawer.o: clean
	$(CC) SceneDrawer.cpp -I$(OPENNI_DIR)/Include -I$(GLFW_INC) $(CFLAGS) -c
fireballaction.o: clean
	$(CC) FireBallAction.cpp -I$(OPENNI_DIR)/Include $(CFLAGS) -c
kinectdatapub.o: clean
	$(CC) KinectDataPub.cpp -I$(OPENNI_DIR)/Include $(CFLAGS) -c
radfactory.o: clean
	$(CC) RADFactory.cpp -I$(OPENNI_DIR)/Include $(CFLAGS) -c
main.o: clean
	$(CC) main.cpp -I$(OPENNI_DIR)/Include -I$(GLFW_INC) $(CFLAGS) -c
clean:
	\rm -fR *.o *~ $(BIN_DIR); mkdir -p $(BIN_DIR)