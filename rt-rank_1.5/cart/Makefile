CXX = g++
CC = gcc
linuxgcc:
	g++ -I ./boost/1.55.0/include/ -L./boost/1.55.0/lib main.cpp impurity.cpp tuple.cpp boosted_classify.cpp forest.cpp -lboost_thread -lboost_system  -lpthread -O4 -o rtrank 
