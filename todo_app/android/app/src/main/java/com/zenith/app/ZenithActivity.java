package com.zenith.app;

import android.app.NativeActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import java.util.HashMap;
import java.util.Map;

/**
 * Main Activity for Zenith Android App
 * Integrates native C++ runtime with Dart/Flutter via JNI
 * Supports plugin architecture for location, camera, sensors, etc.
 */
public class ZenithActivity extends NativeActivity {

    private static final String TAG = "ZenithActivity";
    
    // Map of registered plugin channels
    private Map<String, Object> pluginChannels = new HashMap<>();

    static {
        // Load the native libraries
        System.loadLibrary("zenith_native");
        System.loadLibrary("zenith_app");
        Log.i(TAG, "Native libraries loaded");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "ZenithActivity onCreate");
        super.onCreate(savedInstanceState);

        // Initialize the native runtime with app data path
        String appDataPath = getApplicationInfo().dataDir;
        nativeInit(appDataPath);
        
        // Initialize plugin host with JVM and Activity instance
        initPluginHost(getApplication());
        
        Log.i(TAG, "Native runtime initialized at: " + appDataPath);
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "ZenithActivity onDestroy");
        shutdownPluginHost();
        nativeShutdown();
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "ZenithActivity onPause");
        nativeOnPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "ZenithActivity onResume");
        nativeOnResume();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();
        int action = event.getAction();

        // Pass touch events to native layer
        nativeOnTouch(x, y, action);

        return true;
    }

    // ------------------------------------------------------------------
    // Native Methods (JNI to C++ Runtime)
    // ------------------------------------------------------------------
    
    private native int nativeInit(String appDataPath);
    private native void nativeOnPause();
    private native void nativeOnResume();
    private native void nativeShutdown();
    private native void nativeOnTouch(float x, float y, int action);
    private native String nativeExecute(String code);
    private native String nativeCallFunction(String funcName, String argsJson);
    private native String nativeGetState();
    private native int nativeSetState(String stateJson);
    
    // Plugin Host Native Methods
    private native void initPluginHost(Object app);
    private native void shutdownPluginHost();
    private native void pushEventToVM(String eventName, String dataJson);

    // ------------------------------------------------------------------
    // Methods callable from Dart via JNI
    // ------------------------------------------------------------------
    
    public String executeCode(String code) {
        return nativeExecute(code);
    }

    public String callRustFunction(String funcName, String argsJson) {
        return nativeCallFunction(funcName, argsJson);
    }

    public String getState() {
        return nativeGetState();
    }

    public int setState(String stateJson) {
        return nativeSetState(stateJson);
    }
    
    // ------------------------------------------------------------------
    // Dart Plugin Interface (Called from C++ via JNI)
    // ------------------------------------------------------------------
    
    /**
     * Called from C++ to invoke a Dart plugin method
     * @param pluginName Name of the plugin (e.g., "location", "camera")
     * @param method Method to call (e.g., "getCurrentLocation")
     * @param argsJson JSON arguments
     * @return JSON result string
     */
    public String callDartPlugin(String pluginName, String method, String argsJson) {
        Log.i(TAG, "Calling Dart plugin: " + pluginName + "." + method + " with args: " + argsJson);
        
        // Handle built-in plugins using Android APIs
        // In production: Use Flutter Platform Channels to call actual pub.dev plugins
        
        switch (pluginName) {
            case "location":
                return handleLocationPlugin(method, argsJson);
            case "camera":
                return handleCameraPlugin(method, argsJson);
            case "sensors":
                return handleSensorsPlugin(method, argsJson);
            case "storage":
                return handleStoragePlugin(method, argsJson);
            default:
                // Try to call via Flutter platform channel if registered
                return callFlutterPlugin(pluginName, method, argsJson);
        }
    }
    
    // ------------------------------------------------------------------
    // Built-in Plugin Implementations
    // ------------------------------------------------------------------
    
    private String handleLocationPlugin(String method, String argsJson) {
        if ("getCurrentLocation".equals(method)) {
            // Simulate getting location (in production: use FusedLocationProviderClient)
            // Would require android.permission.ACCESS_FINE_LOCATION
            return "{\"latitude\":37.7749,\"longitude\":-122.4194,\"accuracy\":10.0,\"timestamp\":" + System.currentTimeMillis() + "}";
        } else if ("startListening".equals(method)) {
            // Start continuous location updates
            // Would trigger events via zenith_push_event_to_vm()
            return "{\"status\":\"listening\",\"interval\":1000}";
        } else if ("stopListening".equals(method)) {
            return "{\"status\":\"stopped\"}";
        }
        return "{\"error\":\"Unknown location method\"}";
    }
    
    private String handleCameraPlugin(String method, String argsJson) {
        if ("takePicture".equals(method)) {
            // Simulate taking a picture (in production: use CameraX or camera plugin)
            String photoPath = getExternalFilesDir(null).getAbsolutePath() + "/zenith_photo_" + System.currentTimeMillis() + ".jpg";
            return "{\"path\":\"" + photoPath + "\",\"width\":1920,\"height\":1080,\"format\":\"jpeg\"}";
        } else if ("startPreview".equals(method)) {
            return "{\"status\":\"preview_started\"}";
        } else if ("stopPreview".equals(method)) {
            return "{\"status\":\"preview_stopped\"}";
        }
        return "{\"error\":\"Unknown camera method\"}";
    }
    
    private String handleSensorsPlugin(String method, String argsJson) {
        if ("getAccelerometer".equals(method)) {
            // Simulate accelerometer data (in production: use SensorManager)
            return "{\"x\":0.1,\"y\":9.8,\"z\":0.05,\"timestamp\":" + System.currentTimeMillis() + "}";
        } else if ("getGyroscope".equals(method)) {
            return "{\"x\":0.0,\"y\":0.1,\"z\":0.0,\"timestamp\":" + System.currentTimeMillis() + "}";
        } else if ("getMagnetometer".equals(method)) {
            return "{\"x\":20.5,\"y\":-10.2,\"z\":45.8,\"timestamp\":" + System.currentTimeMillis() + "}";
        }
        return "{\"error\":\"Unknown sensor method\"}";
    }
    
    private String handleStoragePlugin(String method, String argsJson) {
        if ("readFile".equals(method)) {
            // Would implement file reading from internal/external storage
            return "{\"content\":\"file_content_here\",\"size\":1024}";
        } else if ("writeFile".equals(method)) {
            return "{\"status\":\"written\",\"bytes\":256}";
        } else if ("listFiles".equals(method)) {
            return "{\"files\":[\"file1.txt\",\"file2.dat\",\"photo.jpg\"]}";
        }
        return "{\"error\":\"Unknown storage method\"}";
    }
    
    // ------------------------------------------------------------------
    // Flutter Platform Channel Integration
    // ------------------------------------------------------------------
    
    /**
     * Call a Flutter plugin via platform channel
     * In production: This would use MethodChannel to communicate with Dart code
     */
    private String callFlutterPlugin(String pluginName, String method, String argsJson) {
        Log.w(TAG, "Flutter plugin not implemented yet: " + pluginName + "." + method);
        // TODO: Implement MethodChannel communication with Flutter
        // This would allow using any pub.dev plugin like:
        // - geolocator
        // - camera
        // - image_picker
        // - shared_preferences
        // - http
        // - firebase_core, firebase_auth, etc.
        return "{\"error\":\"Flutter plugin integration pending: \" + pluginName + "}";
    }
    
    // ------------------------------------------------------------------
    // Event Callbacks (From Dart/Rust to Native to Zenith VM)
    // ------------------------------------------------------------------
    
    /**
     * Called when a Dart/Rust plugin has an event to push to Zenith VM
     * This would be called from Flutter via MethodChannel or from Rust directly
     */
    public void onPluginEvent(String eventName, String dataJson) {
        Log.i(TAG, "Plugin event received: " + eventName + " -> " + dataJson);
        // Call native function to push event to Zenith VM
        pushEventToVM(eventName, dataJson);
    }
}
