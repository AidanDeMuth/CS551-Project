CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lpqxx -lpq # linker errors otherwise, prob a missing function

TARGET = dbmanager
SRC = DBmanager.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
