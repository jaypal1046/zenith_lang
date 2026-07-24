/**
 * Zenith UI - Yoga Layout Engine Integration
 * 
 * Provides flexbox layout capabilities using Facebook's Yoga engine
 * for responsive, cross-platform UI design in Zenith applications.
 */

#ifndef ZENITH_UI_YOGA_LAYOUT_H
#define ZENITH_UI_YOGA_LAYOUT_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <cmath>

// Forward declaration of Yoga types (actual includes handled in implementation)
struct YGNode;
typedef YGNode* YGNodeRef;

namespace zenith {
namespace ui {

/**
 * @brief Enumerations for Yoga layout properties
 */
enum class FlexDirection {
    Row,
    RowReverse,
    Column,
    ColumnReverse
};

enum class JustifyContent {
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class AlignItems {
    FlexStart,
    FlexEnd,
    Center,
    Baseline,
    Stretch
};

enum class AlignSelf {
    Auto,
    FlexStart,
    FlexEnd,
    Center,
    Baseline,
    Stretch
};

enum class FlexWrap {
    NoWrap,
    Wrap,
    WrapReverse
};

enum class PositionType {
    Relative,
    Absolute
};

enum class DisplayType {
    Flex,
    None
};

/**
 * @brief Edge types for margins, paddings, and borders
 */
enum class Edge {
    Left,
    Top,
    Right,
    Bottom,
    Start,
    End,
    Horizontal,
    Vertical,
    All
};

/**
 * @brief Unit types for measurements
 */
enum class MeasureUnit {
    Point,      // Absolute points
    Percent,    // Percentage of parent
    Auto        // Automatic sizing
};

/**
 * @brief Measurement value with unit
 */
struct MeasureValue {
    float value = 0.0f;
    MeasureUnit unit = MeasureUnit::Point;
    
    static MeasureValue points(float pts) {
        return {pts, MeasureUnit::Point};
    }
    
    static MeasureValue percent(float pct) {
        return {pct, MeasureUnit::Percent};
    }
    
    static MeasureValue auto_value() {
        return {0.0f, MeasureUnit::Auto};
    }
};

/**
 * @brief Layout configuration for a UI node
 */
struct LayoutConfig {
    // Flexbox properties
    FlexDirection flexDirection = FlexDirection::Column;
    JustifyContent justifyContent = JustifyContent::FlexStart;
    AlignItems alignItems = AlignItems::Stretch;
    AlignSelf alignSelf = AlignSelf::Auto;
    FlexWrap flexWrap = FlexWrap::NoWrap;
    float flexGrow = 0.0f;
    float flexShrink = 1.0f;
    MeasureValue flexBasis = MeasureValue::auto_value();
    
    // Dimensions
    MeasureValue width = MeasureValue::auto_value();
    MeasureValue height = MeasureValue::auto_value();
    MeasureValue minWidth = MeasureValue::points(0);
    MeasureValue minHeight = MeasureValue::points(0);
    MeasureValue maxWidth = MeasureValue::auto_value();
    MeasureValue maxHeight = MeasureValue::auto_value();
    
    // Position
    PositionType positionType = PositionType::Relative;
    MeasureValue left = MeasureValue::auto_value();
    MeasureValue top = MeasureValue::auto_value();
    MeasureValue right = MeasureValue::auto_value();
    MeasureValue bottom = MeasureValue::auto_value();
    
    // Margins
    MeasureValue marginLeft = MeasureValue::points(0);
    MeasureValue marginTop = MeasureValue::points(0);
    MeasureValue marginRight = MeasureValue::points(0);
    MeasureValue marginBottom = MeasureValue::points(0);
    
    // Padding
    MeasureValue paddingLeft = MeasureValue::points(0);
    MeasureValue paddingTop = MeasureValue::points(0);
    MeasureValue paddingRight = MeasureValue::points(0);
    MeasureValue paddingBottom = MeasureValue::points(0);
    
    // Border
    float borderLeft = 0.0f;
    float borderTop = 0.0f;
    float borderRight = 0.0f;
    float borderBottom = 0.0f;
    
    // Display
    DisplayType display = DisplayType::Flex;
    
    // Aspect ratio (width/height)
    float aspectRatio = NAN;
};

/**
 * @brief Computed layout result after calculation
 */
struct LayoutResult {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    
    // Computed margins
    float marginLeft = 0.0f;
    float marginTop = 0.0f;
    float marginRight = 0.0f;
    float marginBottom = 0.0f;
    
    // Computed padding
    float paddingLeft = 0.0f;
    float paddingTop = 0.0f;
    float paddingRight = 0.0f;
    float paddingBottom = 0.0f;
    
    // Computed border
    float borderLeft = 0.0f;
    float borderTop = 0.0f;
    float borderRight = 0.0f;
    float borderBottom = 0.0f;
    
    std::string toString() const {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), 
                 "Layout[x=%.2f, y=%.2f, w=%.2f, h=%.2f]",
                 x, y, width, height);
        return std::string(buffer);
    }
};

/**
 * @brief Callback type for custom measure functions
 * 
 * @param width Available width (NAN if unconstrained)
 * @param height Available height (NAN if unconstrained)
 * @return Measured dimensions {width, height}
 */
using MeasureCallback = std::function<std::pair<float, float>(float, float)>;

/**
 * @brief Yoga Layout Node wrapper
 * 
 * Provides a high-level C++ interface to Facebook's Yoga layout engine
 * for creating responsive, flexbox-based layouts in Zenith UI applications.
 */
class YogaNode {
public:
    /**
     * @brief Create a new Yoga layout node
     */
    YogaNode();
    
    /**
     * @brief Destructor - cleans up Yoga resources
     */
    ~YogaNode();
    
    // Prevent copying
    YogaNode(const YogaNode&) = delete;
    YogaNode& operator=(const YogaNode&) = delete;
    
    // Allow moving
    YogaNode(YogaNode&& other) noexcept;
    YogaNode& operator=(YogaNode&& other) noexcept;
    
    /**
     * @brief Configure the node with layout properties
     */
    void configure(const LayoutConfig& config);
    
    /**
     * @brief Set individual layout properties (fluent interface)
     */
    YogaNode& setFlexDirection(FlexDirection direction);
    YogaNode& setJustifyContent(JustifyContent justify);
    YogaNode& setAlignItems(AlignItems align);
    YogaNode& setAlignSelf(AlignSelf align);
    YogaNode& setFlexWrap(FlexWrap wrap);
    YogaNode& setFlexGrow(float grow);
    YogaNode& setFlexShrink(float shrink);
    YogaNode& setFlexBasis(const MeasureValue& basis);
    
    YogaNode& setWidth(const MeasureValue& width);
    YogaNode& setHeight(const MeasureValue& height);
    YogaNode& setMinWidth(const MeasureValue& width);
    YogaNode& setMinHeight(const MeasureValue& height);
    YogaNode& setMaxWidth(const MeasureValue& width);
    YogaNode& setMaxHeight(const MeasureValue& height);
    
    YogaNode& setPositionType(PositionType type);
    YogaNode& setLeft(const MeasureValue& left);
    YogaNode& setTop(const MeasureValue& top);
    YogaNode& setRight(const MeasureValue& right);
    YogaNode& setBottom(const MeasureValue& bottom);
    
    YogaNode& setMargin(Edge edge, const MeasureValue& margin);
    YogaNode& setPadding(Edge edge, const MeasureValue& padding);
    YogaNode& setBorder(Edge edge, float width);
    
    YogaNode& setDisplay(DisplayType display);
    YogaNode& setAspectRatio(float ratio);
    
    /**
     * @brief Add a child node to this node
     */
    void addChild(std::shared_ptr<YogaNode> child);
    
    /**
     * @brief Insert a child at a specific index
     */
    void insertChild(std::shared_ptr<YogaNode> child, size_t index);
    
    /**
     * @brief Remove a child node
     */
    void removeChild(std::shared_ptr<YogaNode> child);
    
    /**
     * @brief Remove all children
     */
    void removeAllChildren();
    
    /**
     * @brief Get child count
     */
    size_t getChildCount() const;
    
    /**
     * @brief Get child at index
     */
    std::shared_ptr<YogaNode> getChildAt(size_t index);
    
    /**
     * @brief Set custom measure function for content sizing
     */
    void setMeasureFunction(MeasureCallback callback);
    
    /**
     * @brief Clear measure function
     */
    void clearMeasureFunction();
    
    /**
     * @brief Calculate layout for the node tree
     * 
     * @param availableWidth Available width constraint
     * @param availableHeight Available height constraint
     */
    void calculateLayout(float availableWidth, float availableHeight);
    
    /**
     * @brief Get the computed layout result
     */
    LayoutResult getLayout() const;
    
    /**
     * @brief Mark the node as dirty (needs recalculation)
     */
    void markDirty();
    
    /**
     * @brief Check if the node is dirty
     */
    bool isDirty() const;
    
    /**
     * @brief Get the underlying Yoga node pointer (for advanced usage)
     */
    YGNodeRef getNativeNode() const;
    
    /**
     * @brief Associate user data with this node
     */
    void setUserData(void* data);
    
    /**
     * @brief Get user data associated with this node
     */
    void* getUserData() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

/**
 * @brief Layout context for managing a complete UI tree
 */
class LayoutContext {
public:
    /**
     * @brief Create a root layout node
     */
    std::shared_ptr<YogaNode> createRoot();
    
    /**
     * @brief Create a new child node
     */
    std::shared_ptr<YogaNode> createNode();
    
    /**
     * @brief Calculate layout for entire tree from root
     */
    void calculateTreeLayout(std::shared_ptr<YogaNode> root, 
                            float width, float height);
    
    /**
     * @brief Apply layout results to actual UI elements
     * 
     * @param callback Function called for each node with its computed layout
     */
    using LayoutApplyCallback = std::function<void(void* userData, const LayoutResult&)>;
    void applyLayout(std::shared_ptr<YogaNode> root, LayoutApplyCallback callback);
    
    /**
     * @brief Enable/disable logging of layout calculations
     */
    static void enableLogging(bool enabled);
    
    /**
     * @brief Print layout tree for debugging
     */
    static void printLayoutTree(std::shared_ptr<YogaNode> root, int indent = 0);
};

/**
 * @brief Helper class for building layouts with a fluent API
 */
class LayoutBuilder {
public:
    explicit LayoutBuilder(std::shared_ptr<YogaNode> node);
    
    // Flexbox
    LayoutBuilder& flexDirection(FlexDirection dir);
    LayoutBuilder& justifyContent(JustifyContent justify);
    LayoutBuilder& alignItems(AlignItems align);
    LayoutBuilder& alignSelf(AlignSelf align);
    LayoutBuilder& flexWrap(FlexWrap wrap);
    LayoutBuilder& flexGrow(float grow);
    LayoutBuilder& flexShrink(float shrink);
    LayoutBuilder& flexBasis(const MeasureValue& basis);
    
    // Size
    LayoutBuilder& width(float pts);
    LayoutBuilder& widthPercent(float pct);
    LayoutBuilder& height(float pts);
    LayoutBuilder& heightPercent(float pct);
    LayoutBuilder& minWidth(float pts);
    LayoutBuilder& minHeight(float pts);
    LayoutBuilder& maxWidth(float pts);
    LayoutBuilder& maxHeight(float pts);
    
    // Position
    LayoutBuilder& positionAbsolute();
    LayoutBuilder& positionRelative();
    LayoutBuilder& left(float pts);
    LayoutBuilder& leftPercent(float pct);
    LayoutBuilder& top(float pts);
    LayoutBuilder& topPercent(float pct);
    LayoutBuilder& right(float pts);
    LayoutBuilder& rightPercent(float pct);
    LayoutBuilder& bottom(float pts);
    LayoutBuilder& bottomPercent(float pct);
    
    // Margin
    LayoutBuilder& margin(float pts);
    LayoutBuilder& margin(float horizontal, float vertical);
    LayoutBuilder& margin(float left, float top, float right, float bottom);
    LayoutBuilder& marginPercent(float pct);
    
    // Padding
    LayoutBuilder& padding(float pts);
    LayoutBuilder& padding(float horizontal, float vertical);
    LayoutBuilder& padding(float left, float top, float right, float bottom);
    LayoutBuilder& paddingPercent(float pct);
    
    // Border
    LayoutBuilder& border(float width);
    
    // Other
    LayoutBuilder& aspectRatio(float ratio);
    LayoutBuilder& display(DisplayType display);
    
    /**
     * @brief Add a child node and return builder for it
     */
    LayoutBuilder child();
    
    /**
     * @brief Finish building child and return to parent
     */
    LayoutBuilder& endChild();
    
    /**
     * @brief Get the built node
     */
    std::shared_ptr<YogaNode> getNode() const;

private:
    std::shared_ptr<YogaNode> rootNode;
    std::shared_ptr<YogaNode> currentNode;
    std::vector<std::shared_ptr<YogaNode>> parentStack;
};

} // namespace ui
} // namespace zenith

#endif // ZENITH_UI_YOGA_LAYOUT_H
