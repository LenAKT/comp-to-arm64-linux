

CXX = g++
CXXFLAGS = -std=c++20 -g -I./Subs -MMD -MP
ASAN_FLAGS = -fsanitize=address -g -O1

SRC = main.cpp \
      subs/firstPass/reading.cpp \
      subs/maps/maps.cpp \
      subs/secondPass/second.cpp \
	  subs/backEnd/backEnd.cpp \
	  subs/stack/stack.cpp

OBJ = $(SRC:.cpp=.o)
DEP = $(OBJ:.o=.d)
OUT = build/o.out

# Default target builds normally
all: $(OUT)

# Debug target adds ASan flags and rebuilds
debug: CXXFLAGS += $(ASAN_FLAGS)
debug: clean all

$(OUT): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEP)

clean:
	rm -f $(OBJ) $(DEP) $(OUT)

