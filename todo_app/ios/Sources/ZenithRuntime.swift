// Zenith iOS Runtime - Swift implementation for native iOS apps
// Compiles Zenith code to SwiftUI with native plugin access

import Foundation
import SwiftUI
import Combine

// MARK: - Zenith Runtime Core
class ZenithRuntime: ObservableObject {
    @Published var state: [String: Any] = [:]
    @Published var uiTree: [ZenithNode] = []
    
    private var plugins: [String: Any] = [:]
    private var cancellables = Set<AnyCancellable>()
    
    // Register iOS-native plugins
    func registerPlugin(_ name: String, implementation: @escaping ([String: Any]) async throws -> Any) {
        plugins[name] = implementation
    }
    
    // Execute Zenith function
    func execute(_ funcName: String, args: [String: Any] = [:]) async throws -> Any {
        guard let plugin = plugins[funcName] as? ([String: Any]) async throws -> Any else {
            throw NSError(domain: "ZenithRuntime", code: 404, userInfo: [NSLocalizedDescriptionKey: "Function \(funcName) not found"])
        }
        return try await plugin(args)
    }
    
    // Parse and render Zenith UI
    func render(from zenithCode: String) {
        // In production: compile .zen to AST
        // For demo: hardcoded AST
        uiTree = [
            ZenithNode(type: .container, children: [
                ZenithNode(type: .text, value: "Welcome to Zenith iOS!"),
                ZenithNode(type: .card, children: [
                    ZenithNode(type: .text, value: "Native SwiftUI rendered from Zenith"),
                    ZenithNode(type: .button, label: "Tap Me", action: "handleTap")
                ]),
                ZenithNode(type: .textField, placeholder: "Enter text...", action: "handleInput")
            ])
        ]
    }
}

// MARK: - Zenith UI Node
enum ZenithNodeType {
    case container, text, button, card, textField, image, list
}

struct ZenithNode: Identifiable {
    let id = UUID()
    let type: ZenithNodeType
    var value: String?
    var label: String?
    var placeholder: String?
    var action: String?
    var children: [ZenithNode]?
    
    init(type: ZenithNodeType, value: String? = nil, label: String? = nil, 
         placeholder: String? = nil, action: String? = nil, children: [ZenithNode]? = nil) {
        self.type = type
        self.value = value
        self.label = label
        self.placeholder = placeholder
        self.action = action
        self.children = children
    }
}

// MARK: - SwiftUI Renderer
struct ZenithView: View {
    @ObservedObject var runtime: ZenithRuntime
    @State private var inputText = ""
    
    var body: some View {
        VStack(spacing: 16) {
            ForEach(runtime.uiTree) { node in
                renderNode(node)
            }
        }
        .padding()
    }
    
    @ViewBuilder
    private func renderNode(_ node: ZenithNode) -> some View {
        switch node.type {
        case .container:
            VStack {
                ForEach(node.children ?? []) { child in
                    renderNode(child)
                }
            }
        case .text:
            Text(node.value ?? "")
                .font(.body)
        case .button:
            Button(action: {
                Task {
                    try? await runtime.execute(node.action ?? "", args: [:])
                }
            }) {
                Text(node.label ?? "Button")
                    .foregroundColor(.white)
                    .padding()
                    .background(Color.purple)
                    .cornerRadius(8)
            }
        case .card:
            VStack(alignment: .leading, spacing: 8) {
                ForEach(node.children ?? []) { child in
                    renderNode(child)
                }
            }
            .padding()
            .background(Color.white)
            .cornerRadius(12)
            .shadow(radius: 4)
        case .textField:
            TextField(node.placeholder ?? "", text: $inputText)
                .onChange(of: inputText) { newValue in
                    Task {
                        try? await runtime.execute(node.action ?? "", args: ["value": newValue])
                    }
                }
                .padding()
                .background(Color.gray.opacity(0.1))
                .cornerRadius(8)
        default:
            EmptyView()
        }
    }
}

// MARK: - iOS Native Plugins
extension ZenithRuntime {
    func setupNativePlugins() {
        // HTTP Client
        registerPlugin("http.get") { args in
            guard let urlStr = args["url"] as? String,
                  let url = URL(string: urlStr) else {
                throw NSError(domain: "HTTP", code: 400, userInfo: [NSLocalizedDescriptionKey: "Invalid URL"])
            }
            
            let (data, _) = try await URLSession.shared.data(from: url)
            return try JSONSerialization.jsonObject(with: data) as? [String: Any] ?? [:]
        }
        
        // UserDefaults Storage
        registerPlugin("storage.set") { args in
            if let key = args["key"] as? String, let value = args["value"] {
                UserDefaults.standard.set(value, forKey: key)
                return ["success": true]
            }
            throw NSError(domain: "Storage", code: 400, userInfo: [NSLocalizedDescriptionKey: "Invalid params"])
        }
        
        registerPlugin("storage.get") { args in
            guard let key = args["key"] as? String else {
                throw NSError(domain: "Storage", code: 400, userInfo: [NSLocalizedDescriptionKey: "Missing key"])
            }
            return UserDefaults.standard.object(forKey: key) ?? NSNull()
        }
        
        // Camera (placeholder)
        registerPlugin("camera.capture") { _ in
            // Integrate with UIImagePickerController in real app
            return ["path": "ios_camera_placeholder.jpg"]
        }
        
        // Crypto (SHA256 using CommonCrypto)
        registerPlugin("crypto.sha256") { args in
            guard let data = args["data"] as? String else {
                throw NSError(domain: "Crypto", code: 400, userInfo: [NSLocalizedDescriptionKey: "Missing data"])
            }
            
            let inputData = Data(data.utf8)
            var hash = [UInt8](repeating: 0, count: Int(CC_SHA256_DIGEST_LENGTH))
            inputData.withUnsafeBytes {
                CC_SHA256($0.baseAddress, CC_LONG(inputData.count), &hash)
            }
            
            return hash.map { String(format: "%02x", $0) }.joined()
        }
    }
}

// MARK: - App Entry Point
struct ZenithApp: App {
    @StateObject private var runtime = ZenithRuntime()
    
    var body: some Scene {
        WindowGroup {
            ZenithView(runtime: runtime)
                .onAppear {
                    runtime.setupNativePlugins()
                    runtime.render(from: "")
                }
        }
    }
}

// Import CommonCrypto for SHA256
import CommonCrypto
