# Zenith Cookbook

Welcome to the **Zenith Cookbook**! This is your go-to resource for practical examples and recipes using the Zenith programming language. 

All examples are written **100% in Zenith** - no other languages required. Whether you're building games, mobile apps, web applications, or integrating with Rust/Dart packages, you'll find working examples here.

## Table of Contents

### 🚀 Getting Started
- [Hello World](#hello-world)
- [Basic Calculator](#basic-calculator)
- [File Operations](#file-operations)

### 🎮 Game Development
- [Simple 2D Game](#simple-2d-game)
- [Game Loop Pattern](#game-loop-pattern)
- [Physics Integration](#physics-integration)

### 📱 Mobile Apps (iOS & Android)
- [Mobile UI Layout](#mobile-ui-layout)
- [Camera Access](#camera-access)
- [GPS Location Service](#gps-location-service)
- [Push Notifications](#push-notifications)

### 💻 Desktop Applications
- [Desktop Window App](#desktop-window-app)
- [System Tray Application](#system-tray-application)
- [File Explorer](#file-explorer)

### 🌐 Web & WebAssembly
- [Web Page Component](#web-page-component)
- [Interactive Form](#interactive-form)
- [WASM Module Export](#wasm-module-export)

### 🔗 Interoperability
- [Using Rust Packages](#using-rust-packages)
- [Using Dart/Flutter Packages](#using-dartflutter-packages)
- [FFI with C Libraries](#ffi-with-c-libraries)

### 🤖 AI & Agentic Features
- [AI Assistant Integration](#ai-assistant-integration)
- [Text Summarization](#text-summarization)
- [Code Generation Agent](#code-generation-agent)

### 🧪 Advanced Patterns
- [Async/Await Pattern](#asyncawait-pattern)
- [Actor Concurrency](#actor-concurrency)
- [Custom Memory Management](#custom-memory-management)

---

## Recipes

### Hello World

The classic starting point. Learn how to write and run your first Zenith program.

```zenith
import std.io;

Void main() {
    println("Hello, World! ⚡");
    println("Welcome to Zenith - The Native Agentic Systems Language");
}
```

**Run it:**
```bash
zenith run hello.zn
```

---

### Basic Calculator

A simple command-line calculator demonstrating functions, pattern matching, and user input.

```zenith
import std.io;
import std.math;

class Calculator() {
    Float add(Float a, Float b) => a + b;
    Float subtract(Float a, Float b) => a - b;
    Float multiply(Float a, Float b) => a * b;
    
    Float? divide(Float a, Float b) {
        if (b == 0.0) {
            return null;
        }
        return a / b;
    }
    
    Float power(Float base, Float exponent) => Math.pow(base, exponent);
}

Void main() {
    let calc = Calculator();
    
    println("=== Zenith Calculator ===\n");
    
    println("10 + 5 = " + calc.add(10.0, 5.0).toString());
    println("10 - 5 = " + calc.subtract(10.0, 5.0).toString());
    println("10 × 5 = " + calc.multiply(10.0, 5.0).toString());
    
    match (calc.divide(10.0, 0.0)) {
        Some(result) => println("10 ÷ 0 = " + result.toString()),
        None => println("Cannot divide by zero!"),
    }
    
    println("2^8 = " + calc.power(2.0, 8.0).toString());
}
```

---

### File Operations

Reading and writing files with error handling.

```zenith
import std.io;
import std.fs;

Void main() {
    // Write to a file
    let content = "Hello from Zenith!\nThis is a test file.";
    
    match (File.write("test.txt", content)) {
        Ok(_) => println("✓ File written successfully"),
        Err(e) => println("✗ Error writing file: " + e.message),
    }
    
    // Read from a file
    match (File.read("test.txt")) {
        Ok(data) => {
            println("\n📄 File contents:");
            println(data);
        },
        Err(e) => println("✗ Error reading file: " + e.message),
    }
    
    // List directory contents
    match (Dir.list(".")) {
        Ok(files) => {
            println("\n📁 Current directory:");
            for (file in files) {
                println("  - " + file.name);
            }
        },
        Err(e) => println("Error listing directory: " + e.message),
    }
}
```

---

### Simple 2D Game

Create a basic 2D game with game loop, rendering, and player movement.

```zenith
import std.game;
import std.graphics;
import std.input;

class Player() {
    Float x = 400.0;
    Float y = 300.0;
    Float speed = 5.0;
    Int size = 50;
    
    Void update(Input input) {
        if (input.isKeyDown(KeyCode.W)) y -= speed;
        if (input.isKeyDown(KeyCode.S)) y += speed;
        if (input.isKeyDown(KeyCode.A)) x -= speed;
        if (input.isKeyDown(KeyCode.D)) x += speed;
    }
    
    Void render(Renderer renderer) {
        renderer.setColor(Color.blue);
        renderer.fillRect(x, y, size, size);
    }
}

class MyGame : Game {
    Player player;
    
    New() {
        player = Player();
    }
    
    Void onUpdate(Float deltaTime, Input input) override {
        player.update(input);
    }
    
    Void onRender(Renderer renderer) override {
        renderer.clear(Color.black);
        player.render(renderer);
        
        renderer.setColor(Color.white);
        renderer.drawText("Use WASD to move", 10, 10, 16);
    }
}

Void main() {
    let config = GameConfig(
        title: "My Zenith Game",
        width: 800,
        height: 600,
        fps: 60
    );
    
    let game = MyGame();
    Game.run(config, game);
}
```

---

### Game Loop Pattern

Advanced game loop with delta time, fixed timestep physics, and state management.

```zenith
import std.game;
import std.time;

enum GameState {
    Menu,
    Playing,
    Paused,
    GameOver,
}

class GameLoop() {
    GameState state = GameState.Menu;
    Float accumulator = 0.0;
    Float fixedTimestep = 1.0 / 60.0;
    
    Void tick(Float deltaTime) {
        accumulator += deltaTime;
        
        while (accumulator >= fixedTimestep) {
            update(fixedTimestep);
            accumulator -= fixedTimestep;
        }
        
        render();
    }
    
    Void update(Float dt) {
        match (state) {
            GameState.Menu => updateMenu(),
            GameState.Playing => updateGame(dt),
            GameState.Paused => {}, // Do nothing
            GameState.GameOver => updateGameOver(),
        }
    }
    
    Void render() {
        // Render current frame
    }
    
    private Void updateMenu() { /* ... */ }
    private Void updateGame(Float dt) { /* ... */ }
    private Void updateGameOver() { /* ... */ }
}

Void main() {
    let loop = GameLoop();
    let clock = Clock();
    
    while (true) {
        let deltaTime = clock.restart().asSeconds();
        loop.tick(deltaTime);
    }
}
```

---

### Physics Integration

Integrate with a physics engine for collision detection and rigid body simulation.

```zenith
import std.physics;
import std.game;

class PhysicsObject() {
    RigidBody body;
    Sprite sprite;
    
    New(Float x, Float y, Float width, Float height) {
        body = RigidBody.dynamic(x, y);
        body.addCollider(BoxCollider(width, height));
        sprite = Sprite.load("sprite.png");
    }
    
    Void update(Float dt) {
        body.applyGravity();
        body.step(dt);
        
        // Sync sprite position with physics body
        sprite.x = body.position.x;
        sprite.y = body.position.y;
    }
    
    Void render(Renderer renderer) {
        sprite.draw(renderer);
    }
}

class PhysicsGame : Game {
    Array<PhysicsObject> objects;
    
    New() {
        objects = [];
        
        // Create ground
        objects.push(PhysicsObject(400, 550, 800, 50));
        
        // Create falling boxes
        for (i in 0..5) {
            objects.push(PhysicsObject(
                300.0 + i * 50.0,
                100.0,
                40.0,
                40.0
            ));
        }
    }
    
    Void onUpdate(Float deltaTime, Input input) override {
        for (obj in objects) {
            obj.update(deltaTime);
        }
    }
    
    Void onRender(Renderer renderer) override {
        renderer.clear(Color.skyBlue);
        for (obj in objects) {
            obj.render(renderer);
        }
    }
}
```

---

### Mobile UI Layout

Build responsive mobile UI layouts that work on both iOS and Android.

```zenith
import std.ui;
import std.mobile;

class MobileApp : App {
    Widget build(BuildContext context) override {
        return MaterialApp(
            home: Scaffold(
                appBar: AppBar(
                    title: Text("Zenith Mobile App"),
                    backgroundColor: Colors.blue,
                ),
                body: SafeArea(
                    child: Column(
                        children: [
                            Container(
                                padding: EdgeInsets.all(16),
                                child: Text(
                                    "Welcome to Zenith!",
                                    style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
                                ),
                            ),
                            
                            Expanded(
                                child: ListView.builder(
                                    itemCount: 20,
                                    itemBuilder: (context, index) {
                                        return ListTile(
                                            leading: Icon(Icons.star),
                                            title: Text("Item " + index.toString()),
                                            subtitle: Text("Description for item " + index.toString()),
                                            onTap: () {
                                                print("Clicked item " + index.toString());
                                            },
                                        );
                                    },
                                ),
                            ),
                            
                            Container(
                                padding: EdgeInsets.all(16),
                                child: ElevatedButton(
                                    onPressed: () {
                                        showDialog(
                                            context: context,
                                            builder: (ctx) => AlertDialog(
                                                title: Text("Hello"),
                                                content: Text("This runs on iOS & Android!"),
                                            ),
                                        );
                                    },
                                    child: Text("Click Me"),
                                ),
                            ),
                        ],
                    ),
                ),
            ),
        );
    }
}

Void main() {
    runApp(MobileApp());
}
```

---

### Camera Access

Access device camera from mobile apps using pure Zenith.

```zenith
import std.mobile;
import std.camera;
import std.io;

class CameraApp : StatefulWidget {
    @override
    CameraAppState createState() => CameraAppState();
}

class CameraAppState : State<CameraApp> {
    CameraController? controller;
    Image? capturedImage;
    
    @override
    Void initState() {
        super.initState();
        initializeCamera();
    }
    
    async Void initializeCamera() {
        controller = CameraController(
            lensDirection: CameraLensDirection.back,
            resolutionPreset: ResolutionPreset.high,
        );
        
        try {
            await controller!.initialize();
            setState(() {});
        } catch (e) {
            print("Error initializing camera: " + e.message);
        }
    }
    
    async Void takePicture() async {
        if (controller != null && controller!.value.isInitialized) {
            try {
                let image = await controller!.takePicture();
                setState(() {
                    capturedImage = image;
                });
                print("Picture saved to: " + image.path);
            } catch (e) {
                print("Error taking picture: " + e.message);
            }
        }
    }
    
    @override
    Widget build(BuildContext context) {
        if (controller == null || !controller!.value.isInitialized) {
            return CircularProgressIndicator();
        }
        
        return Scaffold(
            appBar: AppBar(title: Text("Camera Example")),
            body: Stack(
                children: [
                    CameraPreview(controller!),
                    Positioned(
                        bottom: 20,
                        left: 0,
                        right: 0,
                        child: Row(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: [
                                FloatingActionButton(
                                    onPressed: takePicture,
                                    child: Icon(Icons.camera_alt),
                                ),
                            ],
                        ),
                    ),
                    if (capturedImage != null)
                        Positioned(
                            top: 20,
                            right: 20,
                            child: Image.file(capturedImage!, width: 100, height: 100),
                        ),
                ],
            ),
        );
    }
}

Void main() {
    runApp(CameraApp());
}
```

---

### GPS Location Service

Get device location using GPS sensors.

```zenith
import std.mobile;
import std.location;
import std.io;

class LocationApp : StatefulWidget {
    @override
    LocationAppState createState() => LocationAppState();
}

class LocationAppState : State<LocationApp> {
    Location? currentLocation;
    String status = "Waiting for location...";
    
    async Void getCurrentLocation() async {
        try {
            let location = await Geolocator.getCurrentPosition(
                desiredAccuracy: LocationAccuracy.high,
            );
            
            setState(() {
                currentLocation = location;
                status = "Location acquired!";
            });
            
            print("Latitude: " + location.latitude.toString());
            print("Longitude: " + location.longitude.toString());
        } catch (e) {
            setState(() {
                status = "Error: " + e.message;
            });
        }
    }
    
    async Void startLocationStream() async {
        Geolocator.getPositionStream(
            desiredAccuracy: LocationAccuracy.high,
        ).listen((position) {
            setState(() {
                currentLocation = position;
            });
            print("Updated: " + position.latitude.toString() + ", " + position.longitude.toString());
        });
    }
    
    @override
    Widget build(BuildContext context) {
        return Scaffold(
            appBar: AppBar(title: Text("GPS Location")),
            body: Padding(
                padding: EdgeInsets.all(20),
                child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                        Text(status, style: TextStyle(fontSize: 16)),
                        SizedBox(height: 20),
                        if (currentLocation != null) ...[
                            Text("Latitude: " + currentLocation!.latitude.toString()),
                            Text("Longitude: " + currentLocation!.longitude.toString()),
                            Text("Altitude: " + currentLocation!.altitude.toString() + "m"),
                            Text("Speed: " + currentLocation!.speed.toString() + " m/s"),
                        ],
                        SizedBox(height: 20),
                        ElevatedButton(
                            onPressed: getCurrentLocation,
                            child: Text("Get Current Location"),
                        ),
                        ElevatedButton(
                            onPressed: startLocationStream,
                            child: Text("Start Location Stream"),
                        ),
                    ],
                ),
            ),
        );
    }
}

Void main() {
    runApp(LocationApp());
}
```

---

### Push Notifications

Implement push notifications for mobile apps.

```zenith
import std.mobile;
import std.notifications;
import std.io;

class NotificationApp : App {
    async Void initNotifications() async {
        // Request permission
        let settings = await FirebaseMessaging.instance.requestPermission(
            alert: true,
            badge: true,
            sound: true,
        );
        
        print("Notification permissions: " + settings.toString());
        
        // Get FCM token
        let token = await FirebaseMessaging.instance.getToken();
        print("FCM Token: " + token);
        
        // Listen for foreground messages
        FirebaseMessaging.onMessage.listen((RemoteMessage message) {
            print("Received message: " + message.notification?.title);
            showLocalNotification(
                title: message.notification?.title ?? "Notification",
                body: message.notification?.body ?? "",
            );
        });
        
        // Handle background messages
        FirebaseMessaging.onBackgroundMessage(handleBackgroundMessage);
    }
    
    Void showLocalNotification(String title, String body) {
        FlutterLocalNotificationsPlugin().show(
            0,
            title,
            body,
            NotificationDetails(
                android: AndroidNotificationDetails(
                    "channel_id",
                    "Channel Name",
                    importance: Importance.high,
                ),
                ios: IOSNotificationDetails(),
            ),
        );
    }
    
    @override
    Widget build(BuildContext context) {
        return MaterialApp(
            home: Scaffold(
                appBar: AppBar(title: Text("Push Notifications")),
                body: Center(
                    child: ElevatedButton(
                        onPressed: () {
                            showLocalNotification("Test", "This is a test notification!");
                        },
                        child: Text("Show Test Notification"),
                    ),
                ),
            ),
        );
    }
}

async Void handleBackgroundMessage(RemoteMessage message) async {
    print("Handling background message: " + message.messageId);
}

Void main() async {
    WidgetsFlutterBinding.ensureInitialized();
    await Firebase.initializeApp();
    
    let app = NotificationApp();
    await app.initNotifications();
    
    runApp(app);
}
```

---

### Desktop Window App

Create a native desktop application for Windows, macOS, and Linux.

```zenith
import std.desktop;
import std.ui;

class DesktopApp : Window {
    New() : super(
        title: "Zenith Desktop App",
        width: 800,
        height: 600,
        resizable: true,
    ) {
        this.buildUI();
    }
    
    Void buildUI() {
        let vbox = VBox(spacing: 10);
        
        // Header
        let label = Label("Welcome to Zenith Desktop!");
        label.fontSize = 24;
        label.bold = true;
        vbox.addWidget(label);
        
        // Input field
        let input = Entry(placeholder: "Enter your name...");
        input.onChanged = (text) {
            print("Input changed: " + text);
        };
        vbox.addWidget(input);
        
        // Button
        let button = Button("Click Me!");
        button.onClick = () {
            let dialog = MessageDialog(
                title: "Hello",
                message: "Hello, " + input.text + "! Welcome to Zenith!",
                buttons: [DialogButton.OK],
            );
            dialog.show(this);
        };
        vbox.addWidget(button);
        
        // Slider
        let slider = HScale(min: 0, max: 100, step: 1);
        slider.value = 50;
        slider.onChanged = (value) {
            print("Slider value: " + value.toString());
        };
        vbox.addWidget(slider);
        
        this.setChild(vbox);
    }
    
    override Void onClose() {
        print("Window closing...");
    }
}

Void main() {
    Application.init();
    
    let window = DesktopApp();
    window.show();
    
    Application.run();
}
```

---

### System Tray Application

Create an application that runs in the system tray.

```zenith
import std.desktop;
import std.tray;
import std.io;

class TrayApp() {
    SystemTrayItem tray;
    
    New() {
        tray = SystemTrayItem(
            icon: "icon.png",
            tooltip: "Zenith Tray App",
        );
        
        setupMenu();
    }
    
    Void setupMenu() {
        let menu = Menu();
        
        menu.addItem(MenuItem(
            label: "Show Window",
            onClick: () {
                showMainWindow();
            },
        ));
        
        menu.addItem(MenuItem.separator());
        
        menu.addItem(MenuItem(
            label: "Settings",
            onClick: () {
                openSettings();
            },
        ));
        
        menu.addItem(MenuItem(
            label: "Check for Updates",
            onClick: () {
                checkForUpdates();
            },
        ));
        
        menu.addItem(MenuItem.separator());
        
        menu.addItem(MenuItem(
            label: "Quit",
            onClick: () {
                quitApp();
            },
        ));
        
        tray.setMenu(menu);
        
        // Double-click to show window
        tray.onDoubleClick = () {
            showMainWindow();
        };
    }
    
    Void showMainWindow() {
        print("Showing main window...");
        // Create and show main window
    }
    
    Void openSettings() {
        print("Opening settings...");
    }
    
    Void checkForUpdates() {
        print("Checking for updates...");
    }
    
    Void quitApp() {
        print("Quitting application...");
        Application.quit();
    }
}

Void main() {
    Application.init();
    
    let app = TrayApp();
    
    // Keep application running in background
    Application.runTrayMode();
}
```

---

### File Explorer

Build a file explorer application with directory navigation.

```zenith
import std.desktop;
import std.fs;
import std.ui;

class FileExplorer : Window {
    TreeView treeView;
    ListView listView;
    Label pathLabel;
    String currentPath = "/";
    
    New() : super(
        title: "Zenith File Explorer",
        width: 1000,
        height: 700,
    ) {
        this.buildUI();
        this.loadDirectory("/");
    }
    
    Void buildUI() {
        let hbox = HBox(spacing: 5);
        
        // Left panel - Directory tree
        treeView = TreeView();
        treeView.onSelectionChanged = (node) {
            if (node != null) {
                loadDirectory(node.data.path);
            }
        };
        hbox.addWidget(treeView, stretch: 1);
        
        // Right panel - File list
        let vbox = VBox(spacing: 5);
        
        // Path bar
        let pathBox = HBox(spacing: 5);
        pathLabel = Label("Path: /");
        pathBox.addWidget(pathLabel);
        
        let upButton = Button("⬆ Up");
        upButton.onClick = () {
            goUpOneLevel();
        };
        pathBox.addWidget(upButton);
        
        vbox.addWidget(pathBox);
        
        // File list
        listView = ListView();
        listView.onItemActivated = (item) {
            let path = item.data.path;
            if (Dir.exists(path)) {
                loadDirectory(path);
            } else {
                openFile(path);
            }
        };
        vbox.addWidget(listView, stretch: 1);
        
        hbox.addWidget(vbox, stretch: 2);
        
        this.setChild(hbox);
    }
    
    Void loadDirectory(String path) {
        currentPath = path;
        pathLabel.text = "Path: " + path;
        
        // Clear existing items
        listView.clear();
        
        match (Dir.list(path)) {
            Ok(entries) => {
                for (entry in entries) {
                    let icon = entry.isDirectory ? "📁" : "📄";
                    let item = ListViewItem(icon + " " + entry.name);
                    item.data.path = entry.fullPath;
                    listView.addItem(item);
                }
            },
            Err(e) => {
                print("Error loading directory: " + e.message);
            },
        }
    }
    
    Void goUpOneLevel() {
        if (currentPath != "/" && currentPath.contains("/")) {
            let parent = Dir.parent(currentPath);
            loadDirectory(parent);
        }
    }
    
    Void openFile(String path) {
        print("Opening file: " + path);
        Process.open(path);
    }
}

Void main() {
    Application.init();
    
    let explorer = FileExplorer();
    explorer.show();
    
    Application.run();
}
```

---

### Web Page Component

Create interactive web components using Zenith's web target.

```zenith
import std.web;
import std.dom;

class HomePage : Component {
    String title = "Welcome to Zenith Web";
    Int counter = 0;
    
    Void increment() {
        counter++;
        rerender();
    }
    
    Void decrement() {
        counter--;
        rerender();
    }
    
    Element render() override {
        return div(class: "container") [
            h1(text: title),
            p(text: "This page is built with 100% Zenith code!"),
            
            div(class: "counter-box") [
                button(
                    class: "btn btn-danger",
                    onClick: decrement,
                    text: "-",
                ),
                
                span(class: "counter-value", text: counter.toString()),
                
                button(
                    class: "btn btn-primary",
                    onClick: increment,
                    text: "+",
                ),
            ],
            
            form(onSubmit: handleSubmit) [
                input(type: "text", placeholder: "Enter your name", id: "nameInput"),
                button(type: "submit", text: "Submit"),
            ],
            
            div(id: "message"),
        ];
    }
    
    Void handleSubmit(Event event) {
        event.preventDefault();
        
        let input = document.getElementById("nameInput") as HTMLInputElement;
        let name = input.value;
        
        let messageDiv = document.getElementById("message");
        messageDiv.innerText = "Hello, " + name + "! Welcome to Zenith!";
        messageDiv.style.color = "green";
        
        input.value = "";
    }
}

Void main() {
    let app = HomePage();
    app.mount(document.getElementById("app"));
}
```

---

### Interactive Form

Build forms with validation and real-time feedback.

```zenith
import std.web;
import std.validation;

class RegistrationForm : Component {
    Map<String, String> formData = {};
    Map<String, String> errors = {};
    Bool isSubmitting = false;
    
    Bool validateField(String field, String value) -> Bool {
        match (field) {
            "email" => {
                if (!Validator.isEmail(value)) {
                    errors["email"] = "Please enter a valid email";
                    return false;
                }
            },
            "password" => {
                if (value.length < 8) {
                    errors["password"] = "Password must be at least 8 characters";
                    return false;
                }
            },
            "confirmPassword" => {
                if (value != formData["password"]) {
                    errors["confirmPassword"] = "Passwords do not match";
                    return false;
                }
            },
        }
        
        errors.remove(field);
        return true;
    }
    
    Void handleChange(String field, String value) {
        formData[field] = value;
        validateField(field, value);
        rerender();
    }
    
    async Void handleSubmit(Event event) async {
        event.preventDefault();
        
        // Validate all fields
        let isValid = validateField("email", formData["email"] ?? "") &&
                     validateField("password", formData["password"] ?? "") &&
                     validateField("confirmPassword", formData["confirmPassword"] ?? "");
        
        if (!isValid) {
            rerender();
            return;
        }
        
        isSubmitting = true;
        rerender();
        
        // Simulate API call
        try {
            await Future.delay(Duration(seconds: 2));
            
            print("Form submitted successfully!");
            print("Email: " + formData["email"]);
            print("Password: " + formData["password"]);
            
            // Show success message
            alert("Registration successful!");
        } catch (e) {
            errors["submit"] = "Submission failed: " + e.message;
        } finally {
            isSubmitting = false;
            rerender();
        }
    }
    
    Element render() override {
        return div(class: "form-container") [
            h2(text: "Create Account"),
            
            if (errors["submit"] != null)
                div(class: "error-message", text: errors["submit"]),
            
            form(onSubmit: handleSubmit) [
                div(class: "form-group") [
                    label(for: "email", text: "Email"),
                    input(
                        type: "email",
                        id: "email",
                        value: formData["email"] ?? "",
                        onChange: (e) => handleChange("email", (e.target as HTMLInputElement).value),
                        class: if (errors["email"] != null) "error" else "",
                    ),
                    if (errors["email"] != null)
                        div(class: "field-error", text: errors["email"]),
                ],
                
                div(class: "form-group") [
                    label(for: "password", text: "Password"),
                    input(
                        type: "password",
                        id: "password",
                        value: formData["password"] ?? "",
                        onChange: (e) => handleChange("password", (e.target as HTMLInputElement).value),
                        class: if (errors["password"] != null) "error" else "",
                    ),
                    if (errors["password"] != null)
                        div(class: "field-error", text: errors["password"]),
                ],
                
                div(class: "form-group") [
                    label(for: "confirmPassword", text: "Confirm Password"),
                    input(
                        type: "password",
                        id: "confirmPassword",
                        value: formData["confirmPassword"] ?? "",
                        onChange: (e) => handleChange("confirmPassword", (e.target as HTMLInputElement).value),
                        class: if (errors["confirmPassword"] != null) "error" else "",
                    ),
                    if (errors["confirmPassword"] != null)
                        div(class: "field-error", text: errors["confirmPassword"]),
                ],
                
                button(
                    type: "submit",
                    class: "btn btn-primary",
                    disabled: isSubmitting,
                    text: if (isSubmitting) "Submitting..." else "Register",
                ),
            ],
        ];
    }
}

Void main() {
    let form = RegistrationForm();
    form.mount(document.getElementById("app"));
}
```

---

### WASM Module Export

Export Zenith functions to WebAssembly for use in JavaScript.

```zenith
// compile with: zenith build --target wasm mymodule.zn

import std.wasm;

// Export a simple function
@export
Int add(Int a, Int b) {
    return a + b;
}

@export
Int multiply(Int a, Int b) {
    return a * b;
}

@export
String greet(String name) {
    return "Hello, " + name + "! From Zenith WASM";
}

// Export a more complex function
@export
Array<Int> fibonacci(Int n) {
    let result: Array<Int> = [];
    
    if (n <= 0) return result;
    if (n == 1) {
        result.push(0);
        return result;
    }
    
    result.push(0);
    result.push(1);
    
    for (i in 2..n) {
        result.push(result[i - 1] + result[i - 2]);
    }
    
    return result;
}

// Memory allocation for JS interop
@export
Ptr<Void> allocateMemory(Int size) {
    return malloc(size);
}

@export
Void freeMemory(Ptr<Void> ptr) {
    free(ptr);
}

Void main() {
    // This won't be called in WASM mode
    // but required for compilation
}
```

**Usage in JavaScript:**
```javascript
// Load the WASM module
const zenithModule = await loadWasm('mymodule.wasm');

// Call exported functions
console.log(zenithModule.add(5, 3)); // 8
console.log(zenithModule.multiply(4, 7)); // 28
console.log(zenithModule.greet("World")); // "Hello, World! From Zenith WASM"

// Get Fibonacci sequence
let fib = zenithModule.fibonacci(10);
console.log(fib); // [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]
```

---

### Using Rust Packages

Integrate Rust libraries into your Zenith project using the `zenith_rust` bridge.

```zenith
import std.rust;
import std.io;

// Declare external Rust function
@rust_link("my_rust_lib")
external Int rust_add(Int a, Int b);

@rust_link("my_rust_lib")
external String rust_process_data(String data);

@rust_link("my_rust_lib")
external Array<Int> rust_compute(Array<Int> numbers);

// Use FFI types
@rust_link("my_rust_lib")
external RustString rust_get_greeting();

Void main() {
    println("=== Rust Integration Example ===\n");
    
    // Call simple Rust function
    let sum = rust_add(10, 20);
    println("10 + 20 = " + sum.toString() + " (computed in Rust)");
    
    // Process string in Rust
    let input = "Hello from Zenith";
    let processed = rust_process_data(input);
    println("Rust processed: " + processed);
    
    // Work with arrays
    let numbers = [1, 2, 3, 4, 5];
    let result = rust_compute(numbers);
    println("Rust computed: " + result.toString());
    
    // Use custom Rust types
    let greeting = rust_get_greeting();
    println("Greeting: " + greeting.toString());
}
```

**Rust Library (`src/lib.rs`):**
```rust
use std::ffi::{CStr, CString};
use std::os::raw::c_char;

#[no_mangle]
pub extern "C" fn rust_add(a: i64, b: i64) -> i64 {
    a + b
}

#[no_mangle]
pub extern "C" fn rust_process_data(data: *const c_char) -> *mut c_char {
    unsafe {
        let c_str = CStr::from_ptr(data);
        let rust_str = c_str.to_string_lossy();
        let processed = format!("Processed in Rust: {}", rust_str);
        CString::new(processed).unwrap().into_raw()
    }
}

#[no_mangle]
pub extern "C" fn rust_compute(numbers: *const i64, len: usize) -> *mut i64 {
    unsafe {
        let slice = std::slice::from_raw_parts(numbers, len);
        let result: Vec<i64> = slice.iter().map(|x| x * 2).collect();
        
        let boxed = result.into_boxed_slice();
        let ptr = boxed.as_mut_ptr();
        std::mem::forget(boxed);
        ptr
    }
}
```

**Build Configuration (`Zenith.toml`):**
```toml
[package]
name = "my_zenith_app"
version = "0.1.0"

[rust]
enabled = true
path = "./rust_lib"
crate_name = "my_rust_lib"
```

---

### Using Dart/Flutter Packages

Leverage the Flutter ecosystem by using Dart packages in Zenith.

```zenith
import std.dart;
import std.io;

// Import Dart package
@dart_import("http")
class HttpClient {
    external Future<Response> get(String url);
    external Future<Response> post(String url, Map<String, String> body);
}

@dart_import("shared_preferences")
class SharedPreferences {
    external static Future<SharedPreferences> getInstance();
    external Future<Bool> setString(String key, String value);
    external String? getString(String key);
}

@dart_import("url_launcher")
class UrlLauncher {
    external static Future<Bool> launch(String url);
}

async Void main() async {
    println("=== Dart Package Integration ===\n");
    
    // Use HTTP client
    let http = HttpClient();
    
    try {
        let response = await http.get("https://api.example.com/data");
        println("Status: " + response.statusCode.toString());
        println("Body: " + response.body);
    } catch (e) {
        println("HTTP Error: " + e.message);
    }
    
    // Use shared preferences
    let prefs = await SharedPreferences.getInstance();
    await prefs.setString("username", "zenith_user");
    
    let username = prefs.getString("username");
    println("Saved username: " + username);
    
    // Launch URL
    let launched = await UrlLauncher.launch("https://zenith-lang.dev");
    if (launched) {
        println("URL launched successfully!");
    }
}
```

**Auto-binding Generator:**

You can automatically generate bindings for any Dart package:

```bash
# Generate bindings for a Dart package
zenith dart-bind generate http
zenith dart-bind generate shared_preferences
zenith dart-bind generate url_launcher

# Build with Dart packages
zenith build --with-dart
```

---

### FFI with C Libraries

Call C libraries directly from Zenith using FFI.

```zenith
import std.ffi;
import std.io;

// Define C function signatures
@c_link("libc")
external Int printf(String format, ...);

@c_link("libc")
external Ptr<Void> malloc(Int size);

@c_link("libc")
external Void free(Ptr<Void> ptr);

@c_link("libc")
external Ptr<CChar> strcpy(Ptr<CChar> dest, Ptr<CChar> src);

// Custom struct matching C layout
@repr(C)
struct Point {
    x: Int,
    y: Int,
}

@c_link("mylib")
external Point create_point(Int x, Int y);

@c_link("mylib")
external Float calculate_distance(Point a, Point b);

Void main() {
    println("=== C FFI Example ===\n");
    
    // Use libc functions
    printf("Hello from C via Zenith!\n");
    
    // Allocate memory manually
    let ptr = malloc(100);
    println("Allocated memory at: " + ptr.address.toString());
    free(ptr);
    
    // Work with structs
    let p1 = create_point(0, 0);
    let p2 = create_point(3, 4);
    
    let distance = calculate_distance(p1, p2);
    println("Distance between points: " + distance.toString()); // 5.0
}
```

---

### AI Assistant Integration

Use Zenith's native `agentic` keyword for AI integration.

```zenith
import std.ai;
import std.io;

class AIAssistant() {
    agentic String summarize(String text) {
        prompt: "Summarize the following text in 3 bullet points:\n\n{text}"
        temperature: 0.7
        maxTokens: 200
    }
    
    agentic String translate(String text, String targetLanguage) {
        prompt: "Translate the following text to {targetLanguage}:\n\n{text}"
        preserveFormatting: true
    }
    
    agentic Array<String> extractKeywords(String text) {
        prompt: "Extract the top 5 keywords from this text as a JSON array:\n\n{text}"
        outputFormat: "json"
    }
    
    agentic String answerQuestion(String question, String context) {
        prompt: "Based on the following context, answer the question.\n\nContext: {context}\n\nQuestion: {question}"
        streaming: true
    }
}

async Void main() async {
    let assistant = AIAssistant();
    
    println("=== AI Assistant Demo ===\n");
    
    // Summarize text
    let article = """
    Zenith is a revolutionary programming language that combines the performance 
    of C++ with modern developer ergonomics. It features native AI primitives, 
    cross-platform UI capabilities, and seamless interoperability with Rust and 
    Dart ecosystems. Built for the next generation of software development.
    """;
    
    println("📝 Summary:");
    let summary = await assistant.summarize(article);
    println(summary);
    println();
    
    // Translate text
    println("🌍 Translation:");
    let translation = await assistant.translate("Hello, World!", "Spanish");
    println(translation);
    println();
    
    // Extract keywords
    println("🏷️ Keywords:");
    let keywords = await assistant.extractKeywords(article);
    for (keyword in keywords) {
        println("  - " + keyword);
    }
}
```

---

### Text Summarization

Build a document summarization tool using AI agents.

```zenith
import std.ai;
import std.io;
import std.fs;

class DocumentSummarizer() {
    agentic String summarizeDocument(String content, String summaryType) {
        prompt: """
        Create a {summaryType} summary of the following document.
        
        Requirements:
        - Keep it concise and informative
        - Highlight key points
        - Maintain technical accuracy
        
        Document:
        {content}
        """
        temperature: 0.5
        maxTokens: 500
    }
    
    agentic Array<String> generateTLDR(String content) {
        prompt: "Generate 5 TL;DR bullet points for this content:\n\n{content}"
        outputFormat: "list"
    }
    
    agentic String extractExecutiveSummary(String content) {
        prompt: "Write an executive summary (150 words max) for this document:\n\n{content}"
        tone: "professional"
    }
}

async Void main() async {
    let summarizer = DocumentSummarizer();
    
    println("📄 Document Summarizer\n");
    
    // Read document
    match (File.read("document.txt")) {
        Ok(content) => {
            println("Document loaded (" + content.length.toString() + " chars)\n");
            
            // Generate different types of summaries
            println("=== Bullet Summary ===");
            let bullets = await summarizer.summarizeDocument(content, "bullet-point");
            println(bullets);
            println();
            
            println("=== TL;DR ===");
            let tldr = await summarizer.generateTLDR(content);
            for (point in tldr) {
                println("• " + point);
            }
            println();
            
            println("=== Executive Summary ===");
            let executive = await summarizer.extractExecutiveSummary(content);
            println(executive);
        },
        Err(e) => println("Error reading file: " + e.message),
    }
}
```

---

### Code Generation Agent

Create an AI agent that generates code based on specifications.

```zenith
import std.ai;
import std.io;
import std.fs;

class CodeGenerator() {
    agentic String generateFunction(String description, String language) {
        prompt: """
        Write a {language} function that: {description}
        
        Requirements:
        - Include proper error handling
        - Add doc comments
        - Follow best practices
        - Include example usage
        """
        temperature: 0.3
    }
    
    agentic String refactorCode(String code, String improvement) {
        prompt: """
        Refactor the following code to: {improvement}
        
        Code:
        {code}
        
        Provide only the refactored code without explanation.
        """
        temperature: 0.2
    }
    
    agentic String generateTests(String code, String testFramework) {
        prompt: """
        Write comprehensive unit tests for this code using {testFramework}:
        
        {code}
        
        Include:
        - Happy path tests
        - Edge case tests
        - Error handling tests
        """
    }
    
    agentic String explainCode(String code) {
        prompt: """
        Explain what this code does in simple terms:
        
        {code}
        
        Break down:
        1. Overall purpose
        2. Key logic flow
        3. Important details
        """
    }
}

async Void main() async {
    let generator = CodeGenerator();
    
    println("🤖 Code Generation Agent\n");
    
    // Generate a function
    println("=== Generating Function ===");
    let func = await generator.generateFunction(
        "calculates the factorial of a number using recursion",
        "Zenith"
    );
    println(func);
    println();
    
    // Refactor code
    println("=== Refactoring Code ===");
    let oldCode = """
    def calc(a,b):
        return a+b
    """;
    
    let refactored = await generator.refactorCode(
        oldCode,
        "add type hints, docstring, and input validation"
    );
    println(refactored);
    println();
    
    // Generate tests
    println("=== Generating Tests ===");
    let tests = await generator.generateTests(func, "Zenith Test Framework");
    println(tests);
}
```

---

### Async/Await Pattern

Master asynchronous programming in Zenith.

```zenith
import std.async;
import std.io;
import std.net;
import std.time;

// Async function that fetches data
async String fetchData(String url) async {
    println("Fetching: " + url);
    
    // Simulate network delay
    await Future.delay(Duration(milliseconds: 1000));
    
    return "Data from " + url;
}

// Async function with error handling
async String? fetchWithRetry(String url, Int retries) async {
    for (attempt in 1..retries) {
        try {
            return await fetchData(url);
        } catch (e) {
            println("Attempt " + attempt.toString() + " failed: " + e.message);
            if (attempt < retries) {
                await Future.delay(Duration(milliseconds: 500 * attempt));
            }
        }
    }
    return null;
}

// Run multiple async operations concurrently
async Void fetchAll() async {
    let urls = [
        "https://api.example.com/users",
        "https://api.example.com/posts",
        "https://api.example.com/comments",
    ];
    
    // Run concurrently
    let futures = urls.map(url => fetchData(url));
    let results = await Future.wait(futures);
    
    println("\n=== All Results ===");
    for (result in results) {
        println(result);
    }
}

// Async stream processing
async Void processStream() async {
    let stream = Stream<int>.generate(
        count: 10,
        producer: (index) async {
            await Future.delay(Duration(milliseconds: 100));
            return index * 2;
        },
    );
    
    println("\n=== Stream Processing ===");
    
    await stream.forEach((value) {
        println("Received: " + value.toString());
    });
}

async Void main() async {
    println("=== Async/Await Demo ===\n");
    
    // Sequential execution
    let data1 = await fetchData("url1");
    let data2 = await fetchData("url2");
    println(data1);
    println(data2);
    
    // With retry logic
    let result = await fetchWithRetry("flaky-url", 3);
    if (result != null) {
        println("Success: " + result);
    } else {
        println("All retries failed");
    }
    
    // Concurrent execution
    await fetchAll();
    
    // Stream processing
    await processStream();
}
```

---

### Actor Concurrency

Implement actor-based concurrency for safe parallel processing.

```zenith
import std.actor;
import std.io;
import std.time;

// Define actor messages
enum Message {
    Ping,
    Pong,
    Stop,
}

// Worker actor
class Worker : Actor<Message> {
    Int id;
    
    New(Int id) {
        this.id = id;
    }
    
    override Void onReceive(Message msg, Sender sender) {
        match (msg) {
            Message.Ping => {
                println("Worker " + id.toString() + " received Ping");
                sender.send(Message.Pong);
            },
            Message.Pong => {
                println("Worker " + id.toString() + " received Pong");
            },
            Message.Stop => {
                println("Worker " + id.toString() + " stopping");
                context.stop(self);
            },
        }
    }
}

// Coordinator actor
class Coordinator : Actor<Message> {
    Array<ActorRef<Message>> workers;
    Int pendingResponses = 0;
    
    New(Int workerCount) {
        workers = [];
        
        for (i in 0..workerCount) {
            let worker = context.spawn(Worker(i));
            workers.push(worker);
        }
    }
    
    override Void onReceive(Message msg, Sender sender) {
        match (msg) {
            Message.Ping => {
                println("Coordinator sending Ping to all workers");
                pendingResponses = workers.length;
                
                for (worker in workers) {
                    worker.send(Message.Ping);
                }
            },
            Message.Pong => {
                pendingResponses--;
                println("Received Pong (" + pendingResponses.toString() + " remaining)");
                
                if (pendingResponses == 0) {
                    println("All workers responded!");
                    
                    // Send stop signal
                    for (worker in workers) {
                        worker.send(Message.Stop);
                    }
                }
            },
            Message.Stop => {
                context.stop(self);
            },
        }
    }
}

Void main() {
    println("=== Actor Concurrency Demo ===\n");
    
    // Create actor system
    let system = ActorSystem("MySystem");
    
    // Spawn coordinator with 5 workers
    let coordinator = system.spawn(Coordinator(5));
    
    // Send initial message
    coordinator.send(Message.Ping);
    
    // Let it run
    sleep(Duration(seconds: 2));
    
    // Shutdown
    system.shutdown();
    
    println("\nActor system shut down");
}
```

---

### Custom Memory Management

Implement custom memory management strategies for performance-critical code.

```zenith
import std.memory;
import std.io;

// Custom arena allocator
class Arena {
    Ptr<Void> buffer;
    Int capacity;
    Int offset = 0;
    
    New(Int size) {
        capacity = size;
        buffer = malloc(size);
        println("Arena allocated: " + size.toString() + " bytes");
    }
    
    Ptr<Void> allocate(Int size) {
        if (offset + size > capacity) {
            throw OutOfMemoryError("Arena full");
        }
        
        let ptr = buffer + offset;
        offset += size;
        return ptr;
    }
    
    Void reset() {
        offset = 0;
        println("Arena reset");
    }
    
    Destroy() {
        free(buffer);
        println("Arena freed");
    }
}

// Reference counted pointer
class Rc<T> {
    private Ptr<T> data;
    private Ptr<Int> refCount;
    
    New(T value) {
        data = malloc(sizeof(T)) as Ptr<T>;
        refCount = malloc(sizeof(Int)) as Ptr<Int>;
        *data = value;
        *refCount = 1;
    }
    
    Clone() -> Rc<T> {
        (*refCount)++;
        return Rc<T>.wrap(data, refCount);
    }
    
    T get() => *data;
    
    Int referenceCount() => *refCount;
    
    Destroy() {
        (*refCount)--;
        if (*refCount == 0) {
            free(data);
            free(refCount);
            println("Rc data freed");
        }
    }
    
    private New.wrap(Ptr<T> data, Ptr<Int> refCount) {
        this.data = data;
        this.refCount = refCount;
    }
}

// Object pool for reuse
class ObjectPool<T> {
    Array<T> available;
    Int maxSize;
    
    New(Int size, T prototype) {
        maxSize = size;
        available = [];
        
        for (i in 0..size) {
            available.push(prototype.clone());
        }
    }
    
    T acquire() {
        if (available.length > 0) {
            return available.pop();
        }
        throw PoolExhaustedError("No objects available");
    }
    
    Void release(T obj) {
        if (available.length < maxSize) {
            obj.reset();
            available.push(obj);
        }
    }
}

Void main() {
    println("=== Memory Management Demo ===\n");
    
    // Arena allocation
    println("--- Arena Allocator ---");
    let arena = Arena(1024);
    
    let ptr1 = arena.allocate(100);
    let ptr2 = arena.allocate(200);
    println("Allocated at offsets 0 and 100");
    
    arena.reset();
    let ptr3 = arena.allocate(150);
    println("After reset, allocated at offset 0");
    
    // Reference counting
    println("\n--- Reference Counting ---");
    let rc1 = Rc<Int>(42);
    println("Value: " + rc1.get().toString());
    println("Ref count: " + rc1.referenceCount().toString());
    
    let rc2 = rc1.clone();
    println("After clone, ref count: " + rc1.referenceCount().toString());
    
    // Object pool
    println("\n--- Object Pool ---");
    let pool = ObjectPool<StringBuilder>(5, StringBuilder());
    
    let sb1 = pool.acquire();
    sb1.append("Hello");
    println("Acquired: " + sb1.toString());
    
    pool.release(sb1);
    println("Released back to pool");
}
```

---

## Contributing

Have a great recipe to share? Contribute to the cookbook!

1. Fork the repository
2. Create a new branch for your recipe
3. Add your example following the existing format
4. Submit a pull request

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## License

This cookbook is part of the Zenith project and is licensed under the MIT License. See [LICENSE](../LICENSE) for details.

---

**Happy Coding with Zenith! ⚡**
