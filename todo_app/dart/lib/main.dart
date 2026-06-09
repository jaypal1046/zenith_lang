// ZENITH DART MAIN
// Entry point for Flutter app with Plugin System

import 'package:flutter/material.dart';
import 'dart:ffi';
import 'dart:io';
import 'plugin_wrappers.dart';
import 'zenith_native_bindings.dart';

void main() {
  // Register all Dart plugin wrappers
  registerAllPlugins();
  
  runApp(ZenithApp());
}

class ZenithApp extends StatefulWidget {
  @override
  _ZenithAppState createState() => _ZenithAppState();
}

class _ZenithAppState extends State<ZenithApp> {
  late ZenithNativeBindings _native;
  String _output = "Zenith Plugin System Ready\n";
  bool _isInitialized = false;

  @override
  void initState() {
    super.initState();
    _initZenith();
  }

  Future<void> _initZenith() async {
    try {
      // Load native library
      final dylib = Platform.isAndroid 
          ? DynamicLibrary.open("libzenith_native.so")
          : DynamicLibrary.process();
      
      _native = ZenithNativeBindings(dylib);
      
      // Initialize runtime
      _native.zenith_native_init();
      
      setState(() {
        _isInitialized = true;
        _output += "✓ Native runtime initialized\n";
        _output += "✓ Plugins registered\n";
      });
      
      // Run example Zenith code with plugins
      await _runPluginExample();
    } catch (e) {
      setState(() {
        _output += "✗ Initialization error: $e\n";
      });
    }
  }

  Future<void> _runPluginExample() async {
    setState(() {
      _output += "\n--- Running Plugin Example ---\n";
    });

    // Example 1: Call Location Plugin (Dart/pub.dev)
    try {
      var locationResult = await _callPlugin(
        "location",
        "get_current_position",
        {"accuracy": "high", "timeout": 5000}
      );
      setState(() {
        _output += "📍 Location: $locationResult\n";
      });
    } catch (e) {
      setState(() {
        _output += "✗ Location error: $e\n";
      });
    }

    // Example 2: Call Crypto Plugin (Rust crate)
    try {
      var hashResult = _native.zenith_native_call_function(
        "crypto_sha256",
        "Hello Zenith"
      );
      setState(() {
        _output += "🔐 SHA256: $hashResult\n";
      });
    } catch (e) {
      setState(() {
        _output += "✗ Crypto error: $e\n";
      });
    }

    setState(() {
      _output += "\n✓ Plugin system operational\n";
    });
  }

  Future<String> _callPlugin(
    String pluginName,
    String method,
    Map<String, dynamic> args
  ) async {
    // This would use MethodChannel in real implementation
    // For now, simulate plugin call
    await Future.delayed(Duration(milliseconds: 100));
    return '{"status": "success", "plugin": "$pluginName"}';
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text('Zenith - Plugin System'),
          backgroundColor: Colors.deepPurple,
        ),
        body: Container(
          color: Colors.black,
          child: SingleChildScrollView(
            child: Padding(
              padding: EdgeInsets.all(16),
              child: Text(
                _output,
                style: TextStyle(
                  fontFamily: 'monospace',
                  fontSize: 14,
                  color: Colors.greenAccent,
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
