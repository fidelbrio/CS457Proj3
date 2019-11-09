all: controller

controller: controller.cpp
	pkg-config --cflags protobuf
	c++ controller.cpp -g -Wall bank.pb.cc -o controller `pkg-config --cflags --libs protobuf`

branch2: branch2.cpp
	pkg-config --cflags protobuf
	c++ branch2.cpp -g bank.pb.cc -o branch2 `pkg-config --cflags --libs protobuf`
