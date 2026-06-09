import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

/// FFI bindings for Zenith native Rust library
class ZenithNative {
  late final DynamicLibrary _lib;
  late final ZenithNativeBindings _bindings;

  ZenithNative() {
    if (Platform.isAndroid) {
      _lib = DynamicLibrary.open('libzenith_native.so');
    } else if (Platform.isLinux) {
      _lib = DynamicLibrary.open('libzenith_native.so');
    } else if (Platform.isMacOS) {
      _lib = DynamicLibrary.open('libzenith_native.dylib');
    } else if (Platform.isWindows) {
      _lib = DynamicLibrary.open('zenith_native.dll');
    } else {
      throw UnsupportedError('Unsupported platform: ${Platform.operatingSystem}');
    }
    
    _bindings = ZenithNativeBindings(_lib);
  }

  /// Initialize the Zenith runtime
  int init(String appDataPath) {
    final pathPtr = appDataPath.toNativeUtf8();
    try {
      return _bindings.zenith_native_init(pathPtr.cast());
    } finally {
      calloc.free(pathPtr);
    }
  }

  /// Execute Zenith code and return JSON result
  String execute(String code) {
    final codePtr = code.toNativeUtf8();
    try {
      final resultPtr = _bindings.zenith_native_execute(codePtr.cast());
      try {
        return resultPtr.cast<Utf8>().toDartString();
      } finally {
        _bindings.zenith_native_free(resultPtr.cast());
      }
    } finally {
      calloc.free(codePtr);
    }
  }

  /// Call a Rust function by name with JSON arguments
  String callFunction(String funcName, String argsJson) {
    final funcNamePtr = funcName.toNativeUtf8();
    final argsPtr = argsJson.toNativeUtf8();
    try {
      final resultPtr = _bindings.zenith_native_call_function(
        funcNamePtr.cast(),
        argsPtr.cast(),
      );
      try {
        return resultPtr.cast<Utf8>().toDartString();
      } finally {
        _bindings.zenith_native_free(resultPtr.cast());
      }
    } finally {
      calloc.free(funcNamePtr);
      calloc.free(argsPtr);
    }
  }

  /// Process a touch event
  void onTouch(double x, double y, int action) {
    _bindings.zenith_native_on_touch(x, y, action);
  }

  /// Process a touch move event
  void onTouchMove(double x, double y) {
    _bindings.zenith_native_on_touch_move(x, y);
  }

  /// Get current state as JSON
  String getState() {
    final resultPtr = _bindings.zenith_native_get_state();
    try {
      return resultPtr.cast<Utf8>().toDartString();
    } finally {
      _bindings.zenith_native_free(resultPtr.cast());
    }
  }

  /// Set state from JSON
  int setState(String stateJson) {
    final statePtr = stateJson.toNativeUtf8();
    try {
      return _bindings.zenith_native_set_state(statePtr.cast());
    } finally {
      calloc.free(statePtr);
    }
  }

  /// Shutdown the runtime
  void shutdown() {
    _bindings.zenith_native_shutdown();
  }

  /// Render UI to pixel buffer (returns RGBA data)
  Uint8List render(int width, int height) {
    final stridePtr = calloc<Int32>();
    try {
      final bufferPtr = _bindings.zenith_native_render(width, height, stridePtr);
      if (bufferPtr == nullptr) {
        return Uint8List(0);
      }
      
      final stride = stridePtr.value;
      final size = width * height * 4;
      
      final buffer = Uint8List.fromList(
        bufferPtr.cast<Uint8>().asTypedList(size),
      );
      
      // Free the native buffer after copying
      _bindings.zenith_native_free(bufferPtr.cast());
      
      return buffer;
    } finally {
      calloc.free(stridePtr);
    }
  }
}

/// Native bindings definition
class ZenithNativeBindings {
  final DynamicLibrary lib;

  ZenithNativeBindings(this.lib);

  /// int zenith_native_init(const char* app_data_path)
  final zenith_native_init = lib
      .lookupFunction<Int32 Function(Pointer<Int8>),
          int Function(Pointer<Int8>)>('zenith_native_init');

  /// char* zenith_native_execute(const char* code)
  final zenith_native_execute = lib
      .lookupFunction<Pointer<Int8> Function(Pointer<Int8>),
          Pointer<Int8> Function(Pointer<Int8>)>('zenith_native_execute');

  /// char* zenith_native_call_function(const char* func_name, const char* args_json)
  final zenith_native_call_function = lib
      .lookupFunction<
          Pointer<Int8> Function(Pointer<Int8>, Pointer<Int8>),
          Pointer<Int8> Function(Pointer<Int8>, Pointer<Int8>)>(
              'zenith_native_call_function');

  /// void zenith_native_on_touch(float x, float y, int action)
  final zenith_native_on_touch = lib
      .lookupFunction<Void Function(Float, Float, Int32),
          void Function(double, double, int)>('zenith_native_on_touch');

  /// void zenith_native_on_touch_move(float x, float y)
  final zenith_native_on_touch_move = lib
      .lookupFunction<Void Function(Float, Float),
          void Function(double, double)>('zenith_native_on_touch_move');

  /// void* zenith_native_render(int width, int height, int* out_stride)
  final zenith_native_render = lib
      .lookupFunction<Pointer<Void> Function(Int32, Int32, Pointer<Int32>),
          Pointer<Void> Function(int, int, Pointer<Int32>)>(
              'zenith_native_render');

  /// void zenith_native_free(void* ptr)
  final zenith_native_free = lib
      .lookupFunction<Void Function(Pointer<Void>),
          void Function(Pointer<Void>)>('zenith_native_free');

  /// char* zenith_native_get_state()
  final zenith_native_get_state = lib
      .lookupFunction<Pointer<Int8> Function(),
          Pointer<Int8> Function()>('zenith_native_get_state');

  /// int zenith_native_set_state(const char* state_json)
  final zenith_native_set_state = lib
      .lookupFunction<Int32 Function(Pointer<Int8>),
          int Function(Pointer<Int8>)>('zenith_native_set_state');

  /// void zenith_native_shutdown()
  final zenith_native_shutdown = lib
      .lookupFunction<Void Function(), void Function()>(
          'zenith_native_shutdown');
}
