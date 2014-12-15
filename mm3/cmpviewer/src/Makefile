CXX = clang++
CXXFLAGS = -O2 -std=c++11 -Wall -Werror

LD = $(CXX)
LDFLAGS = -losg -losgGA -losgViewer

OBJS = cmp.o main.o
OUT = cmpviewer

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

all: $(OUT)

clean:
	rm -f $(OBJS) $(OUT)
