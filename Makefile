EXE = dist/cg
IMGUI_DIR = lib/imgui

SOURCES = src/main.cpp src/*.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

# Convert source files (.cpp) to object files (.o) in the dist/ directory
OBJS = $(addprefix dist/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))

LINUX_GL_LIBS = -lGL -lGLEW

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += -I /usr/local/include -I /usr/local/include/opencv4  # Incluir diretórios de cabeçalho do OpenCV

LIBS += -L$(OPENCV_DIR)/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc  # Vincular as bibliotecas OpenCV

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

# Ensure the dist/ directory exists before compiling
dist:
	mkdir -p dist

# Compile .cpp files from the src/ directory into dist/ directory
dist/%.o: src/%.cpp | dist
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile .cpp files from the IMGUI_DIR into dist/ directory
dist/%.o: $(IMGUI_DIR)/%.cpp | dist
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile .cpp files from the IMGUI_DIR/backends into dist/ directory
dist/%.o: $(IMGUI_DIR)/backends/%.cpp | dist
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
