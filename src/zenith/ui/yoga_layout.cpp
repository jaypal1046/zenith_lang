/**
 * Zenith UI - Yoga Layout Engine Implementation
 * 
 * Implementation of the Yoga layout wrapper using Facebook's Yoga library.
 */

#include "zenith/ui/yoga_layout.h"
#include <iostream>
#include <algorithm>
#include <cstring>

// Include Yoga headers when available
// #include <yoga/YGNode.h>
// #include <yoga/Yoga.h>

namespace zenith {
namespace ui {

// ============================================================================
// Helper Functions for Enum Conversions
// ============================================================================

namespace {

#ifdef YOGA_AVAILABLE
YGFlexDirection toYogaFlexDirection(FlexDirection dir) {
    switch (dir) {
        case FlexDirection::Row: return YGFlexDirectionRow;
        case FlexDirection::RowReverse: return YGFlexDirectionRowReverse;
        case FlexDirection::Column: return YGFlexDirectionColumn;
        case FlexDirection::ColumnReverse: return YGFlexDirectionColumnReverse;
    }
}

YGJustify toYogaJustifyContent(JustifyContent justify) {
    switch (justify) {
        case JustifyContent::FlexStart: return YGJustifyFlexStart;
        case JustifyContent::FlexEnd: return YGJustifyFlexEnd;
        case JustifyContent::Center: return YGJustifyCenter;
        case JustifyContent::SpaceBetween: return YGJustifySpaceBetween;
        case JustifyContent::SpaceAround: return YGJustifySpaceAround;
        case JustifyContent::SpaceEvenly: return YGJustifySpaceEvenly;
    }
}

YGAlign toYogaAlignItems(AlignItems align) {
    switch (align) {
        case AlignItems::FlexStart: return YGAlignFlexStart;
        case AlignItems::FlexEnd: return YGAlignFlexEnd;
        case AlignItems::Center: return YGAlignCenter;
        case AlignItems::Baseline: return YGAlignBaseline;
        case AlignItems::Stretch: return YGAlignStretch;
    }
}

YGAlign toYogaAlignSelf(AlignSelf align) {
    switch (align) {
        case AlignSelf::Auto: return YGAlignAuto;
        case AlignSelf::FlexStart: return YGAlignFlexStart;
        case AlignSelf::FlexEnd: return YGAlignFlexEnd;
        case AlignSelf::Center: return YGAlignCenter;
        case AlignSelf::Baseline: return YGAlignBaseline;
        case AlignSelf::Stretch: return YGAlignStretch;
    }
}

YGWrap toYogaFlexWrap(FlexWrap wrap) {
    switch (wrap) {
        case FlexWrap::NoWrap: return YGWrapNoWrap;
        case FlexWrap::Wrap: return YGWrapWrap;
        case FlexWrap::WrapReverse: return YGWrapWrapReverse;
    }
}

YGPositionType toYogaPositionType(PositionType type) {
    switch (type) {
        case PositionType::Relative: return YGPositionTypeRelative;
        case PositionType::Absolute: return YGPositionTypeAbsolute;
    }
}

YGDisplay toYogaDisplay(DisplayType display) {
    switch (display) {
        case DisplayType::Flex: return YGDisplayFlex;
        case DisplayType::None: return YGDisplayNone;
    }
}

YGEdge toYogaEdge(Edge edge) {
    switch (edge) {
        case Edge::Left: return YGEdgeLeft;
        case Edge::Top: return YGEdgeTop;
        case Edge::Right: return YGEdgeRight;
        case Edge::Bottom: return YGEdgeBottom;
        case Edge::Start: return YGEdgeStart;
        case Edge::End: return YGEdgeEnd;
        case Edge::Horizontal: return YGEdgeHorizontal;
        case Edge::Vertical: return YGEdgeVertical;
        case Edge::All: return YGEdgeAll;
    }
}

YGValue toYogaValue(const MeasureValue& measure) {
    YGValue value;
    switch (measure.unit) {
        case MeasureUnit::Point:
            value.unit = YGUnitPoint;
            value.value = measure.value;
            break;
        case MeasureUnit::Percent:
            value.unit = YGUnitPercent;
            value.value = measure.value;
            break;
        case MeasureUnit::Auto:
            value.unit = YGUnitAuto;
            value.value = 0;
            break;
    }
    return value;
}

#endif // YOGA_AVAILABLE

} // anonymous namespace

// ============================================================================
// YogaNode Implementation
// ============================================================================

struct YogaNode::Impl {
#ifdef YOGA_AVAILABLE
    YGNodeRef node = nullptr;
#else
    // Stub implementation for when Yoga is not available
    bool initialized = false;
#endif
    LayoutConfig config;
    std::vector<std::shared_ptr<YogaNode>> children;
    MeasureCallback measureCallback;
    void* userData = nullptr;
    
    Impl() {
#ifdef YOGA_AVAILABLE
        node = YGNodeNew();
        YGNodeSetContext(node, this);
        initialized = true;
#endif
    }
    
    ~Impl() {
#ifdef YOGA_AVAILABLE
        if (node) {
            YGNodeFree(node);
        }
#endif
    }
    
    void applyConfig() {
#ifdef YOGA_AVAILABLE
        if (!node) return;
        
        // Flexbox properties
        YGNodeStyleSetFlexDirection(node, toYogaFlexDirection(config.flexDirection));
        YGNodeStyleSetJustifyContent(node, toYogaJustifyContent(config.justifyContent));
        YGNodeStyleSetAlignItems(node, toYogaAlignItems(config.alignItems));
        YGNodeStyleSetAlignSelf(node, toYogaAlignSelf(config.alignSelf));
        YGNodeStyleSetFlexWrap(node, toYogaFlexWrap(config.flexWrap));
        YGNodeStyleSetFlexGrow(node, config.flexGrow);
        YGNodeStyleSetFlexShrink(node, config.flexShrink);
        YGNodeStyleSetFlexBasis(node, toYogaValue(config.flexBasis));
        
        // Dimensions
        YGNodeStyleSetWidth(node, toYogaValue(config.width));
        YGNodeStyleSetHeight(node, toYogaValue(config.height));
        YGNodeStyleSetMinWidth(node, toYogaValue(config.minWidth));
        YGNodeStyleSetMinHeight(node, toYogaValue(config.minHeight));
        YGNodeStyleSetMaxWidth(node, toYogaValue(config.maxWidth));
        YGNodeStyleSetMaxHeight(node, toYogaValue(config.maxHeight));
        
        // Position
        YGNodeStyleSetPositionType(node, toYogaPositionType(config.positionType));
        YGNodeStyleSetPosition(node, YGEdgeLeft, toYogaValue(config.left));
        YGNodeStyleSetPosition(node, YGEdgeTop, toYogaValue(config.top));
        YGNodeStyleSetPosition(node, YGEdgeRight, toYogaValue(config.right));
        YGNodeStyleSetPosition(node, YGEdgeBottom, toYogaValue(config.bottom));
        
        // Margins
        YGNodeStyleSetMargin(node, YGEdgeLeft, toYogaValue(config.marginLeft));
        YGNodeStyleSetMargin(node, YGEdgeTop, toYogaValue(config.marginTop));
        YGNodeStyleSetMargin(node, YGEdgeRight, toYogaValue(config.marginRight));
        YGNodeStyleSetMargin(node, YGEdgeBottom, toYogaValue(config.marginBottom));
        
        // Padding
        YGNodeStyleSetPadding(node, YGEdgeLeft, toYogaValue(config.paddingLeft));
        YGNodeStyleSetPadding(node, YGEdgeTop, toYogaValue(config.paddingTop));
        YGNodeStyleSetPadding(node, YGEdgeRight, toYogaValue(config.paddingRight));
        YGNodeStyleSetPadding(node, YGEdgeBottom, toYogaValue(config.paddingBottom));
        
        // Border
        YGNodeStyleSetBorder(node, YGEdgeLeft, config.borderLeft);
        YGNodeStyleSetBorder(node, YGEdgeTop, config.borderTop);
        YGNodeStyleSetBorder(node, YGEdgeRight, config.borderRight);
        YGNodeStyleSetBorder(node, YGEdgeBottom, config.borderBottom);
        
        // Display
        YGNodeStyleSetDisplay(node, toYogaDisplay(config.display));
        
        // Aspect ratio
        if (!std::isnan(config.aspectRatio)) {
            YGNodeStyleSetAspectRatio(node, config.aspectRatio);
        }
#endif
    }
};

YogaNode::YogaNode() : pimpl(std::make_unique<Impl>()) {}

YogaNode::~YogaNode() = default;

YogaNode::YogaNode(YogaNode&& other) noexcept 
    : pimpl(std::move(other.pimpl)) {}

YogaNode& YogaNode::operator=(YogaNode&& other) noexcept {
    if (this != &other) {
        pimpl = std::move(other.pimpl);
    }
    return *this;
}

void YogaNode::configure(const LayoutConfig& config) {
    pimpl->config = config;
    pimpl->applyConfig();
}

YogaNode& YogaNode::setFlexDirection(FlexDirection direction) {
    pimpl->config.flexDirection = direction;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetFlexDirection(pimpl->node, toYogaFlexDirection(direction));
#endif
    return *this;
}

YogaNode& YogaNode::setJustifyContent(JustifyContent justify) {
    pimpl->config.justifyContent = justify;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetJustifyContent(pimpl->node, toYogaJustifyContent(justify));
#endif
    return *this;
}

YogaNode& YogaNode::setAlignItems(AlignItems align) {
    pimpl->config.alignItems = align;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetAlignItems(pimpl->node, toYogaAlignItems(align));
#endif
    return *this;
}

YogaNode& YogaNode::setAlignSelf(AlignSelf align) {
    pimpl->config.alignSelf = align;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetAlignSelf(pimpl->node, toYogaAlignSelf(align));
#endif
    return *this;
}

YogaNode& YogaNode::setFlexWrap(FlexWrap wrap) {
    pimpl->config.flexWrap = wrap;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetFlexWrap(pimpl->node, toYogaFlexWrap(wrap));
#endif
    return *this;
}

YogaNode& YogaNode::setFlexGrow(float grow) {
    pimpl->config.flexGrow = grow;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetFlexGrow(pimpl->node, grow);
#endif
    return *this;
}

YogaNode& YogaNode::setFlexShrink(float shrink) {
    pimpl->config.flexShrink = shrink;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetFlexShrink(pimpl->node, shrink);
#endif
    return *this;
}

YogaNode& YogaNode::setFlexBasis(const MeasureValue& basis) {
    pimpl->config.flexBasis = basis;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetFlexBasis(pimpl->node, toYogaValue(basis));
#endif
    return *this;
}

YogaNode& YogaNode::setWidth(const MeasureValue& width) {
    pimpl->config.width = width;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetWidth(pimpl->node, toYogaValue(width));
#endif
    return *this;
}

YogaNode& YogaNode::setHeight(const MeasureValue& height) {
    pimpl->config.height = height;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetHeight(pimpl->node, toYogaValue(height));
#endif
    return *this;
}

YogaNode& YogaNode::setMinWidth(const MeasureValue& width) {
    pimpl->config.minWidth = width;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetMinWidth(pimpl->node, toYogaValue(width));
#endif
    return *this;
}

YogaNode& YogaNode::setMinHeight(const MeasureValue& height) {
    pimpl->config.minHeight = height;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetMinHeight(pimpl->node, toYogaValue(height));
#endif
    return *this;
}

YogaNode& YogaNode::setMaxWidth(const MeasureValue& width) {
    pimpl->config.maxWidth = width;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetMaxWidth(pimpl->node, toYogaValue(width));
#endif
    return *this;
}

YogaNode& YogaNode::setMaxHeight(const MeasureValue& height) {
    pimpl->config.maxHeight = height;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetMaxHeight(pimpl->node, toYogaValue(height));
#endif
    return *this;
}

YogaNode& YogaNode::setPositionType(PositionType type) {
    pimpl->config.positionType = type;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetPositionType(pimpl->node, toYogaPositionType(type));
#endif
    return *this;
}

YogaNode& YogaNode::setLeft(const MeasureValue& left) {
    pimpl->config.left = left;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetPosition(pimpl->node, YGEdgeLeft, toYogaValue(left));
#endif
    return *this;
}

YogaNode& YogaNode::setTop(const MeasureValue& top) {
    pimpl->config.top = top;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetPosition(pimpl->node, YGEdgeTop, toYogaValue(top));
#endif
    return *this;
}

YogaNode& YogaNode::setRight(const MeasureValue& right) {
    pimpl->config.right = right;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetPosition(pimpl->node, YGEdgeRight, toYogaValue(right));
#endif
    return *this;
}

YogaNode& YogaNode::setBottom(const MeasureValue& bottom) {
    pimpl->config.bottom = bottom;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetPosition(pimpl->node, YGEdgeBottom, toYogaValue(bottom));
#endif
    return *this;
}

YogaNode& YogaNode::setMargin(Edge edge, const MeasureValue& margin) {
#ifdef YOGA_AVAILABLE
    YGEdge yogaEdge = toYogaEdge(edge);
    YGNodeStyleSetMargin(pimpl->node, yogaEdge, toYogaValue(margin));
#endif
    
    // Update config based on edge
    switch (edge) {
        case Edge::Left: pimpl->config.marginLeft = margin; break;
        case Edge::Top: pimpl->config.marginTop = margin; break;
        case Edge::Right: pimpl->config.marginRight = margin; break;
        case Edge::Bottom: pimpl->config.marginBottom = margin; break;
        default: break;
    }
    return *this;
}

YogaNode& YogaNode::setPadding(Edge edge, const MeasureValue& padding) {
#ifdef YOGA_AVAILABLE
    YGEdge yogaEdge = toYogaEdge(edge);
    YGNodeStyleSetPadding(pimpl->node, yogaEdge, toYogaValue(padding));
#endif
    
    switch (edge) {
        case Edge::Left: pimpl->config.paddingLeft = padding; break;
        case Edge::Top: pimpl->config.paddingTop = padding; break;
        case Edge::Right: pimpl->config.paddingRight = padding; break;
        case Edge::Bottom: pimpl->config.paddingBottom = padding; break;
        default: break;
    }
    return *this;
}

YogaNode& YogaNode::setBorder(Edge edge, float width) {
#ifdef YOGA_AVAILABLE
    YGEdge yogaEdge = toYogaEdge(edge);
    YGNodeStyleSetBorder(pimpl->node, yogaEdge, width);
#endif
    
    switch (edge) {
        case Edge::Left: pimpl->config.borderLeft = width; break;
        case Edge::Top: pimpl->config.borderTop = width; break;
        case Edge::Right: pimpl->config.borderRight = width; break;
        case Edge::Bottom: pimpl->config.borderBottom = width; break;
        default: break;
    }
    return *this;
}

YogaNode& YogaNode::setDisplay(DisplayType display) {
    pimpl->config.display = display;
#ifdef YOGA_AVAILABLE
    YGNodeStyleSetDisplay(pimpl->node, toYogaDisplay(display));
#endif
    return *this;
}

YogaNode& YogaNode::setAspectRatio(float ratio) {
    pimpl->config.aspectRatio = ratio;
#ifdef YOGA_AVAILABLE
    if (!std::isnan(ratio)) {
        YGNodeStyleSetAspectRatio(pimpl->node, ratio);
    }
#endif
    return *this;
}

void YogaNode::addChild(std::shared_ptr<YogaNode> child) {
    pimpl->children.push_back(child);
#ifdef YOGA_AVAILABLE
    YGNodeInsertChild(pimpl->node, child->pimpl->node, 
                      static_cast<uint32_t>(pimpl->children.size() - 1));
#endif
}

void YogaNode::insertChild(std::shared_ptr<YogaNode> child, size_t index) {
    if (index >= pimpl->children.size()) {
        addChild(child);
        return;
    }
    pimpl->children.insert(pimpl->children.begin() + index, child);
#ifdef YOGA_AVAILABLE
    YGNodeInsertChild(pimpl->node, child->pimpl->node, static_cast<uint32_t>(index));
#endif
}

void YogaNode::removeChild(std::shared_ptr<YogaNode> child) {
    auto it = std::find(pimpl->children.begin(), pimpl->children.end(), child);
    if (it != pimpl->children.end()) {
#ifdef YOGA_AVAILABLE
        YGNodeRemoveChild(pimpl->node, child->pimpl->node);
#endif
        pimpl->children.erase(it);
    }
}

void YogaNode::removeAllChildren() {
#ifdef YOGA_AVAILABLE
    uint32_t count = YGNodeGetChildCount(pimpl->node);
    for (uint32_t i = 0; i < count; ++i) {
        YGNodeRef child = YGNodeGetChild(pimpl->node, i);
        YGNodeRemoveChild(pimpl->node, child);
    }
#endif
    pimpl->children.clear();
}

size_t YogaNode::getChildCount() const {
    return pimpl->children.size();
}

std::shared_ptr<YogaNode> YogaNode::getChildAt(size_t index) {
    if (index >= pimpl->children.size()) {
        return nullptr;
    }
    return pimpl->children[index];
}

void YogaNode::setMeasureFunction(MeasureCallback callback) {
    pimpl->measureCallback = callback;
    
#ifdef YOGA_AVAILABLE
    if (callback) {
        YGNodeSetMeasureFunc(pimpl->node, 
            [](YGNodeRef node, float width, YGMeasureMode widthMode,
               float height, YGMeasureMode heightMode) -> YGSize {
                auto* impl = static_cast<Impl*>(YGNodeGetContext(node));
                if (!impl || !impl->measureCallback) {
                    return {0, 0};
                }
                
                float availWidth = (widthMode == YGMeasureModeUndefined) ? NAN : width;
                float availHeight = (heightMode == YGMeasureModeUndefined) ? NAN : height;
                
                auto result = impl->measureCallback(availWidth, availHeight);
                return {result.first, result.second};
            });
    } else {
        YGNodeSetMeasureFunc(pimpl->node, nullptr);
    }
#endif
}

void YogaNode::clearMeasureFunction() {
    pimpl->measureCallback = nullptr;
#ifdef YOGA_AVAILABLE
    YGNodeSetMeasureFunc(pimpl->node, nullptr);
#endif
}

void YogaNode::calculateLayout(float availableWidth, float availableHeight) {
#ifdef YOGA_AVAILABLE
    YGNodeCalculateLayout(pimpl->node, availableWidth, availableHeight);
#else
    // Stub: Set default layout values
    (void)availableWidth;
    (void)availableHeight;
#endif
}

LayoutResult YogaNode::getLayout() const {
    LayoutResult result;
    
#ifdef YOGA_AVAILABLE
    result.x = YGNodeLayoutGetLeft(pimpl->node);
    result.y = YGNodeLayoutGetTop(pimpl->node);
    result.width = YGNodeLayoutGetWidth(pimpl->node);
    result.height = YGNodeLayoutGetHeight(pimpl->node);
    
    result.marginLeft = YGNodeLayoutGetMargin(pimpl->node, YGEdgeLeft);
    result.marginTop = YGNodeLayoutGetMargin(pimpl->node, YGEdgeTop);
    result.marginRight = YGNodeLayoutGetMargin(pimpl->node, YGEdgeRight);
    result.marginBottom = YGNodeLayoutGetMargin(pimpl->node, YGEdgeBottom);
    
    result.paddingLeft = YGNodeLayoutGetPadding(pimpl->node, YGEdgeLeft);
    result.paddingTop = YGNodeLayoutGetPadding(pimpl->node, YGEdgeTop);
    result.paddingRight = YGNodeLayoutGetPadding(pimpl->node, YGEdgeRight);
    result.paddingBottom = YGNodeLayoutGetPadding(pimpl->node, YGEdgeBottom);
    
    result.borderLeft = YGNodeLayoutGetBorder(pimpl->node, YGEdgeLeft);
    result.borderTop = YGNodeLayoutGetBorder(pimpl->node, YGEdgeTop);
    result.borderRight = YGNodeLayoutGetBorder(pimpl->node, YGEdgeRight);
    result.borderBottom = YGNodeLayoutGetBorder(pimpl->node, YGEdgeBottom);
#endif
    
    return result;
}

void YogaNode::markDirty() {
#ifdef YOGA_AVAILABLE
    YGNodeMarkDirty(pimpl->node);
#endif
}

bool YogaNode::isDirty() const {
#ifdef YOGA_AVAILABLE
    return YGNodeIsDirty(pimpl->node);
#else
    return false;
#endif
}

YGNodeRef YogaNode::getNativeNode() const {
#ifdef YOGA_AVAILABLE
    return pimpl->node;
#else
    return nullptr;
#endif
}

void YogaNode::setUserData(void* data) {
    pimpl->userData = data;
}

void* YogaNode::getUserData() const {
    return pimpl->userData;
}

// ============================================================================
// LayoutContext Implementation
// ============================================================================

std::shared_ptr<YogaNode> LayoutContext::createRoot() {
    auto node = std::make_shared<YogaNode>();
    return node;
}

std::shared_ptr<YogaNode> LayoutContext::createNode() {
    return std::make_shared<YogaNode>();
}

void LayoutContext::calculateTreeLayout(std::shared_ptr<YogaNode> root, 
                                        float width, float height) {
    if (root) {
        root->calculateLayout(width, height);
    }
}

void LayoutContext::applyLayout(std::shared_ptr<YogaNode> root, 
                                LayoutApplyCallback callback) {
    if (!root || !callback) return;
    
    // Apply to current node
    void* userData = root->getUserData();
    LayoutResult layout = root->getLayout();
    callback(userData, layout);
    
    // Recursively apply to children
    for (size_t i = 0; i < root->getChildCount(); ++i) {
        auto child = root->getChildAt(i);
        applyLayout(child, callback);
    }
}

void LayoutContext::enableLogging(bool enabled) {
#ifdef YOGA_AVAILABLE
    // YGConfigSetUseWebDefaults or similar logging setup
    (void)enabled;
#endif
}

void LayoutContext::printLayoutTree(std::shared_ptr<YogaNode> root, int indent) {
    if (!root) return;
    
    std::string prefix(indent * 2, ' ');
    LayoutResult layout = root->getLayout();
    
    std::cout << prefix << "Node: " << layout.toString() << std::endl;
    
    for (size_t i = 0; i < root->getChildCount(); ++i) {
        printLayoutTree(root->getChildAt(i), indent + 1);
    }
}

// ============================================================================
// LayoutBuilder Implementation
// ============================================================================

LayoutBuilder::LayoutBuilder(std::shared_ptr<YogaNode> node) 
    : rootNode(node), currentNode(node) {}

LayoutBuilder& LayoutBuilder::flexDirection(FlexDirection dir) {
    currentNode->setFlexDirection(dir);
    return *this;
}

LayoutBuilder& LayoutBuilder::justifyContent(JustifyContent justify) {
    currentNode->setJustifyContent(justify);
    return *this;
}

LayoutBuilder& LayoutBuilder::alignItems(AlignItems align) {
    currentNode->setAlignItems(align);
    return *this;
}

LayoutBuilder& LayoutBuilder::alignSelf(AlignSelf align) {
    currentNode->setAlignSelf(align);
    return *this;
}

LayoutBuilder& LayoutBuilder::flexWrap(FlexWrap wrap) {
    currentNode->setFlexWrap(wrap);
    return *this;
}

LayoutBuilder& LayoutBuilder::flexGrow(float grow) {
    currentNode->setFlexGrow(grow);
    return *this;
}

LayoutBuilder& LayoutBuilder::flexShrink(float shrink) {
    currentNode->setFlexShrink(shrink);
    return *this;
}

LayoutBuilder& LayoutBuilder::flexBasis(const MeasureValue& basis) {
    currentNode->setFlexBasis(basis);
    return *this;
}

LayoutBuilder& LayoutBuilder::width(float pts) {
    currentNode->setWidth(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::widthPercent(float pct) {
    currentNode->setWidth(MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::height(float pts) {
    currentNode->setHeight(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::heightPercent(float pct) {
    currentNode->setHeight(MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::minWidth(float pts) {
    currentNode->setMinWidth(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::minHeight(float pts) {
    currentNode->setMinHeight(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::maxWidth(float pts) {
    currentNode->setMaxWidth(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::maxHeight(float pts) {
    currentNode->setMaxHeight(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::positionAbsolute() {
    currentNode->setPositionType(PositionType::Absolute);
    return *this;
}

LayoutBuilder& LayoutBuilder::positionRelative() {
    currentNode->setPositionType(PositionType::Relative);
    return *this;
}

LayoutBuilder& LayoutBuilder::left(float pts) {
    currentNode->setLeft(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::leftPercent(float pct) {
    currentNode->setLeft(MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::top(float pts) {
    currentNode->setTop(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::topPercent(float pct) {
    currentNode->setTop(MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::right(float pts) {
    currentNode->setRight(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::rightPercent(float pct) {
    currentNode->setRight(MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::bottom(float pts) {
    currentNode->setBottom(MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::bottomPercent(float pct) {
    currentNode->setBottom(MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::margin(float pts) {
    currentNode->setMargin(Edge::Left, MeasureValue::points(pts));
    currentNode->setMargin(Edge::Right, MeasureValue::points(pts));
    currentNode->setMargin(Edge::Top, MeasureValue::points(pts));
    currentNode->setMargin(Edge::Bottom, MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::margin(float horizontal, float vertical) {
    currentNode->setMargin(Edge::Left, MeasureValue::points(horizontal));
    currentNode->setMargin(Edge::Right, MeasureValue::points(horizontal));
    currentNode->setMargin(Edge::Top, MeasureValue::points(vertical));
    currentNode->setMargin(Edge::Bottom, MeasureValue::points(vertical));
    return *this;
}

LayoutBuilder& LayoutBuilder::margin(float left, float top, float right, float bottom) {
    currentNode->setMargin(Edge::Left, MeasureValue::points(left));
    currentNode->setMargin(Edge::Top, MeasureValue::points(top));
    currentNode->setMargin(Edge::Right, MeasureValue::points(right));
    currentNode->setMargin(Edge::Bottom, MeasureValue::points(bottom));
    return *this;
}

LayoutBuilder& LayoutBuilder::marginPercent(float pct) {
    currentNode->setMargin(Edge::All, MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::padding(float pts) {
    currentNode->setPadding(Edge::Left, MeasureValue::points(pts));
    currentNode->setPadding(Edge::Right, MeasureValue::points(pts));
    currentNode->setPadding(Edge::Top, MeasureValue::points(pts));
    currentNode->setPadding(Edge::Bottom, MeasureValue::points(pts));
    return *this;
}

LayoutBuilder& LayoutBuilder::padding(float horizontal, float vertical) {
    currentNode->setPadding(Edge::Left, MeasureValue::points(horizontal));
    currentNode->setPadding(Edge::Right, MeasureValue::points(horizontal));
    currentNode->setPadding(Edge::Top, MeasureValue::points(vertical));
    currentNode->setPadding(Edge::Bottom, MeasureValue::points(vertical));
    return *this;
}

LayoutBuilder& LayoutBuilder::padding(float left, float top, float right, float bottom) {
    currentNode->setPadding(Edge::Left, MeasureValue::points(left));
    currentNode->setPadding(Edge::Top, MeasureValue::points(top));
    currentNode->setPadding(Edge::Right, MeasureValue::points(right));
    currentNode->setPadding(Edge::Bottom, MeasureValue::points(bottom));
    return *this;
}

LayoutBuilder& LayoutBuilder::paddingPercent(float pct) {
    currentNode->setPadding(Edge::All, MeasureValue::percent(pct));
    return *this;
}

LayoutBuilder& LayoutBuilder::border(float width) {
    currentNode->setBorder(Edge::All, width);
    return *this;
}

LayoutBuilder& LayoutBuilder::aspectRatio(float ratio) {
    currentNode->setAspectRatio(ratio);
    return *this;
}

LayoutBuilder& LayoutBuilder::display(DisplayType display) {
    currentNode->setDisplay(display);
    return *this;
}

LayoutBuilder LayoutBuilder::child() {
    parentStack.push_back(currentNode);
    auto childNode = std::make_shared<YogaNode>();
    currentNode->addChild(childNode);
    currentNode = childNode;
    return LayoutBuilder(childNode);
}

LayoutBuilder& LayoutBuilder::endChild() {
    if (!parentStack.empty()) {
        currentNode = parentStack.back();
        parentStack.pop_back();
    }
    return *this;
}

std::shared_ptr<YogaNode> LayoutBuilder::getNode() const {
    return rootNode;
}

} // namespace ui
} // namespace zenith
