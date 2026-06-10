# Hello World - Your First Zenith Program

Welcome to Zenith! This is your first step into building cross-platform applications with 100% Zenith code.

## Overview

In this tutorial, you'll write a simple "Hello World" program and learn:
- Basic Zenith syntax
- How to run Zenith programs
- Platform-specific output options

## Prerequisites

- Zenith SDK installed (see [Installation Guide](../../docs/getting-started/installation.md))
- A text editor or IDE
- Terminal/Command Prompt

## Step 1: Create Your First Program

Create a new file called `hello_world.zenith`:

```zenith
// hello_world.zenith
// Your first Zenith program!

void main() {
  print('Hello, World!');
}
```

## Step 2: Run Your Program

### On Desktop (Windows/Mac/Linux)

```bash
zenith run hello_world.zenith
```

**Output:**
```
Hello, World!
```

### On Mobile (iOS/Android)

```bash
# For iOS
zenith run hello_world.zenith --platform ios

# For Android
zenith run hello_world.zenith --platform android
```

### On Web

```bash
zenith run hello_world.zenith --platform web
```

This will open your default browser with the output displayed on the page.

## Step 3: Understanding the Code

Let's break down what each part does:

```zenith
// This is a comment - ignored by the compiler
```

Comments help document your code. They start with `//`.

```zenith
void main() {
```

Every Zenith program starts with a `main()` function. The `void` keyword means it doesn't return a value.

```zenith
  print('Hello, World!');
```

The `print()` function outputs text to the console or screen.

## Step 4: Try Variations

### Multiple Lines

```zenith
void main() {
  print('Hello,');
  print('World!');
  print('Welcome to Zenith!');
}
```

### Using Variables

```zenith
void main() {
  String greeting = 'Hello';
  String name = 'Zenith Developer';
  
  print('$greeting, $name!');
}
```

### Interactive Version

```zenith
void main() {
  print('What is your name?');
  String name = readLine();
  print('Hello, $name! Welcome to Zenith!');
}
```

## Step 5: Build for Different Platforms

### Build for Desktop

```bash
# Windows
zenith build desktop --platform windows --release

# macOS
zenith build desktop --platform macos --release

# Linux
zenith build desktop --platform linux --release
```

### Build for Mobile

```bash
# iOS
zenith build ios --release

# Android
zenith build android --release
```

### Build for Web

```bash
zenith build web --release
```

This creates optimized production files in the `build/` directory.

## Complete Example with UI

Here's a version with a graphical interface:

```zenith
import 'package:zenith_ui/zenith_ui.dart';

void main() {
  runApp(HelloWorldApp());
}

class HelloWorldApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text('Hello World'),
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                'Hello, World!',
                style: TextStyle(
                  fontSize: 48,
                  fontWeight: FontWeight.bold,
                  color: Colors.blue,
                ),
              ),
              SizedBox(height: 20),
              Text(
                'Built with 100% Zenith Code',
                style: TextStyle(
                  fontSize: 20,
                  color: Colors.grey,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```

Run this with:

```bash
zenith run hello_world_ui.zenith
```

## Try It Yourself

1. Modify the greeting message
2. Add your name to the output
3. Change the colors in the UI version
4. Try running on different platforms
5. Build and share your app!

## Next Steps

Now that you've written your first program, continue with:
- [Variables & Data Types](02-variables.md)
- [Control Flow](03-control-flow.md)
- [Functions](04-functions.md)

## Common Issues

### "zenith command not found"
Make sure Zenith is installed and added to your PATH. See [Installation Guide](../../docs/getting-started/installation.md).

### Build fails on mobile
Ensure you have the required platform SDKs:
- iOS: Xcode
- Android: Android Studio + SDK

## Resources

- [Zenith Language Documentation](../../docs/INDEX.md)
- [Platform Support](../../README.md#supported-platforms)
- [Community Forum](https://community.zenith-lang.dev)

---

**Congratulations!** You've written your first Zenith program! 🎉

*Built with 100% Zenith Code*
