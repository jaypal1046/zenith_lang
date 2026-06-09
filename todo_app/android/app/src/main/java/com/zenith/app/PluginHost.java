package com.zenith.app;

import android.app.Activity;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import androidx.annotation.NonNull;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.plugin.common.MethodChannel;
import java.util.HashMap;
import java.util.Map;

/**
 * PluginHost - Bridges Zenith Runtime with Dart Plugins (pub.dev)
 * 
 * Architecture:
 * [Zenith Code] -> [C++ Runtime] -> [JNI] -> [PluginHost] -> [MethodChannel] -> [Dart Plugin]
 */
public class PluginHost {
    
    private static final String TAG = "ZenithPluginHost";
    private static final String CHANNEL_PREFIX = "com.zenith.plugin/";
    
    private final Activity activity;
    private final Map<String, MethodChannel> registeredPlugins;
    private final Handler mainHandler;
    
    public PluginHost(Activity activity) {
        this.activity = activity;
        this.registeredPlugins = new HashMap<>();
        this.mainHandler = new Handler(Looper.getMainLooper());
    }
    
    /**
     * Registers a Dart plugin with the Zenith runtime.
     * Called from C++ via JNI when Zenith code imports a plugin.
     */
    public void registerPlugin(String zenithName, String dartChannelName) {
        mainHandler.post(() -> {
            try {
                MethodChannel channel = new MethodChannel(
                    FlutterEngine(activity).getDartExecutor().getBinaryMessenger(),
                    CHANNEL_PREFIX + dartChannelName
                );
                
                registeredPlugins.put(zenithName, channel);
                System.out.println(TAG + ": Registered plugin '" + zenithName + "' -> '" + dartChannelName + "'");
            } catch (Exception e) {
                System.err.println(TAG + ": Failed to register plugin: " + e.getMessage());
            }
        });
    }
    
    /**
     * Invokes a method on a registered Dart plugin.
     * @param zenithName Name used in Zenith code
     * @param method Method to call
     * @param argsJson JSON arguments from Zenith
     * @param callback Result callback (called on main thread)
     */
    public void invokePlugin(
        String zenithName, 
        String method, 
        String argsJson,
        PluginResultCallback callback
    ) {
        MethodChannel channel = registeredPlugins.get(zenithName);
        
        if (channel == null) {
            callback.onError("Plugin not registered: " + zenithName);
            return;
        }
        
        mainHandler.post(() -> {
            Map<String, Object> args = new HashMap<>();
            args.put("method", method);
            args.put("args_json", argsJson);
            
            channel.invokeMethod("call", args, new MethodChannel.Result() {
                @Override
                public void success(Object result) {
                    callback.onSuccess(result != null ? result.toString() : "{}");
                }
                
                @Override
                public void error(String errorCode, String errorMessage, Object errorDetails) {
                    callback.onError(errorMessage != null ? errorMessage : errorCode);
                }
                
                @Override
                public void notImplemented() {
                    callback.onError("Method not implemented: " + method);
                }
            });
        });
    }
    
    /**
     * Callback interface for plugin results.
     */
    public interface PluginResultCallback {
        void onSuccess(String jsonResult);
        void onError(String errorMessage);
    }
}
