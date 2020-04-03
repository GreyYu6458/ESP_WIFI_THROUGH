cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include_directories(
	./main
	./main/SOCKET
	./main/UART
	./main/WIFI
	./main/CONFORM
	./main/MSG
)
project(UART_UDP_BRIDGE)