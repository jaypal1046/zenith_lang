CXX = g++
CXXFLAGS = -O3 -std=c++17 -Iinclude

# Platform-specific linker flags
UNAME := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME),Windows)
    LDFLAGS = -lws2_32 -luser32 -lgdi32 -lcomctl32 -lopengl32
    TARGET = zenith.exe
    PYTHON = py
else ifeq ($(UNAME),Darwin)
    # macOS
    LDFLAGS = -framework Cocoa -lpthread -ldl $(shell python3-config --ldflags)
    CXXFLAGS += $(shell python3-config --includes)
    TARGET = zenith
    PYTHON = python3
else
    # Linux and other POSIX systems
    LDFLAGS = -lpthread -ldl $(shell python3-config --ldflags) $(shell pkg-config --libs gtk+-3.0 2>/dev/null || echo "")
    CXXFLAGS += $(shell python3-config --includes) $(shell pkg-config --cflags gtk+-3.0 2>/dev/null || echo "")
    TARGET = zenith
    PYTHON = python3
endif

SRC = src/main.cpp \
      src/frontend/lexer.cpp \
      src/frontend/parser.cpp \
      src/frontend/semantic.cpp \
      src/frontend/formatter.cpp \
      src/lsp/lsp.cpp \
      src/backend/codegen.cpp \
      src/backend/js_codegen.cpp \
      src/backend/wasm_codegen.cpp

# UI source files (platform-specific)
UI_SRC = 
ifeq ($(OS),Windows_NT)
    UI_SRC += src/ui/win32_window.cpp
else ifeq ($(UNAME),Darwin)
    # Add Cocoa implementation when available
    # UI_SRC += src/ui/cocoa_window.mm
else
    # Add GTK implementation when available
    # UI_SRC += src/ui/gtk_window.cpp
endif

all: $(TARGET)

$(TARGET): $(SRC) $(UI_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(TARGET) tests/sandbox tests/ui_tests/gallery.cpp tests/ui_tests/main.cpp tests/language_tests/*.cpp

check: all
	$(PYTHON) tests/run_tests.py

distcheck: check
