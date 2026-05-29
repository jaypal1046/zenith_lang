CXX = g++
CXXFLAGS = -O3 -std=c++17 -Iinclude

# Platform-specific linker flags
UNAME := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(UNAME),Windows)
    LDFLAGS = -lws2_32
    TARGET = zenith.exe
    PYTHON = py
else
    # POSIX systems: add pthread, dl for dynamic library loading, and Python embedding
    LDFLAGS = -lpthread -ldl $(shell python3-config --ldflags)
    CXXFLAGS += $(shell python3-config --includes)
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

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(TARGET) tests/sandbox tests/ui_tests/gallery.cpp tests/ui_tests/main.cpp tests/language_tests/*.cpp

check: all
	$(PYTHON) tests/run_tests.py

distcheck: check
