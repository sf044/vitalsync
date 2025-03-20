/**
 * @file i_parameter_model.h
 * @brief Interface for parameter data models
 * 
 * This interface defines the contract that all parameter data models must implement.
 * Parameter models are responsible for storing, processing, and providing access
 * to vital sign parameter data to be displayed by UI components.
 * 
 * Parameter models handle various aspects of physiological parameters including:
 * - Current parameter values and timestamps
 * - Parameter metadata (units, display names)
 * - Alarm limits and current alarm state
 * - Visual properties like display color
 * - Active/inactive state management
 */

#ifndef I_PARAMETER_MODEL_H
#define I_PARAMETER_MODEL_H

#include <QObject>
#include <QColor>
#include <QDateTime>
#include "vital_sync_types.h"

/**
 * @brief Interface for parameter models
 * 
 * This interface defines the contract for classes that manage physiological parameter data,
 * such as heart rate, blood pressure, SpO2, and other vital signs. Parameter models
 * process raw data from data providers, maintain the current value, track alarm states,
 * and notify views when changes occur.
 * 
 * Parameter models are typically created by the DataManager and associated with views
 * to create complete parameter display components. They emit signals when parameter values
 * or properties change, allowing views to update in response.
 */
class IParameterModel : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Alarm states for parameters
     * 
     * Defines the possible alarm states for a parameter value. These states are used
     * to visually indicate when a parameter is outside normal ranges, with different
     * levels of severity.
     */
    enum class AlarmState {
        Normal,       ///< Parameter is within normal range
        HighWarning,  ///< Parameter is above warning threshold
        HighCritical, ///< Parameter is above critical threshold
        LowWarning,   ///< Parameter is below warning threshold
        LowCritical,  ///< Parameter is below critical threshold
        Technical     ///< Technical alarm (e.g., sensor disconnected)
    };

    /**
     * @brief Constructor
     * @param parent Parent object
     * 
     * Creates a new parameter model. The parent QObject takes ownership
     * of this model for memory management purposes.
     */
    explicit IParameterModel(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Virtual destructor
     * 
     * Ensures proper cleanup when derived classes are destroyed through
     * a pointer to this interface.
     */
    virtual ~IParameterModel() {}

    /**
     * @brief Get the parameter ID
     * @return Parameter ID
     * 
     * Returns the numeric identifier for this parameter, which corresponds
     * to a VitalSync::ParameterType value. This ID is used to identify the
     * parameter type (heart rate, blood pressure, etc.) throughout the system.
     */
    virtual int GetParameterId() const = 0;

    /**
     * @brief Get the display name for this parameter
     * @return Display name
     * 
     * Returns the human-readable name for this parameter, such as "Heart Rate"
     * or "SpO2". This name is typically used for labels in the user interface.
     */
    virtual QString GetDisplayName() const = 0;

    /**
     * @brief Get the unit for this parameter
     * @return Unit string (e.g., "bpm", "mmHg")
     * 
     * Returns the unit of measurement for this parameter, such as "bpm" for
     * heart rate or "mmHg" for blood pressure. This is used for display purposes
     * in the user interface.
     */
    virtual QString GetUnit() const = 0;

    /**
     * @brief Get the current parameter value
     * @return Current value
     * 
     * Returns the most recent value for this parameter. The value is a 
     * floating-point number representing the physiological measurement,
     * such as heart rate in beats per minute.
     */
    virtual float GetValue() const = 0;

    /**
     * @brief Get the timestamp of the latest value
     * @return Timestamp of the latest value
     * 
     * Returns the time when the current parameter value was captured.
     * This can be used to determine the age of the data and handle stale values.
     */
    virtual QDateTime GetTimestamp() const = 0;

    /**
     * @brief Get the parameter's color
     * @return Color used for displaying this parameter
     * 
     * Returns the color associated with this parameter for consistent
     * visual representation across different displays and components.
     */
    virtual QColor GetColor() const = 0;

    /**
     * @brief Set the parameter's color
     * @param color New color for this parameter
     * 
     * Sets the color to be used when displaying this parameter.
     * This allows for consistent color coding of parameters across
     * the application interface.
     */
    virtual void setColor(const QColor& color) = 0;

    /**
     * @brief Get the parameter's minimum expected value
     * @return Minimum expected value
     * 
     * Returns the minimum expected value for this parameter type.
     * This is used for scaling displays and can be different from
     * alarm limits.
     */
    virtual float GetMinValue() const = 0;

    /**
     * @brief Get the parameter's maximum expected value
     * @return Maximum expected value
     * 
     * Returns the maximum expected value for this parameter type.
     * This is used for scaling displays and can be different from
     * alarm limits.
     */
    virtual float GetMaxValue() const = 0;

    /**
     * @brief Get the current alarm state
     * @return Current alarm state of the parameter
     * 
     * Returns the current alarm state based on the parameter value
     * and configured alarm limits. This determines how the parameter
     * is visually represented in alarm conditions.
     */
    virtual AlarmState GetAlarmState() const = 0;

    /**
     * @brief Set a new parameter value
     * @param timestamp Timestamp of the new value
     * @param value New parameter value
     * 
     * Updates the model with a new parameter value and timestamp.
     * This method is typically called by data providers when new
     * parameter data is available. The model will update its internal
     * state and emit appropriate signals.
     */
    virtual void UpdateValue(qint64 timestamp, float value) = 0;

    /**
     * @brief Set the alarm limits for this parameter
     * @param lowCritical Low critical limit
     * @param lowWarning Low warning limit
     * @param highWarning High warning limit
     * @param highCritical High critical limit
     * 
     * Sets the four alarm thresholds for this parameter. When the parameter
     * value crosses these thresholds, the alarm state changes accordingly,
     * and the alarmStateChanged signal is emitted.
     */
    virtual void SetAlarmLimits(float lowCritical, float lowWarning, float highWarning, float highCritical) = 0;

    /**
     * @brief Check if this parameter is currently active/enabled
     * @return True if this parameter is active
     * 
     * Returns whether this parameter is currently active and should be
     * updated with new values. Inactive parameters may still display their
     * last value but are typically shown as disabled in the UI.
     */
    virtual bool isActive() const = 0;
    
    /**
     * @brief Set the active state of this parameter
     * @param active New active state
     * 
     * Enables or disables this parameter. When a parameter is inactive,
     * it may not receive updates or trigger alarms. This is used when a
     * particular parameter is not available from the current data source
     * or has been disabled by user configuration.
     */
    virtual void SetActive(bool active) = 0;

signals:
    /**
     * @brief Signal emitted when value changes
     * @param value New value
     * 
     * This signal is emitted whenever the parameter value is updated.
     * Views can connect to this signal to update their display when
     * new parameter data is available.
     */
    void valueChanged(float value);

    /**
     * @brief Signal emitted when alarm state changes
     * @param state New alarm state
     * 
     * This signal is emitted when the parameter's alarm state changes,
     * such as when a value crosses an alarm threshold. Views can connect
     * to this signal to update their appearance based on alarm state.
     */
    void alarmStateChanged(AlarmState state);

    /**
     * @brief Signal emitted when properties are changed
     * 
     * This signal is emitted when parameter properties such as color,
     * name, units, or display range are modified. Views can connect to
     * this signal to update their configuration based on the new properties.
     */
    void propertiesChanged();

    /**
     * @brief Signal emitted when active state changes
     * @param active New active state
     * 
     * This signal is emitted when the parameter's active state changes.
     * Views can connect to this signal to update their appearance or
     * behavior based on whether the parameter is active or inactive.
     */
    void activeStateChanged(bool active);
};

#endif // I_PARAMETER_MODEL_H 