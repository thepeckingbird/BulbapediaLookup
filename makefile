PROG = bpediaLookup
SOURCE = main.cpp
CFLAGS = `pkg-config --cflags --libs gtkmm-3.0` -lcurl

$(PROG): $(SOURCE)
	$(CXX) $(SOURCE) -o $(PROG) $(CFLAGS)
