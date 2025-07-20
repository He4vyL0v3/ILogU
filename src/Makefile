TARGET ?= OneDrive.exe

SRC = main.cpp get_system_info.cpp startup.cpp
RES = resource.o

CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++17 -I mailio/
LDFLAGS = -luser32 -lwinhttp -liphlpapi -lsetupapi -lole32 -loleaut32 -lshlwapi -luuid -static 
RC = x86_64-w64-mingw32-windres 
.PHONY: all clean build

all: $(TARGET)

$(TARGET): $(SRC) $(RES) $(CONFIG_HEADER)
	$(CXX) $(CXXFLAGS) $(SRC) $(RES) -o $(TARGET) $(LDFLAGS)

$(RES): resources/resource.rc
	$(RC) resources/resource.rc -O coff -o $(RES)

clean:
	rm -f $(TARGET) $(CONFIG_HEADER) $(RES)
