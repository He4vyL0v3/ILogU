TARGET ?= OneDrive.exe

SRC = main.cpp
RES = resource.o

CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++11 -I mailio/
LDFLAGS = -luser32 -static
RC = x86_64-w64-mingw32-windres
.PHONY: all clean build

all: $(TARGET)

$(TARGET): $(SRC) $(RES) $(CONFIG_HEADER)
	$(CXX) $(CXXFLAGS) $(SRC) $(RES) -o $(TARGET) $(LDFLAGS)

$(RES): resources/resource.rc
	$(RC) resources/resource.rc -O coff -o $(RES)

clean:
	rm -f $(TARGET) $(CONFIG_HEADER) $(RES)
