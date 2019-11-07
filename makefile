all: controller

controller: controller.cpp
	g++ controller.cpp -o controller
