//! Zenith Native Rust Library
//! Provides high-performance native functions for Zenith apps
//! 
//! Build for Android:
//!   cargo ndk -t arm64-v8a -t armeabi-v7a -o android/app/src/main/jniLibs build --release
//!
//! Build for Desktop:
//!   cargo build --release

#[cfg(target_os = "android")]
use android_logger::Config;
#[cfg(target_os = "android")]
use log::{info, error};

use serde::{Deserialize, Serialize};
use serde_json;
use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::sync::Mutex;

/// Result structure for Rust function calls
#[derive(Debug, Serialize, Deserialize)]
pub struct RustResult {
    pub success: bool,
    pub data: String,
    pub error: Option<String>,
}

impl RustResult {
    pub fn ok(data: &str) -> Self {
        RustResult {
            success: true,
            data: data.to_string(),
            error: None,
        }
    }
    
    pub fn err(error: &str) -> Self {
        RustResult {
            success: false,
            data: String::new(),
            error: Some(error.to_string()),
        }
    }
}

/// Global state for the Zenith runtime
static ZENITH_STATE: Mutex<Option<ZenithState>> = Mutex::new(None);

struct ZenithState {
    initialized: bool,
    app_data_path: String,
    touch_x: f32,
    touch_y: f32,
    last_action: i32,
}

impl ZenithState {
    fn new(app_data_path: &str) -> Self {
        ZenithState {
            initialized: true,
            app_data_path: app_data_path.to_string(),
            touch_x: 0.0,
            touch_y: 0.0,
            last_action: 0,
        }
    }
}

/// Initialize the Rust runtime with app data path
#[no_mangle]
pub extern "C" fn zenith_native_init(app_data_path: *const c_char) -> i32 {
    unsafe {
        let path = if app_data_path.is_null() {
            "/data/data/zenith.app"
        } else {
            CStr::from_ptr(app_data_path).to_str().unwrap_or("/data/data/zenith.app")
        };
        
        #[cfg(target_os = "android")]
        {
            android_logger::init_once(
                Config::default()
                    .with_max_level(log::LevelFilter::Info)
                    .with_tag("ZenithRust"),
            );
            info!("Zenith Rust runtime initialized on Android at: {}", path);
        }
        
        #[cfg(not(target_os = "android"))]
        {
            println!("Zenith Rust runtime initialized on desktop at: {}", path);
        }
        
        let mut state = ZENITH_STATE.lock().unwrap();
        *state = Some(ZenithState::new(path));
        
        0 // Success
    }
}

/// Execute Zenith code and return result
#[no_mangle]
pub extern "C" fn zenith_native_execute(code: *const c_char) -> *mut c_char {
    unsafe {
        if code.is_null() {
            let result = RustResult::err("Null code");
            let json = serde_json::to_string(&result).unwrap_or_default();
            return CString::new(json).unwrap().into_raw();
        }
        
        let c_str = CStr::from_ptr(code);
        let code_str = match c_str.to_str() {
            Ok(s) => s,
            Err(_) => {
                let result = RustResult::err("Invalid UTF-8 in code");
                let json = serde_json::to_string(&result).unwrap_or_default();
                return CString::new(json).unwrap().into_raw();
            }
        };
        
        #[cfg(target_os = "android")]
        info!("Executing Zenith code: {}", code_str);
        
        // For now, echo the code back as a simple interpreter simulation
        // In production, this would call the actual Zenith transpiler/interpreter
        let result = RustResult::ok(&format!("Executed: {}", code_str));
        let json = serde_json::to_string(&result).unwrap_or_default();
        
        CString::new(json).unwrap().into_raw()
    }
}

/// Call a Rust function by name with JSON arguments
#[no_mangle]
pub extern "C" fn zenith_native_call_function(
    func_name: *const c_char,
    args_json: *const c_char,
) -> *mut c_char {
    unsafe {
        let name = if func_name.is_null() {
            "unknown"
        } else {
            CStr::from_ptr(func_name).to_str().unwrap_or("unknown")
        };
        
        let args = if args_json.is_null() {
            "{}"
        } else {
            CStr::from_ptr(args_json).to_str().unwrap_or("{}")
        };
        
        #[cfg(target_os = "android")]
        info!("Calling Rust function '{}' with args: {}", name, args);
        
        // Route to appropriate function
        let result = match name {
            "process_string" => {
                let result = RustResult::ok(&format!("Called process_string with: {}", args));
                serde_json::to_string(&result).unwrap_or_default()
            },
            "fibonacci" => {
                #[derive(Deserialize)]
                struct FibArgs { n: u32 }
                
                match serde_json::from_str::<FibArgs>(args) {
                    Ok(fib_args) => {
                        let fib_result = fibonacci(fib_args.n);
                        let result = RustResult::ok(&fib_result.to_string());
                        serde_json::to_string(&result).unwrap_or_default()
                    }
                    Err(e) => {
                        let result = RustResult::err(&format!("Parse error: {}", e));
                        serde_json::to_string(&result).unwrap_or_default()
                    }
                }
            },
            "get_state" => {
                let state = ZENITH_STATE.lock().unwrap();
                match &*state {
                    Some(s) => {
                        let result = RustResult::ok(&format!(
                            "{{\"initialized\": {}, \"path\": \"{}\"}}",
                            s.initialized, s.app_data_path
                        ));
                        serde_json::to_string(&result).unwrap_or_default()
                    }
                    None => {
                        let result = RustResult::err("Not initialized");
                        serde_json::to_string(&result).unwrap_or_default()
                    }
                }
            },
            _ => {
                let result = RustResult::err(&format!("Unknown function: {}", name));
                serde_json::to_string(&result).unwrap_or_default()
            }
        };
        
        CString::new(result).unwrap().into_raw()
    }
}

/// Process a touch event
#[no_mangle]
pub extern "C" fn zenith_native_on_touch(x: f32, y: f32, action: i32) {
    let mut state = ZENITH_STATE.lock().unwrap();
    if let Some(ref mut s) = *state {
        s.touch_x = x;
        s.touch_y = y;
        s.last_action = action;
        
        #[cfg(target_os = "android")]
        info!("Touch event: ({}, {}) action={}", x, y, action);
    }
}

/// Process a touch move event
#[no_mangle]
pub extern "C" fn zenith_native_on_touch_move(x: f32, y: f32) {
    let mut state = ZENITH_STATE.lock().unwrap();
    if let Some(ref mut s) = *state {
        s.touch_x = x;
        s.touch_y = y;
        
        #[cfg(target_os = "android")]
        info!("Touch move: ({}, {})", x, y);
    }
}

/// Render UI to buffer (placeholder - returns simple gradient)
#[no_mangle]
pub extern "C" fn zenith_native_render(
    width: i32,
    height: i32,
    out_stride: *mut i32,
) -> *mut c_void {
    use std::ffi::c_void;
    
    unsafe {
        if width <= 0 || height <= 0 {
            return std::ptr::null_mut();
        }
        
        // Allocate RGBA buffer
        let size = (width * height * 4) as usize;
        let mut buffer: Vec<u8> = Vec::with_capacity(size);
        
        // Create a simple gradient for testing
        for y in 0..height {
            for x in 0..width {
                let r = ((x as f32 / width as f32) * 255.0) as u8;
                let g = ((y as f32 / height as f32) * 255.0) as u8;
                let b = 128u8;
                let a = 255u8;
                buffer.extend_from_slice(&[r, g, b, a]);
            }
        }
        
        if !out_stride.is_null() {
            *out_stride = width * 4;
        }
        
        #[cfg(target_os = "android")]
        info!("Rendered {}x{} buffer ({} bytes)", width, height, size);
        
        // Transfer ownership to caller
        let box_slice = buffer.into_boxed_slice();
        let ptr = Box::into_raw(box_slice) as *mut c_void;
        ptr
    }
}

/// Free memory allocated by native functions
#[no_mangle]
pub extern "C" fn zenith_native_free(ptr: *mut c_void) {
    unsafe {
        if !ptr.is_null() {
            drop(Box::from_raw(ptr as *mut u8));
        }
    }
}

/// Get current state as JSON
#[no_mangle]
pub extern "C" fn zenith_native_get_state() -> *mut c_char {
    let state = ZENITH_STATE.lock().unwrap();
    match &*state {
        Some(s) => {
            let json = format!(
                r#"{{"initialized": true, "path": "{}", "touch_x": {}, "touch_y": {}, "last_action": {}}}"#,
                s.app_data_path, s.touch_x, s.touch_y, s.last_action
            );
            unsafe { CString::new(json).unwrap().into_raw() }
        }
        None => {
            let result = RustResult::err("Not initialized");
            let json = serde_json::to_string(&result).unwrap_or_default();
            unsafe { CString::new(json).unwrap().into_raw() }
        }
    }
}

/// Set state from JSON
#[no_mangle]
pub extern "C" fn zenith_native_set_state(state_json: *const c_char) -> i32 {
    unsafe {
        if state_json.is_null() {
            return -1;
        }
        
        let c_str = CStr::from_ptr(state_json);
        let json_str = match c_str.to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        };
        
        #[derive(Deserialize)]
        struct StateInput {
            touch_x: Option<f32>,
            touch_y: Option<f32>,
        }
        
        match serde_json::from_str::<StateInput>(json_str) {
            Ok(input) => {
                let mut state = ZENITH_STATE.lock().unwrap();
                if let Some(ref mut s) = *state {
                    if let Some(x) = input.touch_x {
                        s.touch_x = x;
                    }
                    if let Some(y) = input.touch_y {
                        s.touch_y = y;
                    }
                }
                0
            }
            Err(_) => -1,
        }
    }
}

/// Cleanup and shutdown
#[no_mangle]
pub extern "C" fn zenith_native_shutdown() {
    let mut state = ZENITH_STATE.lock().unwrap();
    *state = None;
    
    #[cfg(target_os = "android")]
    info!("Zenith Rust runtime shut down");
}

// Helper function for fibonacci calculation
fn fibonacci(n: u32) -> u64 {
    if n <= 1 {
        return n as u64;
    }
    
    let mut a: u64 = 0;
    let mut b: u64 = 1;
    
    for _ in 2..=n {
        let temp = a + b;
        a = b;
        b = temp;
    }
    
    b
}

// Tests
#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_fibonacci() {
        assert_eq!(fibonacci(0), 0);
        assert_eq!(fibonacci(1), 1);
        assert_eq!(fibonacci(10), 55);
        assert_eq!(fibonacci(20), 6765);
    }
    
    #[test]
    fn test_rust_result() {
        let ok_result = RustResult::ok("test data");
        assert!(ok_result.success);
        assert_eq!(ok_result.data, "test data");
        assert!(ok_result.error.is_none());
        
        let err_result = RustResult::err("test error");
        assert!(!err_result.success);
        assert!(err_result.data.is_empty());
        assert!(err_result.error.is_some());
    }
    
    #[test]
    fn test_zenith_init() {
        let path = std::ffi::CString::new("/tmp/test").unwrap();
        let result = zenith_native_init(path.as_ptr());
        assert_eq!(result, 0);
        
        let state = ZENITH_STATE.lock().unwrap();
        assert!(state.is_some());
        assert!(state.as_ref().unwrap().initialized);
    }
}
