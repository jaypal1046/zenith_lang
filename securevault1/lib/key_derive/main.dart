import 'dart:convert';
import 'package:pointycastle/export.dart';

@pragma('wasm:export', 'deriveKey')
String deriveKey(String password, String salt) {
  final pkcs = PBKDF2KeyDeriv(HMac(SHA256Digest(), 64));
  final saltBytes = utf8.encode(salt);
  final passwordBytes = utf8.encode(password);
  
  pkcs.init(Pbkdf2Parameters(saltBytes, 1000, 32)); // 1000 iterations, 32 bytes key length
  final keyBytes = pkcs.process(passwordBytes);
  
  return base64.encode(keyBytes);
}

void main() {}
