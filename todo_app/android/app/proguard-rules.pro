# Zenith App ProGuard Rules

# Keep native methods in ZenithActivity
-keepclassmembers class com.zenith.app.ZenithActivity {
    private native <methods>;
}

# Keep Rust FFI classes
-keep class com.zenith.rust.** { *; }

# Keep Flutter engine classes
-keep class io.flutter.** { *; }

# Keep Dart generated classes
-keep class com.zenith.app.BuildConfig { *; }

# Optimization settings
-optimizations !code/simplification/arithmetic,!field/*,!class/merging/*
-optimizationpasses 5
