# Zenith Feature Audit: Core Language Syntax & Parsing

## 1. Executive Overview
The Zenith compiler frontend provides a static type system with PascalCase primitive types (`Int`, `Float`, `Bool`, `String`, `Void`), first-class math types (`Vec2`, `Vec3`, `Vec4`, `Mat4`), class declarations with interface realization (`class X() implements Interface`), block FFI (`@library("builtin") foreign "C"`), Hindley-Milner type inference, code formatting, and statement semicolons.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **Lexer & Tokenizer** ([lexer.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/lexer.h), [lexer.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/lexer.cpp)):
  - Capitalized primitive types (`Int`, `Float`, `Bool`, `String`, `Void`, `List`, `Map`, `Ref`, `Weak`).
  - Keywords (`class`, `implements`, `interface`, `foreign`, `let`, `async`, `await`, `struct`, `import`, `break`, `continue`).
  - Annotation parsing (`@library`, `@managed`, `@weak`, `@gc_root`).
- **Parser & AST Construction** ([parser.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/parser.h), [parser.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/parser.cpp), [ast.h](file:///c:/Jay/_Plugin/zenith_lang/include/ast/ast.h)):
  - Class definitions with parameter lists and interface realizations (`implements`).
  - Foreign function interface blocks (`foreign "C" { ... }`).
  - List and Map literal parsing (`[e1, e2]`, `{k1: v1}`).
  - Lambda expression parsing (`fn(arg) { ... }`).
- **Semantic Type Checker & Scope Analysis** ([semantic.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/semantic.h), [semantic.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/semantic.cpp)):
  - Symbol table resolution, type checking, and scope validation.
- **Type Inference Engine** ([type_inference.hpp](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/type_inference.hpp)):
  - Hindley-Milner based type inference engine for implicit variable binding types.
- **Code Formatter** ([formatter.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/formatter.h), [formatter.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/formatter.cpp)):
  - AST-driven code formatting and canonical source layout tool.

---

## 3. ❌ Missing / Incomplete Features
- **Pattern & Struct Destructuring in Match**: Destructuring structs/tuples inside `match` arms (crucial for game state machines).
- **Macro System**: AST-level compile-time code generation macros (`#[]`).

---

## 4. 💻 How to Use (Real Compiling Syntax)
```zenith
import std.io;

@library("builtin")
foreign "C" {
    Bool isKeyPressed(String key);
    Bool wasKeyPressed(String key);
}

class Player() implements Entity {
    Int hp = 100;
    Float speed = 5.0;
    String name = "Nova";

    Void takeDamage(amount: Int) {
        hp = hp - amount;
        if (hp < 0) {
            hp = 0;
        }
    }
}

Void main() {
    Player p = Player();
    p.takeDamage(20);
    println("Player HP: " + p.hp.toString());
}
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation File |
| :--- | :--- | :--- |
| **Lexer & Tokenizer** | [lexer.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/lexer.h) | [lexer.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/lexer.cpp) |
| **Parser & AST Builder** | [parser.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/parser.h) | [parser.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/parser.cpp) |
| **AST Definitions** | [ast.h](file:///c:/Jay/_Plugin/zenith_lang/include/ast/ast.h) | N/A (Header-only AST hierarchy) |
| **Semantic Analysis** | [semantic.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/semantic.h) | [semantic.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/semantic.cpp) |
| **Type Inference** | [type_inference.hpp](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/type_inference.hpp) | N/A (Header-only C++ template engine) |
| **Code Formatter** | [formatter.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/formatter.h) | [formatter.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/formatter.cpp) |
