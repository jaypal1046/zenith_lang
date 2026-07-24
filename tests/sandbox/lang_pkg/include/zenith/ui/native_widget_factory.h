/**
 * Zenith Native Widget Factory
 * 
 * Creates native platform widgets from Zenith UIElements.
 * Maps Zenith widget types to native controls (Win32, Cocoa, GTK).
 */

#ifndef ZENITH_NATIVE_WIDGET_FACTORY_H
#define ZENITH_NATIVE_WIDGET_FACTORY_H

#include "native_window.h"
#include "../common/zenith_common.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace zenith {
namespace ui {

/**
 * @brief Abstract native widget interface
 */
class NativeWidget {
public:
    virtual ~NativeWidget() = default;
    virtual void* getNativeHandle() = 0;
    virtual void setPosition(int x, int y) = 0;
    virtual void setSize(int width, int height) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual void setVisible(bool visible) = 0;
    virtual void setText(const std::string& text) = 0;
    virtual std::string getText() const = 0;
};

/**
 * @brief Factory for creating native widgets from UIElements
 * 
 * Platform-specific implementations create appropriate native controls:
 * - Win32: CreateWindowEx with BUTTON, EDIT, etc.
 * - Cocoa: NSButton, NSTextField, etc.
 * - GTK: GtkButton, GtkEntry, etc.
 */
class NativeWidgetFactory {
public:
    virtual ~NativeWidgetFactory() = default;
    
    /**
     * @brief Create a native button
     * @param element Zenith UIElement describing the button
     * @param parent Parent native window/container handle
     * @return Native widget instance
     */
    virtual std::unique_ptr<NativeWidget> createButton(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Create a native text field
     */
    virtual std::unique_ptr<NativeWidget> createTextField(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Create a native checkbox
     */
    virtual std::unique_ptr<NativeWidget> createCheckbox(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Create a native slider
     */
    virtual std::unique_ptr<NativeWidget> createSlider(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Create a native dropdown/combo box
     */
    virtual std::unique_ptr<NativeWidget> createDropdown(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Create a native text label
     */
    virtual std::unique_ptr<NativeWidget> createLabel(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Create a container/native view for layout
     */
    virtual std::unique_ptr<NativeWidget> createContainer(
        const UIElement& element, 
        void* parent
    ) = 0;
    
    /**
     * @brief Set callback for widget events
     * @param widget Native widget to monitor
     * @param callback Function to call on events
     */
    virtual void setCallback(
        NativeWidget* widget, 
        std::function<void(const std::string& action, const std::string& value)> callback
    ) = 0;
};

/**
 * @brief Factory function to create platform-appropriate widget factory
 * @return Unique pointer to NativeWidgetFactory implementation
 */
std::unique_ptr<NativeWidgetFactory> createNativeWidgetFactory();

} // namespace ui
} // namespace zenith

#endif // ZENITH_NATIVE_WIDGET_FACTORY_H
