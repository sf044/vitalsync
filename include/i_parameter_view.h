/**
 * @file i_parameter_view.h
 * @brief Interface for parameter view components
 * 
 * This interface defines the contract that all parameter view components must implement.
 * Parameter views are responsible for rendering parameter data on screen and providing
 * user interaction with the parameter display. They connect to parameter models to receive
 * data updates and reflect changes in the UI automatically.
 * 
 * Parameter views handle various display aspects including:
 * - Numerical value display with appropriate formatting
 * - Parameter labels and units
 * - Visual alarm indicators with appropriate color coding
 * - Font sizes and visual styling
 */

#ifndef I_PARAMETER_VIEW_H
#define I_PARAMETER_VIEW_H

#include <QWidget>
#include <memory>
#include "i_parameter_model.h"

/**
 * @brief Interface for parameter view components
 * 
 * This interface defines the contract that all parameter view components must implement.
 * It provides methods for connecting to a parameter model, configuring display options,
 * and managing the view's lifecycle. Parameter views are typically used to display vital
 * signs such as heart rate, blood pressure, SpO2, and other physiological parameters.
 * 
 * A parameter view implementation is responsible for:
 * - Displaying the current parameter value in an appropriate format
 * - Showing alarm states visually when parameters are outside normal ranges
 * - Providing appropriate visual styling and configuration options
 * - Updating automatically when the underlying model changes
 */
class IParameterView {
public:
    /**
     * @brief Virtual destructor
     * 
     * Ensures proper cleanup when derived classes are destroyed through
     * a pointer to this interface.
     */
    virtual ~IParameterView() = default;

    /**
     * @brief Get the widget for this view
     * @return Pointer to the widget
     * 
     * Returns a pointer to the Qt widget that represents this parameter view.
     * This widget can be added to layouts or set as a central widget in windows.
     */
    virtual QWidget* GetWidget() = 0;

    /**
     * @brief Set the parameter model to display
     * @param model Shared pointer to the parameter model
     * 
     * Associates this view with a parameter model. The view will connect to the model's
     * signals to receive updates when the parameter value or properties change. If a
     * previous model was set, the view should disconnect from it appropriately.
     */
    virtual void SetModel(std::shared_ptr<IParameterModel> model) = 0;

    /**
     * @brief Get the associated parameter model
     * @return Shared pointer to the parameter model or nullptr if none
     * 
     * Returns the parameter model currently associated with this view.
     * May return nullptr if no model has been set.
     */
    virtual std::shared_ptr<IParameterModel> GetModel() const = 0;

    /**
     * @brief Set whether to show the parameter label
     * @param visible True to show label, false to hide
     * 
     * Controls the visibility of the parameter label (e.g., "HR", "SpO2").
     * This can be used to create more compact views when screen space is limited.
     */
    virtual void SetLabelVisible(bool visible) = 0;

    /**
     * @brief Set whether to show the parameter unit
     * @param visible True to show unit, false to hide
     * 
     * Controls the visibility of the parameter unit (e.g., "bpm", "mmHg").
     * This can be used to create more compact views when screen space is limited.
     */
    virtual void SetUnitVisible(bool visible) = 0;

    /**
     * @brief Set the font size for the parameter value
     * @param size Font size in points
     * 
     * Sets the font size used to display the parameter value.
     * Larger font sizes may be used for better visibility in clinical settings.
     */
    virtual void SetValueFontSize(int size) = 0;

    /**
     * @brief Set the font size for the parameter label
     * @param size Font size in points
     * 
     * Sets the font size used for the parameter label and unit.
     * Typically smaller than the value font size.
     */
    virtual void SetLabelFontSize(int size) = 0;

    /**
     * @brief Set the background color
     * @param color Background color
     * 
     * Sets the background color for the parameter view in normal state.
     * This color is used when no alarm conditions are present.
     */
    virtual void SetBackgroundColor(const QColor& color) = 0;

    /**
     * @brief Set the text color for normal state
     * @param color Text color
     * 
     * Sets the text color for the parameter view in normal state.
     * This color is used when no alarm conditions are present.
     */
    virtual void SetTextColor(const QColor& color) = 0;

    /**
     * @brief Set the background color for alarm states
     * @param state The alarm state (warning or critical)
     * @param color Color for the specified alarm state
     * 
     * Sets the background color to use when the parameter is in the specified
     * alarm state. Typically, warning states use yellow and critical states use red,
     * but this allows for customization of the alarm appearance.
     */
    virtual void SetAlarmBackgroundColor(IParameterModel::AlarmState state, const QColor& color) = 0;

    /**
     * @brief Set the text color for alarm states
     * @param state The alarm state (warning or critical)
     * @param color Color for the specified alarm state
     * 
     * Sets the text color to use when the parameter is in the specified
     * alarm state. This allows for customization of the alarm appearance.
     */
    virtual void SetAlarmTextColor(IParameterModel::AlarmState state, const QColor& color) = 0;

    /**
     * @brief Update the view to reflect the current model state
     * 
     * Forces an immediate update of the view to reflect the current state of the
     * associated parameter model. This is typically called automatically when the
     * model changes, but can be called manually if needed.
     */
    virtual void update() = 0;
};

#endif // I_PARAMETER_VIEW_H 