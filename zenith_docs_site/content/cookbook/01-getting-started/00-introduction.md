# Getting Started with Zenith

Welcome to Zenith! This section will help you get started with your first programs.

## Prerequisites

- A computer running Windows, macOS, or Linux
- Internet connection for downloading the SDK
- A code editor (VS Code recommended with Zenith extension)

## Installation

### Step 1: Install Zenith CLI

**macOS/Linux:**
```bash
curl -fsSL https://zenith.dev/install.sh | sh
```

**Windows (PowerShell):**
```powershell
Invoke-WebRequest -Uri https://zenith.dev/install.ps1 -OutFile install.ps1
.\install.ps1
```

### Step 2: Verify Installation

```bash
zenith --version
```

You should see something like: `Zenith CLI v1.0.0`

### Step 3: Create Your First Project

```bash
zenith create hello_world
cd hello_world
```

### Step 4: Run Your App

```bash
# Run on web
zenith run web

# Run on desktop
zenith run desktop

# Run on Android (requires Android Studio)
zenith run android

# Run on iOS (requires macOS + Xcode)
zenith run ios
```

## Your First Program

Open `lib/main.zenith` and you'll see:

```zenith
import 'package:zenith_ui/zenith_ui.dart';

fun main() {
  runApp(MyApp());
}

class MyApp extends App {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Hello World',
      home: Scaffold(
        body: Center(
          child: Text('Hello, World!'),
        ),
      ),
    );
  }
}
```

## Next Steps

- [Variables & Types](02-variables.md) - Learn about data types
- [Functions](03-functions.md) - Write reusable code
- [Control Flow](04-control-flow.md) - Conditionals and loops
- [Error Handling](05-error-handling.md) - Handle errors gracefully

## Need Help?

- [Documentation](../INDEX.md)
- [Cookbook](../cookbook/README.md)
- [Discord Community](https://discord.gg/zenith)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/zenith)
