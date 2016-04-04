linuxgcc:
	g++ -I ./boost/1.55.0/include/ -L./boost/1.55.0/lib main.cpp impurity.cpp tuple.cpp boosted_classify.cpp forest.cpp -lboost_thread -lboost_system  -lpthread -O4 -o rtrank 
mac:
	g++ main.cpp impurity.cpp tuple.cpp boosted_classify.cpp forest.cpp -I./boost/1.55.0_mac/include/ -L./boost/1.55.0_mac/lib -lboost_thread-mt -lboost_system-mt -O4 -o rtrank
intel:
	icc main.cpp impurity.cpp tuple.cpp boosted_classify.cpp forest.cpp -lboost_thread -lboost_system -lpthread -static -fast -o rtrank
