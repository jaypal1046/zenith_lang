#include <iostream>
#include <cassert>
#include "../include/frontend/type_inference.hpp"
#include "../include/ast/ast.h"

void testLiteralInference() {
    TypeInferencer inferencer;
    
    // Test integer literal
    NumberLiteralNode intLit("42", false);
    assert(inferencer.inferType(&intLit) == "Int");
    std::cout << "✓ Integer literal inference: Int\n";
    
    // Test float literal
    NumberLiteralNode floatLit("3.14", true);
    assert(inferencer.inferType(&floatLit) == "Float");
    std::cout << "✓ Float literal inference: Float\n";
    
    // Test string literal
    StringLiteralNode strLit("hello");
    assert(inferencer.inferType(&strLit) == "String");
    std::cout << "✓ String literal inference: String\n";
    
    // Test bool literal
    BoolLiteralNode boolLit(true);
    assert(inferencer.inferType(&boolLit) == "Bool");
    std::cout << "✓ Bool literal inference: Bool\n";
}

void testListInference() {
    TypeInferencer inferencer;
    
    ListLiteralNode listLit;
    listLit.elements.push_back(std::make_unique<NumberLiteralNode>("1", false));
    listLit.elements.push_back(std::make_unique<NumberLiteralNode>("2", false));
    listLit.elements.push_back(std::make_unique<NumberLiteralNode>("3", false));
    
    assert(inferencer.inferType(&listLit) == "List<Int>");
    std::cout << "✓ List[Int] inference: List<Int>\n";
    
    // Test empty list
    ListLiteralNode emptyList;
    assert(inferencer.inferType(&emptyList) == "List<Void>");
    std::cout << "✓ Empty list inference: List<Void>\n";
}

void testMapInference() {
    TypeInferencer inferencer;
    
    MapLiteralNode mapLit;
    auto key1 = std::make_unique<StringLiteralNode>("name");
    auto val1 = std::make_unique<StringLiteralNode>("Alice");
    mapLit.entries.push_back({std::move(key1), std::move(val1)});
    
    std::string result = inferencer.inferType(&mapLit);
    std::cout << "Map inference result: " << result << "\n";
    // Note: there's a space after comma in the output
    assert(result == "Map<String, String>" || result == "Map<String,String>");
    std::cout << "✓ Map[String,String] inference: " << result << "\n";
}

void testBinaryExprInference() {
    TypeInferencer inferencer;
    
    // Test arithmetic
    auto left = std::make_unique<NumberLiteralNode>("5", false);
    auto right = std::make_unique<NumberLiteralNode>("3", false);
    BinaryExprNode addExpr(std::move(left), "+", std::move(right));
    assert(inferencer.inferType(&addExpr) == "Int");
    std::cout << "✓ Arithmetic expression inference: Int\n";
    
    // Test comparison
    auto left2 = std::make_unique<NumberLiteralNode>("5", false);
    auto right2 = std::make_unique<NumberLiteralNode>("3", false);
    BinaryExprNode cmpExpr(std::move(left2), ">", std::move(right2));
    assert(inferencer.inferType(&cmpExpr) == "Bool");
    std::cout << "✓ Comparison expression inference: Bool\n";
    
    // Test string concatenation
    auto str1 = std::make_unique<StringLiteralNode>("Hello");
    auto str2 = std::make_unique<StringLiteralNode>("World");
    BinaryExprNode concatExpr(std::move(str1), "+", std::move(str2));
    assert(inferencer.inferType(&concatExpr) == "String");
    std::cout << "✓ String concatenation inference: String\n";
}

void testOptionResultInference() {
    TypeInferencer inferencer;
    
    // Test Some
    OptionExprNode some(OptionExprNode::OptionKind::Some, 
                        std::make_unique<NumberLiteralNode>("42", false));
    assert(inferencer.inferType(&some) == "Option<Int>");
    std::cout << "✓ Some(Int) inference: Option<Int>\n";
    
    // Test None
    OptionExprNode none(OptionExprNode::OptionKind::None);
    assert(inferencer.inferType(&none) == "Option<Void>");
    std::cout << "✓ None inference: Option<Void>\n";
    
    // Test Ok
    ResultExprNode ok(ResultExprNode::ResultKind::Ok, 
                      std::make_unique<StringLiteralNode>("success"));
    assert(inferencer.inferType(&ok) == "Result<String, Error>");
    std::cout << "✓ Ok(String) inference: Result<String, Error>\n";
}

void testCanInferFromContext() {
    TypeInferencer inferencer;
    
    NumberLiteralNode num("42", false);
    assert(inferencer.canInferFromContext(&num) == true);
    std::cout << "✓ Can infer from number literal\n";
    
    StringLiteralNode str("test");
    assert(inferencer.canInferFromContext(&str) == true);
    std::cout << "✓ Can infer from string literal\n";
    
    IdentifierNode ident("unknown");
    assert(inferencer.canInferFromContext(&ident) == false);
    std::cout << "✓ Cannot infer from identifier (needs context)\n";
}

void testUnification() {
    TypeInferencer inferencer;
    
    assert(inferencer.unifyTypes("Int", "Int") == true);
    assert(inferencer.unifyTypes("String", "String") == true);
    assert(inferencer.unifyTypes("Int", "i32") == true);
    assert(inferencer.unifyTypes("Float", "f64") == true);
    assert(inferencer.unifyTypes("Int", "Float") == false);
    assert(inferencer.unifyTypes("List<Int>", "List<Int>") == true);
    assert(inferencer.unifyTypes("List<Int>", "List<i32>") == true);
    assert(inferencer.unifyTypes("List<Int>", "List<Float>") == false);
    assert(inferencer.unifyTypes("Map<String, Int>", "Map<String, i32>") == true);
    
    std::cout << "✓ Type unification tests passed\n";
}

int main() {
    std::cout << "=== Type Inference System Tests ===\n\n";
    
    std::cout << "--- Type Unification ---\n";
    testUnification();
    
    std::cout << "\n--- Literal Type Inference ---\n";
    testLiteralInference();
    
    std::cout << "\n--- Collection Type Inference ---\n";
    testListInference();
    testMapInference();
    
    std::cout << "\n--- Expression Type Inference ---\n";
    testBinaryExprInference();
    
    std::cout << "\n--- Option/Result Type Inference ---\n";
    testOptionResultInference();
    
    std::cout << "\n--- Context-Based Inference ---\n";
    testCanInferFromContext();
    
    std::cout << "\n✅ All type inference tests passed!\n";
    return 0;
}
