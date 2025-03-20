/**
 * @file parameter_model.h
 * @brief Definition of the ParameterModel class
 *
 * This file contains the definition of the ParameterModel class which implements
 * the IParameterModel interface. The ParameterModel class provides functionality
 * for managing physiological parameter data (heart rate, blood pressure, etc.),
 * handling alarm states based on threshold values, and managing parameter properties
 * such as color, display name, and units.
 */
#ifndef PARAMETER_MODEL_H
#define PARAMETER_MODEL_H

#include "../../include/i_parameter_model.h"
#include "../../include/vital_sync_types.h"
#include <QReadWriteLock>
#include <QDateTime>

/**
 * @brief Standard implementation of IParameterModel
 * 
 * This class provides a standard implementation of the IParameterModel interface
 * that is suitable for most types of parameters. It handles value storage, range
 * checking, alarm limit management, and timestamp tracking.
 */
class ParameterModel : public IParameterModel {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parameterType The type of parameter this model represents
     * @param parent Parent QObject
     */
    explicit ParameterModel(VitalSync::ParameterType parameterType, QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ParameterModel() override;

    /**
     * @brief Get the parameter's identifier
     * @return Unique identifier for this parameter
     */
    int GetParameterId() const override;

    /**
     * @brief Get the parameter's display name
     * @return Human-readable name for this parameter
     */
    QString GetDisplayName() const override;

    /**
     * @brief Get the parameter's unit of measure
     * @return Unit of measure for this parameter
     */
    QString GetUnit() const override;

    /**
     * @brief Get the parameter's current value
     * @return Current value of the parameter
     */
    float GetValue() const override;

    /**
     * @brief Get the timestamp of the latest value
     * @return Timestamp of the latest value
     */
    QDateTime GetTimestamp() const override;

    /**
     * @brief Get the parameter's color
     * @return Color for this parameter
     */
    QColor GetColor() const override;

    /**
     * @brief Set the color for this parameter
     * @param color New color
     */
    void setColor(const QColor& color) override;

    /**
     * @brief Get the parameter's minimum value
     * @return Minimum value for this parameter
     */
    float GetMinValue() const override;

    /**
     * @brief Get the parameter's maximum value
     * @return Maximum value for this parameter
     */
    float GetMaxValue() const override;

    /**
     * @brief Get the current alarm state
     * @return Current alarm state
     */
    AlarmState GetAlarmState() const override;

    /**
     * @brief Update the parameter value
     * @param timestamp Timestamp of the new value
     * @param value New value
     */
    void UpdateValue(qint64 timestamp, float value) override;

    /**
     * @brief Set alarm limits for this parameter
     * @param lowCritical Critical low threshold
     * @param lowWarning Warning low threshold
     * @param highWarning Warning high threshold
     * @param highCritical Critical high threshold
     */
    void SetAlarmLimits(float lowCritical, float lowWarning, float highWarning, float highCritical) override;

    /**
     * @brief Check if this parameter is active
     * @return True if the parameter is active
     */
    bool isActive() const override;

    /**
     * @brief Set the active state
     * @param active New active state
     */
    void SetActive(bool active) override;

signals:
    /**
     * @brief Signal emitted when the parameter value changes
     * @param value The new parameter value
     * 
     * This signal is emitted whenever the parameter value is updated through
     * the UpdateValue method, allowing UI components to react to value changes.
     */
    void onValueChanged(float value);

    /**
     * @brief Signal emitted when the alarm state changes
     * @param state The new alarm state
     * 
     * This signal is emitted whenever the parameter's alarm state changes,
     * allowing UI components to update visual indicators accordingly.
     */
    void onAlarmStateChanged(AlarmState state);

    /**
     * @brief Signal emitted when any parameter properties change
     * 
     * This signal is emitted when properties such as color, alarm limits,
     * or active state change, allowing UI components to refresh their display.
     */
    void onPropertiesChanged();

    /**
     * @brief Signal emitted when the active state changes
     * @param active The new active state
     * 
     * This signal is emitted when the parameter's active state changes,
     * allowing UI components to show or hide the parameter as appropriate.
     */
    void onActiveStateChanged(bool active);

private:
    /**
     * @brief Updates the alarm state based on current value and thresholds
     * 
     * Evaluates the current parameter value against the defined alarm thresholds
     * and updates the alarm state accordingly. Emits the onAlarmStateChanged signal
     * if the state changes as a result of this evaluation.
     */
    void UpdateAlarmState();

private:
    VitalSync::ParameterType parameter_type_;  ///< Type of physiological parameter this model represents
    float value_;                             ///< Current parameter value
    QDateTime timestamp_;                     ///< Timestamp of the most recent value update
    QColor color_;                            ///< Display color for this parameter
    float min_value_;                         ///< Minimum expected value in normal range
    float max_value_;                         ///< Maximum expected value in normal range
    AlarmState alarm_state_;                  ///< Current alarm state based on thresholds
    float low_critical_;                      ///< Low critical alarm threshold
    float low_warning_;                       ///< Low warning alarm threshold
    float high_warning_;                      ///< High warning alarm threshold
    float high_critical_;                     ///< High critical alarm threshold
    bool active_;                             ///< Whether this parameter is active and being monitored
    
    mutable QReadWriteLock lock_;             ///< Thread safety lock for concurrent read/write access
};

#endif // PARAMETER_MODEL_H 