all: controller

controller: controller.cpp
	pkg-config --cflags protobuf
	c++ controller.cpp bank.pb.cc -o controller `pkg-config --cflags --libs protobuf`
