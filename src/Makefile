TARGET ?= OneDrive.exe

SRC = main.cpp get_system_info.cpp startup.cpp

RES_RESOURCE = resources/resource.o
RES_VERSION = version.o

CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -std=c++17 -I mailio/
LDFLAGS = -luser32 -lwinhttp -liphlpapi -lsetupapi -lole32 -loleaut32 -lshlwapi -luuid -static
RC = x86_64-w64-mingw32-windres

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(RES_RESOURCE) $(RES_VERSION)
	$(CXX) $(CXXFLAGS) $(SRC) $(RES_RESOURCE) $(RES_VERSION) -o $(TARGET) $(LDFLAGS)

$(RES_RESOURCE): resources/resource.rc
	$(RC) resources/resource.rc -O coff -o $(RES_RESOURCE)

$(RES_VERSION): version.rc
	$(RC) version.rc -O coff -o $(RES_VERSION)

clean:
	rm -f $(TARGET) $(RES_RESOURCE) $(RES_VERSION)
