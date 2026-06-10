# Quick Start Guide

Get up and running with Zenith in just a few minutes! This guide will walk you through the basics.

## Prerequisites

- Zenith installed (see [Installation Guide](installation.md))
- A text editor or IDE (VS Code recommended)
- Basic programming knowledge

## Step 1: Create Your First Project

```bash
# Create a new project directory
mkdir my-first-zenith-app
cd my-first-zenith-app

# Initialize a Zenith project
zenith create . --template=app
```

This creates the following structure:
```
my-first-zenith-app/
├── src/
│   └── main.zen
├── zenith.json
└── README.md
```

## Step 2: Write Hello World

Open `src/main.zen` and replace its contents with:

```zenith
import std.io;

Void main() {
    println("Hello, World from Zenith!");
    println("Welcome to the future of programming!");
}
```

## Step 3: Compile and Run

```bash
# Compile to C++ and run
zenith src/main.zen -target cpp

# Run the compiled program
./main  # On Windows: main.exe
```

You should see:
```
Hello, World from Zenith!
Welcome to the future of programming!
```

## Step 4: Try Web Target

Zenith can compile to web too!

```bash
# Compile to web
zenith src/main.zen -target web

# This generates an HTML file
# Open it in your browser
open index.html  # macOS
xdg-open index.html  # Linux
start index.html  # Windows
```

## Step 5: Build a Simple UI

Let's create an interactive application:

```zenith
import std.io;

class CounterApp() {
    Int count = 0;
    
    UI build() {
        return Column(
            Text("Counter App", fontWeight: "bold", fontSize: "24"),
            Text("Count: " + String(count), fontSize: "18"),
            Row(
                Button("Increment", onClick: increment),
                Button("Decrement", onClick: decrement),
                Button("Reset", onClick: reset)
            ),
            spacing: 10
        );
    }
    
    Void increment() {
        count = count + 1;
        println("Count incremented to: " + String(count));
    }
    
    Void decrement() {
        count = count - 1;
        println("Count decremented to: " + String(count));
    }
    
    Void reset() {
        count = 0;
        println("Count reset to: " + String(count));
    }
}

Void main() {
    CounterApp app = CounterApp();
    zenith::runInteractiveLoop(app);
}
```

Compile and run:
```bash
zenith src/main.zen -target cpp
./main
```

## Step 6: Explore Language Features

### Variables and Types

```zenith
import std.io;

Void main() {
    // Type inference - no annotation needed!
    let name = "Zenith";           // String
    let version = 1.0;             // Float
    let isAwesome = true;          // Bool
    let numbers = [1, 2, 3, 4, 5]; // List<Int>
    
    // Explicit types
    Int age: 25;
    String greeting: "Hello!";
    
    println(name + " v" + String(version));
    println("Is awesome? " + String(isAwesome));
}
```

### Functions

```zenith
import std.io;

// Simple function
String greet(String name) {
    return "Hello, " + name + "!";
}

// Function with default parameters
Int add(Int a, Int b = 10) {
    return a + b;
}

// Async function
async String fetchData(String url) {
    String data = await httpGet(url);
    return data;
}

Void main() {
    println(greet("World"));
    println(add(5));      // Uses default b=10
    println(add(5, 20));  // Overrides default
}
```

### Classes

```zenith
import std.io;

class Person(String name, Int age) {
    // Constructor parameters become fields
    
    String introduce() {
        return "Hi, I'm " + name + " and I'm " + String(age) + " years old.";
    }
    
    Void haveBirthday() {
        age = age + 1;
        println(name + " is now " + String(age));
    }
}

Void main() {
    Person alice = Person("Alice", 30);
    println(alice.introduce());
    alice.haveBirthday();
}
```

### Error Handling

```zenith
import std.io;

Option<Int> safeDivide(Int a, Int b) {
    if (b == 0) {
        return None;
    }
    return Some(a / b);
}

Result<String, String> readFile(String path) {
    // Returns Ok(content) or Err(error_message)
    if (path == "") {
        return Err("Empty path");
    }
    return Ok("File content here");
}

Void main() {
    let result = safeDivide(10, 0);
    
    match result {
        Some(value) => println("Result: " + String(value)),
        None => println("Division by zero!")
    }
    
    let fileResult = readFile("");
    match fileResult {
        Ok(content) => println(content),
        Err(error) => println("Error: " + error)
    }
}
```

## Step 7: Use the Package Manager

```bash
# Install a package
zenith install https://github.com/zenith-lang/zenith-http.git

# List installed packages
zenith list

# Search for packages
zenith search http

# Add npm package (for web target)
zenith add chart.js
```

## Step 8: Enable LSP in Your Editor

### VS Code

1. Install the Zenith extension from `vscode-zenith/`
2. Or manually configure in `settings.json`:

```json
{
  "zenith.lsp.command": ["/path/to/zenith", "lsp"]
}
```

### Neovim

Add to your `init.lua`:

```lua
vim.api.nvim_create_autocmd("FileType", {
  pattern = "zenith",
  callback = function()
    vim.lsp.start({
      name = "zenith",
      cmd = { "zenith", "lsp" },
    })
  end,
})
```

## Next Steps

Now that you know the basics, explore:

- [Language Guide](../language-guide/syntax.md) - Deep dive into syntax
- [UI Components](../ui/components.md) - Build beautiful interfaces
- [Cookbook](../cookbook/hello-world.md) - Real-world examples
- [Advanced Topics](../advanced/concurrency.md) - Master concurrency and AI

## Quick Reference Card

| Concept | Syntax | Example |
|---------|--------|---------|
| Variable | `let name = value` | `let x = 42` |
| Function | `Type name(params) { }` | `Int add(Int a, Int b) { return a + b; }` |
| Class | `class Name(fields) { }` | `class Point(Int x, Int y) { }` |
| If statement | `if (cond) { } else { }` | `if (x > 0) { println("positive"); }` |
| Loop | `while (cond) { }` | `while (i < 10) { i = i + 1; }` |
| Import | `import module;` | `import std.io;` |
| Print | `println(value)` | `println("Hello")` |

## Getting Help

- Documentation: [docs/INDEX.md](INDEX.md)
- Issues: [GitHub Issues](https://github.com/jaypal1046/zenith_lang/issues)
- Community: [INSERT DISCORD/SLACK LINK]

Happy coding with Zenith! 🚀

---

*Last updated: January 2024*
