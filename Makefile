CXX = g++
CXXFLAGS = -O3 -std=c++17 -Iinclude
LDFLAGS = -lpthread

SRC = src/main.cpp \
      src/frontend/lexer.cpp \
      src/frontend/parser.cpp \
      src/frontend/semantic.cpp \
      src/frontend/formatter.cpp \
      src/lsp/lsp.cpp \
      src/backend/codegen.cpp \
      src/backend/js_codegen.cpp \
      src/backend/wasm_codegen.cpp

TARGET = zenith

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) tests/zenith_app tests/gallery_app tests/main.cpp tests/main.wat tests/main.html tests/main_wasm.html tests/website.cpp tests/website.wat tests/website_wasm.html tests/gallery.cpp tests/gallery.wat tests/gallery.html tests/gallery_wasm.html

check: all
	./$(TARGET) tests/main.zen
	./$(TARGET) tests/main.zen -target web
	./$(TARGET) tests/main.zen -target wasm
	./$(TARGET) tests/website.zen -target wasm
	./$(TARGET) tests/gallery.zen -target cpp
	./$(TARGET) tests/gallery.zen -target web
	./$(TARGET) tests/gallery.zen -target wasm
	$(CXX) $(CXXFLAGS) tests/main.cpp -o tests/zenith_app $(LDFLAGS)
	$(CXX) $(CXXFLAGS) tests/gallery.cpp -o tests/gallery_app $(LDFLAGS)
	./tests/zenith_app

distcheck: check
