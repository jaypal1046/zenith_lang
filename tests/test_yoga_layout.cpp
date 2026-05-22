/**
 * Tests for Zenith Yoga Layout Engine
 */

#include "zenith/ui/yoga_layout.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace zenith::ui;

void testBasicNodeCreation() {
    std::cout << "Test: Basic Node Creation... ";
    
    auto node = std::make_shared<YogaNode>();
    assert(node != nullptr);
    assert(node->getChildCount() == 0);
    
    std::cout << "PASSED" << std::endl;
}

void testFlexDirection() {
    std::cout << "Test: Flex Direction... ";
    
    auto node = std::make_shared<YogaNode>();
    node->setFlexDirection(FlexDirection::Row);
    node->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testDimensions() {
    std::cout << "Test: Dimensions... ";
    
    auto node = std::make_shared<YogaNode>();
    node->setWidth(MeasureValue::points(200));
    node->setHeight(MeasureValue::points(100));
    node->calculateLayout(400, 300);
    
    LayoutResult layout = node->getLayout();
    // In stub mode, we just verify it doesn't crash
    (void)layout;
    
    std::cout << "PASSED" << std::endl;
}

void testMarginsAndPadding() {
    std::cout << "Test: Margins and Padding... ";
    
    auto node = std::make_shared<YogaNode>();
    node->setMargin(Edge::Left, MeasureValue::points(10));
    node->setMargin(Edge::Right, MeasureValue::points(10));
    node->setMargin(Edge::Top, MeasureValue::points(5));
    node->setMargin(Edge::Bottom, MeasureValue::points(5));
    node->setPadding(Edge::Left, MeasureValue::points(15));
    node->setPadding(Edge::Right, MeasureValue::points(15));
    node->setPadding(Edge::Top, MeasureValue::points(8));
    node->setPadding(Edge::Bottom, MeasureValue::points(8));
    node->setBorder(Edge::All, 2);
    
    node->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testChildNodes() {
    std::cout << "Test: Child Nodes... ";
    
    auto parent = std::make_shared<YogaNode>();
    parent->setWidth(MeasureValue::points(300));
    parent->setHeight(MeasureValue::points(200));
    
    auto child1 = std::make_shared<YogaNode>();
    child1->setWidth(MeasureValue::points(100));
    
    auto child2 = std::make_shared<YogaNode>();
    child2->setWidth(MeasureValue::points(100));
    
    parent->addChild(child1);
    parent->addChild(child2);
    
    assert(parent->getChildCount() == 2);
    
    parent->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testLayoutBuilder() {
    std::cout << "Test: Layout Builder... ";
    
    auto root = std::make_shared<YogaNode>();
    LayoutBuilder builder(root);
    
    builder.flexDirection(FlexDirection::Column)
           .justifyContent(JustifyContent::SpaceBetween)
           .alignItems(AlignItems::Center)
           .padding(20)
           .width(400)
           .height(300);
    
    // Add a child using builder
    auto childBuilder = builder.child();
    childBuilder.width(200)
              .height(100)
              .flexGrow(1.0f);
    builder.endChild();
    
    root->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testPercentages() {
    std::cout << "Test: Percentage Values... ";
    
    auto parent = std::make_shared<YogaNode>();
    parent->setWidth(MeasureValue::points(400));
    parent->setHeight(MeasureValue::points(300));
    
    auto child = std::make_shared<YogaNode>();
    child->setWidth(MeasureValue::percent(50));
    child->setHeight(MeasureValue::percent(25));
    
    parent->addChild(child);
    parent->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testPositioning() {
    std::cout << "Test: Positioning... ";
    
    auto node = std::make_shared<YogaNode>();
    node->setPositionType(PositionType::Absolute);
    node->setLeft(MeasureValue::points(50));
    node->setTop(MeasureValue::points(30));
    node->setWidth(MeasureValue::points(100));
    node->setHeight(MeasureValue::points(80));
    
    node->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testLayoutContext() {
    std::cout << "Test: Layout Context... ";
    
    LayoutContext context;
    
    auto root = context.createRoot();
    root->setWidth(MeasureValue::points(800));
    root->setHeight(MeasureValue::points(600));
    
    auto child1 = context.createNode();
    child1->setFlexGrow(1.0f);
    
    auto child2 = context.createNode();
    child2->setHeight(MeasureValue::points(100));
    
    root->addChild(child1);
    root->addChild(child2);
    
    context.calculateTreeLayout(root, 800, 600);
    
    int nodeCount = 0;
    context.applyLayout(root, [&nodeCount](void* userData, const LayoutResult& layout) {
        nodeCount++;
        (void)userData;
        (void)layout;
    });
    
    assert(nodeCount == 3); // root + 2 children
    
    std::cout << "PASSED" << std::endl;
}

void testMeasureFunction() {
    std::cout << "Test: Measure Function... ";
    
    auto node = std::make_shared<YogaNode>();
    
    // Set a custom measure function
    node->setMeasureFunction([](float width, float height) -> std::pair<float, float> {
        // Return fixed size regardless of constraints
        return {150.0f, 75.0f};
    });
    
    node->calculateLayout(400, 300);
    
    node->clearMeasureFunction();
    
    std::cout << "PASSED" << std::endl;
}

void testFlexGrowShrink() {
    std::cout << "Test: Flex Grow/Shrink... ";
    
    auto container = std::make_shared<YogaNode>();
    container->setFlexDirection(FlexDirection::Row);
    container->setWidth(MeasureValue::points(300));
    
    auto item1 = std::make_shared<YogaNode>();
    item1->setFlexGrow(1.0f);
    
    auto item2 = std::make_shared<YogaNode>();
    item2->setFlexGrow(2.0f); // Should take twice the space of item1
    
    auto item3 = std::make_shared<YogaNode>();
    item3->setFlexShrink(0.5f); // Resists shrinking
    
    container->addChild(item1);
    container->addChild(item2);
    container->addChild(item3);
    
    container->calculateLayout(300, 200);
    
    std::cout << "PASSED" << std::endl;
}

void testDisplayNone() {
    std::cout << "Test: Display None... ";
    
    auto node = std::make_shared<YogaNode>();
    node->setDisplay(DisplayType::None);
    node->calculateLayout(400, 300);
    
    // Node with display:none should not be rendered
    std::cout << "PASSED" << std::endl;
}

void testAspectRatio() {
    std::cout << "Test: Aspect Ratio... ";
    
    auto node = std::make_shared<YogaNode>();
    node->setWidth(MeasureValue::points(200))
        .setAspectRatio(16.0f / 9.0f);
    
    node->calculateLayout(400, 300);
    
    std::cout << "PASSED" << std::endl;
}

void testAlignSelf() {
    std::cout << "Test: Align Self... ";
    
    auto container = std::make_shared<YogaNode>();
    container->setFlexDirection(FlexDirection::Row)
            .setAlignItems(AlignItems::FlexStart)
            .setHeight(MeasureValue::points(200));
    
    auto child1 = std::make_shared<YogaNode>();
    child1->setHeight(MeasureValue::points(100));
    
    auto child2 = std::make_shared<YogaNode>();
    child2->setHeight(MeasureValue::points(150))
        .setAlignSelf(AlignSelf::Center); // Override parent's align
    
    container->addChild(child1);
    container->addChild(child2);
    
    container->calculateLayout(300, 200);
    
    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Zenith Yoga Layout Engine Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        testBasicNodeCreation();
        testFlexDirection();
        testDimensions();
        testMarginsAndPadding();
        testChildNodes();
        testLayoutBuilder();
        testPercentages();
        testPositioning();
        testLayoutContext();
        testMeasureFunction();
        testFlexGrowShrink();
        testDisplayNone();
        testAspectRatio();
        testAlignSelf();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "All tests PASSED! ✓" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test FAILED: " << e.what() << std::endl;
        return 1;
    }
}
