# Dart Packages - Using Dart Ecosystem in Zenith

Learn how to leverage the extensive Dart package ecosystem in your Zenith applications through auto-generated bindings. This guide shows you how to use popular Dart packages while maintaining 100% Zenith code.

## Overview

Zenith's Dart bridge (`zenith_dart`) provides:
- Automatic binding generation for Dart packages
- Full access to pub.dev ecosystem (20,000+ packages)
- Null safety support
- Hot reload compatibility
- Type-safe interop

## Prerequisites

- Zenith SDK installed
- Basic understanding of Zenith syntax
- Familiarity with Dart packages (optional)

## Installation

### Step 1: Add Dart Bridge Dependencies

In your `pubspec.yaml`:

```yaml
dependencies:
  zenith_dart: ^1.0.0
  build_runner: ^2.4.0
  zenith_dart_generator: ^1.0.0

dev_dependencies:
  build_runner: ^2.4.0
```

Run:
```bash
zenith pub get
```

### Step 2: Configure Package Bindings

Create `build.yaml`:

```yaml
targets:
  $default:
    builders:
      zenith_dart_generator:
        options:
          packages:
            - http
            - provider
            - shared_preferences
            - firebase_core
            - cloud_firestore
          output_dir: lib/generated/
          null_safety: true
```

### Step 3: Generate Bindings

```bash
zenith run build_runner build --delete-conflicting-outputs
```

This generates type-safe bindings for all specified packages.

## Basic Usage

### Example: HTTP Client

**Step 1**: Add to `build.yaml`:
```yaml
packages:
  - http
```

**Step 2**: Generate bindings:
```bash
zenith run build_runner build
```

**Step 3**: Use in Zenith code:

```zenith
import 'package:zenith_ui/zenith_ui.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';

void main() {
  runApp(HttpExampleApp());
}

class HttpExampleApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: HttpDemo(),
    );
  }
}

class HttpDemo extends StatefulWidget {
  @override
  _HttpDemoState createState() => _HttpDemoState();
}

class _HttpDemoState extends State<HttpDemo> {
  String _data = 'Click button to fetch data';
  bool _isLoading = false;
  
  Future<void> fetchData() async {
    setState(() {
      _isLoading = true;
    });
    
    try {
      // Use Dart http package directly
      final response = await http.get(
        Uri.parse('https://jsonplaceholder.typicode.com/posts/1'),
      );
      
      if (response.statusCode == 200) {
        final jsonData = jsonDecode(response.body);
        setState(() {
          _data = 'Title: ${jsonData['title']}\n\nBody: ${jsonData['body']}';
          _isLoading = false;
        });
      } else {
        setState(() {
          _data = 'Error: ${response.statusCode}';
          _isLoading = false;
        });
      }
    } catch (e) {
      setState(() {
        _data = 'Exception: $e';
        _isLoading = false;
      });
    }
  }
  
  Future<void> postData() async {
    try {
      final response = await http.post(
        Uri.parse('https://jsonplaceholder.typicode.com/posts'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({
          'title': 'My Post',
          'body': 'This is the content',
          'userId': 1,
        }),
      );
      
      print('Status: ${response.statusCode}');
      print('Response: ${response.body}');
    } catch (e) {
      print('Error: $e');
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('HTTP Example'),
      ),
      body: Padding(
        padding: EdgeInsets.all(20),
        child: Column(
          children: [
            ElevatedButton(
              onPressed: _isLoading ? null : fetchData,
              child: _isLoading 
                ? CircularProgressIndicator() 
                : Text('Fetch Data'),
            ),
            SizedBox(height: 10),
            ElevatedButton(
              onPressed: postData,
              child: Text('Post Data'),
            ),
            SizedBox(height: 20),
            Expanded(
              child: Container(
                padding: EdgeInsets.all(15),
                decoration: BoxDecoration(
                  color: Colors.grey[200],
                  borderRadius: BorderRadius.circular(8),
                ),
                child: SingleChildScrollView(
                  child: Text(_data),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
```

## Popular Package Examples

### 1. State Management with Provider

**Add to `build.yaml`**:
```yaml
packages:
  - provider
```

**Zenith Code**:

```zenith
import 'package:zenith_ui/zenith_ui.dart';
import 'package:provider/provider.dart';

void main() {
  runApp(
    ChangeNotifierProvider(
      create: (_) => CounterModel(),
      child: ProviderApp(),
    ),
  );
}

class CounterModel extends ChangeNotifier {
  int _count = 0;
  
  int get count => _count;
  
  void increment() {
    _count++;
    notifyListeners();
  }
  
  void decrement() {
    _count--;
    notifyListeners();
  }
}

class ProviderApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('Provider Example')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Consumer<CounterModel>(
                builder: (context, counter, child) {
                  return Text(
                    'Count: ${counter.count}',
                    style: TextStyle(fontSize: 48),
                  );
                },
              ),
              SizedBox(height: 20),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  ElevatedButton(
                    onPressed: () {
                      context.read<CounterModel>().decrement();
                    },
                    child: Icon(Icons.remove),
                  ),
                  SizedBox(width: 20),
                  ElevatedButton(
                    onPressed: () {
                      context.read<CounterModel>().increment();
                    },
                    child: Icon(Icons.add),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```

### 2. Local Storage with SharedPreferences

**Add to `build.yaml`**:
```yaml
packages:
  - shared_preferences
```

**Zenith Code**:

```zenith
import 'package:zenith_ui/zenith_ui.dart';
import 'package:shared_preferences/shared_preferences.dart';

class StorageDemo extends StatefulWidget {
  @override
  _StorageDemoState createState() => _StorageDemoState();
}

class _StorageDemoState extends State<StorageDemo> {
  TextEditingController _controller = TextEditingController();
  String _savedData = '';
  
  Future<void> saveData() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('user_input', _controller.text);
    
    setState(() {
      _savedData = 'Saved: ${_controller.text}';
    });
    
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text('Data saved!')),
    );
  }
  
  Future<void> loadData() async {
    final prefs = await SharedPreferences.getInstance();
    final data = prefs.getString('user_input') ?? 'No data saved';
    
    setState(() {
      _savedData = 'Loaded: $data';
    });
  }
  
  Future<void> clearData() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.clear();
    
    setState(() {
      _savedData = 'Data cleared';
      _controller.clear();
    });
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('SharedPreferences')),
      body: Padding(
        padding: EdgeInsets.all(20),
        child: Column(
          children: [
            TextField(
              controller: _controller,
              decoration: InputDecoration(
                labelText: 'Enter text to save',
                border: OutlineInputBorder(),
              ),
            ),
            SizedBox(height: 20),
            Row(
              children: [
                ElevatedButton(
                  onPressed: saveData,
                  child: Text('Save'),
                ),
                SizedBox(width: 10),
                ElevatedButton(
                  onPressed: loadData,
                  child: Text('Load'),
                ),
                SizedBox(width: 10),
                ElevatedButton(
                  onPressed: clearData,
                  child: Text('Clear'),
                ),
              ],
            ),
            SizedBox(height: 20),
            Text(_savedData),
          ],
        ),
      ),
    );
  }
}
```

### 3. Firebase Integration

**Add to `build.yaml`**:
```yaml
packages:
  - firebase_core
  - cloud_firestore
```

**Zenith Code**:

```zenith
import 'package:zenith_ui/zenith_ui.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:cloud_firestore/cloud_firestore.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(FirebaseApp());
}

class FirebaseApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: FirestoreDemo(),
    );
  }
}

class FirestoreDemo extends StatefulWidget {
  @override
  _FirestoreDemoState createState() => _FirestoreDemoState();
}

class _FirestoreDemoState extends State<FirestoreDemo> {
  final FirebaseFirestore _firestore = FirebaseFirestore.instance;
  TextEditingController _nameController = TextEditingController();
  
  Future<void> addDocument() async {
    await _firestore.collection('users').add({
      'name': _nameController.text,
      'timestamp': FieldValue.serverTimestamp(),
    });
    
    _nameController.clear();
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text('User added!')),
    );
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Firebase Firestore')),
      body: Column(
        children: [
          Padding(
            padding: EdgeInsets.all(20),
            child: Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: _nameController,
                    decoration: InputDecoration(
                      labelText: 'User Name',
                      border: OutlineInputBorder(),
                    ),
                  ),
                ),
                SizedBox(width: 10),
                ElevatedButton(
                  onPressed: addDocument,
                  child: Text('Add'),
                ),
              ],
            ),
          ),
          Expanded(
            child: StreamBuilder<QuerySnapshot>(
              stream: _firestore
                  .collection('users')
                  .orderBy('timestamp', descending: true)
                  .snapshots(),
              builder: (context, snapshot) {
                if (snapshot.hasError) {
                  return Text('Error: ${snapshot.error}');
                }
                
                if (snapshot.connectionState == ConnectionState.waiting) {
                  return CircularProgressIndicator();
                }
                
                return ListView(
                  children: snapshot.data!.docs.map((doc) {
                    return ListTile(
                      title: Text(doc['name']),
                      subtitle: Text('ID: ${doc.id}'),
                    );
                  }).toList(),
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}
```

## Building for Different Platforms

### Desktop

```bash
zenith build desktop --release
```

### Mobile

```bash
# iOS
zenith build ios --release

# Android
zenith build android --release
```

Note: Some packages may require platform-specific configuration. Check package documentation.

### Web

```bash
zenith build web --release
```

Note: Not all Dart packages work on web. Check pub.dev for web compatibility.

## Creating Custom Bindings

For packages not in your build.yaml, you can create manual bindings:

```zenith
// lib/bindings/custom_package.zenith
@JS()
library custom_package;

import 'package:js/js.dart';

@JS('CustomFunction')
external dynamic customFunction(String param);

@JS('CustomClass')
class CustomClass {
  external factory CustomClass();
  external void doSomething();
  external String getResult();
}
```

## Best Practices

1. **Generate bindings upfront** - Run build_runner before development
2. **Check package compatibility** - Verify platform support on pub.dev
3. **Use null safety** - Enable null_safety in build.yaml
4. **Minimize generated files** - Only include needed packages
5. **Version lock dependencies** - Pin specific versions in pubspec.yaml

## Common Issues

### Binding Generation Fails

**Solution**: Ensure package has proper exports and is compatible:
```bash
# Clean and regenerate
zenith clean
zenith run build_runner build --delete-conflicting-outputs
```

### Package Not Found at Runtime

**Solution**: Add package to pubspec.yaml dependencies:
```yaml
dependencies:
  http: ^1.1.0  # Must be in pubspec.yaml
```

### Type Errors

**Solution**: Check generated binding types match expected usage:
```zenith
// Generated binding might return dynamic
final result = await http.get(url) as http.Response;
```

## Performance Considerations

| Operation | Overhead | Recommendation |
|-----------|----------|----------------|
| Simple function call | <1ms | ✅ Great |
| Complex object marshaling | 5-10ms | ⚠️ Use sparingly |
| Stream events | 2-5ms/event | ✅ Good for UI |
| Large data transfer | 10-50ms | ⚠️ Consider Rust bridge |

## Next Steps

- [Animation Libraries](05-animations.md) - Beautiful animations
- [Firebase Advanced](04-firebase.md) - Complete Firebase setup
- [Rust Integration](../../05-rust-integration/01-setup.md) - For performance-critical code
- [State Management Patterns](../../04-advanced/01-state-management.md) - Architecture patterns

## Resources

- [pub.dev - Dart Packages](https://pub.dev)
- [Zenith Dart Bridge API](../../docs/api/zenith_dart.md)
- [Build Runner Documentation](https://pub.dev/packages/build_runner)
- [Example Projects](https://github.com/zenith-lang/dart-examples)

---

**Built with 100% Zenith Code** (leveraging Dart ecosystem via auto-bindings) 🎯

*Access 20,000+ Dart packages while writing pure Zenith!*
