import 'dart:convert';
import 'dart:js_interop';
import 'dart:js_interop_unsafe';
import 'package:pointycastle/export.dart';

@JS('window')
external JSObject get window;

JSString dartDeriveKey(JSString password, JSString salt) {
  final passStr = password.toDart;
  final saltStr = salt.toDart;
  
  final pkcs = PBKDF2KeyDerivator(HMac(SHA256Digest(), 64));
  final saltBytes = utf8.encode(saltStr);
  final passwordBytes = utf8.encode(passStr);
  
  pkcs.init(Pbkdf2Parameters(saltBytes, 1000, 32)); // 1000 iterations, 32 bytes key length
  final keyBytes = pkcs.process(passwordBytes);
  
  return base64.encode(keyBytes).toJS;
}

void main() {
  window['deriveKey'] = dartDeriveKey.toJS;
}
