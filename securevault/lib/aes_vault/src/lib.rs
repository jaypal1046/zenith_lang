use std::ffi::{CString, CStr};
use std::os::raw::c_char;
use aes_gcm::{
    aead::{Aead, KeyInit},
    Aes256Gcm, Nonce
};

#[no_mangle]
pub extern "C" fn encrypt(plaintext: *const c_char, key: *const c_char) -> *mut c_char {
    let plaintext = unsafe { CStr::from_ptr(plaintext).to_string_lossy().into_owned() };
    let key_str = unsafe { CStr::from_ptr(key).to_string_lossy().into_owned() };
    
    // Hash/pad key to 32 bytes (AES-256)
    let mut key_bytes = [0u8; 32];
    let key_src = key_str.as_bytes();
    let len = key_src.len().min(32);
    key_bytes[..len].copy_from_slice(&key_src[..len]);
    
    let cipher = Aes256Gcm::new_from_slice(&key_bytes).unwrap();
    let nonce = Nonce::from_slice(b"unique_nonce"); // 12 bytes nonce
    
    match cipher.encrypt(nonce, plaintext.as_bytes()) {
        Ok(ciphertext_bytes) => {
            let hex_str = hex::encode(ciphertext_bytes);
            CString::new(hex_str).unwrap().into_raw()
        }
        Err(_) => CString::new("").unwrap().into_raw()
    }
}

#[no_mangle]
pub extern "C" fn decrypt(ciphertext: *const c_char, key: *const c_char) -> *mut c_char {
    let ciphertext_hex = unsafe { CStr::from_ptr(ciphertext).to_string_lossy().into_owned() };
    let key_str = unsafe { CStr::from_ptr(key).to_string_lossy().into_owned() };
    
    // Hash/pad key to 32 bytes (AES-256)
    let mut key_bytes = [0u8; 32];
    let key_src = key_str.as_bytes();
    let len = key_src.len().min(32);
    key_bytes[..len].copy_from_slice(&key_src[..len]);
    
    let cipher = Aes256Gcm::new_from_slice(&key_bytes).unwrap();
    let nonce = Nonce::from_slice(b"unique_nonce"); // 12 bytes nonce
    
    if let Ok(ciphertext_bytes) = hex::decode(ciphertext_hex) {
        match cipher.decrypt(nonce, ciphertext_bytes.as_slice()) {
            Ok(plaintext_bytes) => {
                let plaintext = String::from_utf8_lossy(&plaintext_bytes).into_owned();
                CString::new(plaintext).unwrap().into_raw()
            }
            Err(_) => CString::new("[Decryption Error]").unwrap().into_raw()
        }
    } else {
        CString::new("[Decryption Error: Invalid Hex]").unwrap().into_raw()
    }
}
