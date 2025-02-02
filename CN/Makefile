TARGET1=client
TARGET2=server
CXX=gcc
OBJECTS1=client.c
OBJECTS2=server.c

TARGET:$(OBJECTS)
	$(CXX) -o $(TARGET1) $(OBJECTS1)
	$(CXX) -o $(TARGET2) $(OBJECTS2)

clean:
	rm client server