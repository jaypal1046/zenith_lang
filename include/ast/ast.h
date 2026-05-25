#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

// Type aliases for common types
using f32 = float;
using f64 = double;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Alias for Int type used in Zenith
using Int = i32;
using Bool = bool;

// Base Node
class ASTNode {
public:
    int line = 0;
    int column = 0;
    virtual ~ASTNode() = default;
};

// Type Node with inference support
class TypeNode : public ASTNode {
public:
    std::string type_name;
    std::vector<std::unique_ptr<TypeNode>> generics;
    bool is_inferred = false;  // For type inference (let x = ...)
    bool is_nullable = false;  // For Option<T> types
    bool is_async = false;     // For async types
    
    TypeNode(std::string name) : type_name(std::move(name)) {}
    
    // Check if this is an Option type
    bool isOption() const {
        return type_name == "Option" || is_nullable;
    }
    
    // Check if this is a Result type
    bool isResult() const {
        return type_name == "Result";
    }
    
    // Get the inner type for Option/Result
    TypeNode* getInnerType() {
        if (!generics.empty()) {
            return generics[0].get();
        }
        return nullptr;
    }
};

// Expressions
class ExprNode : public ASTNode {};

// Dummy expression to hold logic like 'x > 10' for now
class BinaryExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> left;
    std::string op;
    std::unique_ptr<ExprNode> right;
    bool is_string_concat = false;
    BinaryExprNode(std::unique_ptr<ExprNode> l, std::string o, std::unique_ptr<ExprNode> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
};

class IdentifierNode : public ExprNode {
public:
    std::string name;
    std::string type_hint;  // For type inference resolution
    IdentifierNode(std::string n) : name(std::move(n)) {}
};

// Null literal for Option types
class NullLiteralNode : public ExprNode {
public:
    NullLiteralNode() {}
};

// Option/Some/None expressions for safe null handling
class OptionExprNode : public ExprNode {
public:
    enum class OptionKind { Some, None };
    OptionKind kind;
    std::unique_ptr<ExprNode> value;  // Only used for Some
    
    OptionExprNode(OptionKind k, std::unique_ptr<ExprNode> val = nullptr) 
        : kind(k), value(std::move(val)) {}
};

// Result expression for error handling (Result<T, E>)
class ResultExprNode : public ExprNode {
public:
    enum class ResultKind { Ok, Err };
    ResultKind kind;
    std::unique_ptr<ExprNode> value;
    
    ResultExprNode(ResultKind k, std::unique_ptr<ExprNode> val) 
        : kind(k), value(std::move(val)) {}
};

class StringLiteralNode : public ExprNode {
public:
    std::string value;
    StringLiteralNode(std::string v) : value(std::move(v)) {}
};

class NumberLiteralNode : public ExprNode {
public:
    std::string value;
    bool is_float;
    NumberLiteralNode(std::string v, bool f) : value(std::move(v)), is_float(f) {}
};

class BoolLiteralNode : public ExprNode {
public:
    bool value;
    BoolLiteralNode(bool v) : value(v) {}
};

class ListLiteralNode : public ExprNode {
public:
    std::vector<std::unique_ptr<ExprNode>> elements;
};

class MapLiteralNode : public ExprNode {
public:
    std::vector<std::pair<std::unique_ptr<ExprNode>, std::unique_ptr<ExprNode>>> entries;
};

class PropertyAccessNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> object;
    std::string property_name;
    PropertyAccessNode(std::unique_ptr<ExprNode> obj, std::string prop)
        : object(std::move(obj)), property_name(std::move(prop)) {}
};

class FunctionCallNode : public ExprNode {
public:
    std::string function_name;
    std::vector<std::unique_ptr<ExprNode>> arguments;
    
    FunctionCallNode(std::string name) : function_name(std::move(name)) {}
};

class MethodCallNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> object;
    std::string method_name;
    std::vector<std::unique_ptr<ExprNode>> arguments;
    
    MethodCallNode(std::unique_ptr<ExprNode> obj, std::string method)
        : object(std::move(obj)), method_name(std::move(method)) {}
};

// Memory hint for variable declarations
enum class MemoryAnnotation {
    Default,  // plain stack/value type (no RC tracking)
    Ref,      // strong Ref<T> — increments RC
    Weak,     // weak Weak<T> — does not increment RC
    GcRoot,   // explicitly pinned as a GC root
};

// Variable Declaration (let x: String = "val" or String x = "val")
class VarDeclNode : public ASTNode {
public:
    std::string var_name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ExprNode> initializer;
    MemoryAnnotation memory_hint = MemoryAnnotation::Default;
    
    VarDeclNode(std::unique_ptr<TypeNode> t, std::string name, std::unique_ptr<ExprNode> init = nullptr)
        : type(std::move(t)), var_name(std::move(name)), initializer(std::move(init)) {}
};

// ==========================================
// CONTROL FLOW
// ==========================================
class IfStmtNode : public ASTNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<ASTNode>> then_branch;
    std::vector<std::unique_ptr<ASTNode>> else_branch; // Can be empty
    
    IfStmtNode(std::unique_ptr<ExprNode> cond) : condition(std::move(cond)) {}
};

class WhileStmtNode : public ASTNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
    
    WhileStmtNode(std::unique_ptr<ExprNode> cond) : condition(std::move(cond)) {}
};

class ReturnStmtNode : public ASTNode {
public:
    std::unique_ptr<ExprNode> expression;
    ReturnStmtNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
};

class SetStateStmtNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> body;
    SetStateStmtNode(std::vector<std::unique_ptr<ASTNode>> body_stmts) : body(std::move(body_stmts)) {}
};

// Async/Await support (defined after FunctionNode)
class AwaitExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> expression;
    AwaitExprNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
};

// Try-Catch for Result types
class TryExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> expression;
    TryExprNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
};

class MatchExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> subject;
    std::vector<std::pair<std::string, std::unique_ptr<ExprNode>>> arms;  // pattern -> expression
    
    MatchExprNode(std::unique_ptr<ExprNode> subj) : subject(std::move(subj)) {}
};

// Closure/Lambda expressions
class LambdaNode : public ExprNode {
public:
    std::vector<std::unique_ptr<VarDeclNode>> parameters;
    std::unique_ptr<TypeNode> return_type;
    std::vector<std::unique_ptr<ASTNode>> body;
    
    LambdaNode(std::unique_ptr<TypeNode> ret_type) : return_type(std::move(ret_type)) {}
};

// Functions
class FunctionNode : public ASTNode {
public:
    std::string function_name;
    std::unique_ptr<TypeNode> return_type;
    std::vector<std::unique_ptr<VarDeclNode>> parameters;
    std::vector<std::unique_ptr<ASTNode>> body;
    bool is_async = false;
    
    // Interop fields
    bool is_foreign = false;
    std::string foreign_abi; // "C", "js", "python"
    bool is_exported = false;

    FunctionNode(std::unique_ptr<TypeNode> ret_type, std::string name)
        : return_type(std::move(ret_type)), function_name(std::move(name)) {}
};

// Enhanced Agentic Functions with streaming and multi-modal support
class AgenticFunctionNode : public FunctionNode {
public:
    std::string prompt_template;
    bool is_streaming = false;      // For streaming LLM responses
    bool is_multimodal = false;     // True if function accepts multimodal inputs (images)
    std::string model_name;         // Specific LLM model to use
    f32 temperature = 0.7f;         // Temperature for generation
    Int max_tokens = 1024;          // Max tokens to generate
    std::vector<std::string> tools; // Available tools/functions for the agent
    
    AgenticFunctionNode(std::unique_ptr<TypeNode> ret_type, std::string name, std::string prompt)
        : FunctionNode(std::move(ret_type), std::move(name)), prompt_template(std::move(prompt)) {}
};

// Agent orchestration - coordinate multiple AI agents
class AgentOrchestrationNode : public ASTNode {
public:
    std::string orchestration_name;
    std::vector<std::string> agents;  // List of agent function names
    std::string strategy;             // "sequential", "parallel", "conditional"
    std::vector<std::unique_ptr<ASTNode>> body;
    
    AgentOrchestrationNode(std::string name) : orchestration_name(std::move(name)) {}
};

// Classes with Primary Constructors (Kotlin style) and Reactivity
class ClassDeclNode : public ASTNode {
public:
    std::string class_name;
    std::vector<std::string> implemented_interfaces;
    std::vector<std::unique_ptr<VarDeclNode>> primary_constructor_args;
    std::vector<std::unique_ptr<VarDeclNode>> fields;
    std::vector<std::unique_ptr<FunctionNode>> methods;
    bool is_reactive = false;   // For reactive state management
    bool is_component = false;  // For UI components
    bool is_managed = false;    // For @managed RC+GC heap tracking
    
    ClassDeclNode(std::string name) : class_name(std::move(name)) {}
};

// Enhanced UI Components with Yoga layout properties
class UIComponentNode : public ExprNode {
public:
    std::string component_type; // e.g. Column, Text, Row
    std::vector<std::pair<std::string, std::unique_ptr<ExprNode>>> named_args;
    std::vector<std::unique_ptr<ExprNode>> children;
    
    // Yoga Flexbox properties
    std::string flex_direction = "column";
    std::string justify_content = "flex-start";
    std::string align_items = "stretch";
    f32 flex_grow = 0.0f;
    f32 flex_shrink = 1.0f;
    f32 flex_basis = -1.0f;  // -1 means auto
    f32 margin_top = 0.0f, margin_right = 0.0f, margin_bottom = 0.0f, margin_left = 0.0f;
    f32 padding_top = 0.0f, padding_right = 0.0f, padding_bottom = 0.0f, padding_left = 0.0f;
    f32 width = -1.0f;   // -1 means auto
    f32 height = -1.0f;  // -1 means auto
    
    UIComponentNode(std::string type) : component_type(std::move(type)) {}
    
    // Helper to check if this is a semantic HTML element
    bool isSemanticElement() const {
        return component_type == "Text" || component_type == "Button" || 
               component_type == "Image" || component_type == "Link";
    }
};

class InterfaceDeclNode : public ASTNode {
public:
    std::string interface_name;
    std::vector<std::unique_ptr<FunctionNode>> methods;
    InterfaceDeclNode(std::string name) : interface_name(std::move(name)) {}
};

class ImportNode : public ASTNode {
public:
    enum class ImportKind {
        Module,  // import std.io              (classic Zenith module)
        Cdn,     // import cdn "https://..."   (direct CDN URL, web only)
        Npm,     // import npm "chart.js"      (jsDelivr CDN, web/wasm only)
        Native,  // import native "openssl"    (system/C++ lib, cpp/android/ios targets)
        Zen,     // import zen "zenith-chart"  (cross-platform Zenith package, all targets)
    };

    std::string module_name;    // package name or module path
    std::string cdn_url;        // resolved CDN/path URL
    std::string target_filter;  // "" = all targets | "web" | "cpp" | "android" | "ios" | "wasm"
    ImportKind  kind = ImportKind::Module;

    ImportNode(std::string name, ImportKind k = ImportKind::Module,
               std::string url = "", std::string filter = "")
        : module_name(std::move(name)), kind(k),
          cdn_url(std::move(url)), target_filter(std::move(filter)) {}

    // Returns true if this import should be active for the given target
    bool isActiveFor(const std::string& target) const {
        if (target_filter.empty()) return true;   // no filter = all targets
        return target_filter == target;
    }
};

// Memory annotation node — represents @managed, @weak, @gc_root decorators
class MemoryAnnotationNode : public ASTNode {
public:
    std::string annotation;  // "managed", "weak", "gc_root"
    MemoryAnnotationNode(std::string ann) : annotation(std::move(ann)) {}
};

// Program Root
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
};

#endif // AST_H
