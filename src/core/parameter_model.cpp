/**
 * @file parameter_model.cpp
 * @brief Implementation of the ParameterModel class
 *
 * This file implements the ParameterModel class that provides functionality for
 * managing physiological parameter data. It handles value updates, alarm state
 * management based on configurable thresholds, and parameter configuration persistence.
 * The implementation uses thread-safe methods to ensure data integrity during
 * concurrent access from multiple threads.
 */
#include "parameter_model.h"
#include "../../include/config_manager.h"
#include <QDebug>

/**
 * @brief Constructs a ParameterModel for the specified parameter type
 * @param parameterType The type of physiological parameter
 * @param parent The parent QObject for memory management
 * 
 * Initializes a new parameter model with appropriate default values for the
 * specified parameter type, including default alarm limits, range, and color.
 * Also loads any saved configuration for this parameter type from the ConfigManager.
 */
ParameterModel::ParameterModel(VitalSync::ParameterType parameterType, QObject* parent)
    : IParameterModel(parent)
    , parameter_type_(parameterType)
    , value_(0.0f)
    , alarm_state_(AlarmState::Normal)
    , active_(false)
{
    // Get default range for this parameter type
    auto range = VitalSync::GetDefaultParameterRange(parameterType);
    min_value_ = range.first;
    max_value_ = range.second;
    
    // Get default alarm limits
    auto limits = VitalSync::GetDefaultAlarmLimits(parameterType);
    low_critical_ = std::get<0>(limits);
    low_warning_ = std::get<1>(limits);
    high_warning_ = std::get<2>(limits);
    high_critical_ = std::get<3>(limits);
    
    // Default color is yellow
    color_ = Qt::yellow;
    
    // Set initial timestamp to current time
    timestamp_ = QDateTime::currentDateTime();
    
    // Load configuration if available
    auto& config = ConfigManager::GetInstance();
    QVariantMap paramConfig = config.GetParameterConfig(parameterType);
    
    if (!paramConfig.isEmpty()) {
        if (paramConfig.contains("active")) {
            active_ = paramConfig["active"].toBool();
        }
        
        if (paramConfig.contains("color")) {
            color_ = paramConfig["color"].value<QColor>();
        }
        
        if (paramConfig.contains("minValue")) {
            min_value_ = paramConfig["minValue"].toFloat();
        }
        
        if (paramConfig.contains("maxValue")) {
            max_value_ = paramConfig["maxValue"].toFloat();
        }
        
        if (paramConfig.contains("lowCritical")) {
            low_critical_ = paramConfig["lowCritical"].toFloat();
        }
        
        if (paramConfig.contains("lowWarning")) {
            low_warning_ = paramConfig["lowWarning"].toFloat();
        }
        
        if (paramConfig.contains("highWarning")) {
            high_warning_ = paramConfig["highWarning"].toFloat();
        }
        
        if (paramConfig.contains("highCritical")) {
            high_critical_ = paramConfig["highCritical"].toFloat();
        }
    }
}

/**
 * @brief Destroys the ParameterModel and saves its configuration
 * 
 * Saves the current configuration (active state, color, range, alarm limits)
 * to the ConfigManager before destruction.
 */
ParameterModel::~ParameterModel()
{
    // Save configuration
    QVariantMap config;
    config["active"] = active_;
    config["color"] = color_;
    config["minValue"] = min_value_;
    config["maxValue"] = max_value_;
    config["lowCritical"] = low_critical_;
    config["lowWarning"] = low_warning_;
    config["highWarning"] = high_warning_;
    config["highCritical"] = high_critical_;
    
    ConfigManager::GetInstance().SetParameterConfig(parameter_type_, config);
}

/**
 * @brief Gets the parameter's unique identifier
 * @return Numeric ID based on the parameter type
 * 
 * Returns the numeric identifier for this parameter, which corresponds
 * to the VitalSync::ParameterType enum value.
 */
int ParameterModel::GetParameterId() const
{
    return static_cast<int>(parameter_type_);
}

/**
 * @brief Gets the parameter's human-readable display name
 * @return Display name for this parameter
 * 
 * Returns a user-friendly name for this parameter (e.g., "Heart Rate", "SpO2").
 */
QString ParameterModel::GetDisplayName() const
{
    return VitalSync::GetParameterDisplayName(parameter_type_);
}

/**
 * @brief Gets the parameter's unit of measurement
 * @return Unit string (e.g., "bpm", "mmHg", "%")
 * 
 * Returns the appropriate unit of measurement for this parameter type.
 */
QString ParameterModel::GetUnit() const
{
    return VitalSync::GetParameterUnit(parameter_type_);
}

/**
 * @brief Gets the parameter's current value
 * @return Current parameter value
 * 
 * Returns the most recently updated value for this parameter.
 */
float ParameterModel::GetValue() const
{
    QReadLocker locker(&lock_);
    return value_;
}

/**
 * @brief Gets the timestamp of the last value update
 * @return Timestamp as QDateTime
 * 
 * Returns the timestamp of when the parameter value was last updated.
 */
QDateTime ParameterModel::GetTimestamp() const
{
    QReadLocker locker(&lock_);
    return timestamp_;
}

/**
 * @brief Gets the parameter's display color
 * @return Current display color
 * 
 * Returns the color used to display this parameter in the UI.
 */
QColor ParameterModel::GetColor() const
{
    QReadLocker locker(&lock_);
    return color_;
}

/**
 * @brief Sets the parameter's display color
 * @param color New display color
 * 
 * Updates the color used to display this parameter and emits
 * the propertiesChanged signal if the color was changed.
 */
void ParameterModel::setColor(const QColor& color)
{
    {
        QWriteLocker locker(&lock_);
        if (color_ == color) {
            return;
        }
        color_ = color;
    }
    
    emit propertiesChanged();
}

/**
 * @brief Gets the parameter's minimum expected value
 * @return Minimum value in normal range
 * 
 * Returns the minimum value expected for this parameter in normal conditions.
 */
float ParameterModel::GetMinValue() const
{
    QReadLocker locker(&lock_);
    return min_value_;
}

/**
 * @brief Gets the parameter's maximum expected value
 * @return Maximum value in normal range
 * 
 * Returns the maximum value expected for this parameter in normal conditions.
 */
float ParameterModel::GetMaxValue() const
{
    QReadLocker locker(&lock_);
    return max_value_;
}

/**
 * @brief Gets the parameter's current alarm state
 * @return Current alarm state
 * 
 * Returns the current alarm state (Normal, LowWarning, LowCritical,
 * HighWarning, HighCritical) based on the parameter value and thresholds.
 */
IParameterModel::AlarmState ParameterModel::GetAlarmState() const
{
    QReadLocker locker(&lock_);
    return alarm_state_;
}

/**
 * @brief Checks if this parameter is active
 * @return True if the parameter is active
 * 
 * Returns whether this parameter is currently active and
 * processing updates.
 */
bool ParameterModel::isActive() const
{
    QReadLocker locker(&lock_);
    return active_;
}

/**
 * @brief Updates the parameter value and recalculates the alarm state
 * @param timestamp Timestamp of the new value in milliseconds since epoch
 * @param new_value The new parameter value
 * 
 * Updates the parameter with a new value, calculates the appropriate alarm state
 * based on the current alarm thresholds, and emits signals if the value or alarm
 * state has changed.
 */
void ParameterModel::UpdateValue(qint64 timestamp, float new_value)
{
    // Store old values for comparison
    float old_value = value_;
    AlarmState old_alarm_state = alarm_state_;
    
    // Check limits and update alarm state
    if (new_value > high_critical_) {
        alarm_state_ = AlarmState::HighCritical;
    } else if (new_value > high_warning_) {
        alarm_state_ = AlarmState::HighWarning;
    } else if (new_value < low_critical_) {
        alarm_state_ = AlarmState::LowCritical;
    } else if (new_value < low_warning_) {
        alarm_state_ = AlarmState::LowWarning;
    } else {
        alarm_state_ = AlarmState::Normal;
    }
    
    // Update value and timestamp
    value_ = new_value;
    timestamp_ = (timestamp > 0) ? 
        QDateTime::fromMSecsSinceEpoch(timestamp) : 
        QDateTime::currentDateTime();
    
    // Log the update with more details
    qDebug() << "ParameterModel: " << GetDisplayName() 
             << "UPDATED from" << old_value << "to" << value_ << GetUnit()
             << "at" << GetTimestamp().toString("hh:mm:ss.zzz")
             << "- Alarm state:" << static_cast<int>(alarm_state_)
             << "- Active:" << active_;
    
    // Emit signal if value or alarm state changed
    bool value_changed = (old_value != value_);
    bool alarm_changed = (old_alarm_state != alarm_state_);
    
    if (value_changed || alarm_changed) {
        qDebug() << "ParameterModel: Emitting propertiesChanged for" << GetDisplayName();
        emit propertiesChanged();
    } else {
        qDebug() << "ParameterModel: No change in value or alarm state for" << GetDisplayName();
    }
}

/**
 * @brief Sets the alarm threshold limits for the parameter
 * @param lowCritical Critical low threshold that triggers a high-priority alarm
 * @param lowWarning Warning low threshold that triggers a medium-priority alarm
 * @param highWarning Warning high threshold that triggers a medium-priority alarm
 * @param highCritical Critical high threshold that triggers a high-priority alarm
 * 
 * Sets the alarm threshold limits and recalculates the current alarm state based
 * on the new thresholds. Emits signals if the alarm state changes.
 */
void ParameterModel::SetAlarmLimits(float lowCritical, float lowWarning, float highWarning, float highCritical)
{
    AlarmState oldState;
    
    {
        QWriteLocker locker(&lock_);
        oldState = alarm_state_;
        
        low_critical_ = lowCritical;
        low_warning_ = lowWarning;
        high_warning_ = highWarning;
        high_critical_ = highCritical;
        
        UpdateAlarmState();
    }
    
    emit propertiesChanged();
    
    if (oldState != alarm_state_) {
        emit alarmStateChanged(alarm_state_);
    }
}

/**
 * @brief Sets the active state of this parameter
 * @param active New active state
 * 
 * Updates the active state of the parameter and emits the appropriate
 * signals to notify of the change. When a parameter is inactive, it
 * will not process or display updates.
 */
void ParameterModel::SetActive(bool active)
{
    {
        QWriteLocker locker(&lock_);
        if (active_ == active) {
            return;
        }
        active_ = active;
    }
    
    emit activeStateChanged(active);
    emit propertiesChanged();
}

/**
 * @brief Recalculates the alarm state based on current value and thresholds
 * 
 * Internal method that determines the appropriate alarm state
 * (Normal, LowWarning, LowCritical, HighWarning, HighCritical)
 * based on the current parameter value and the configured alarm thresholds.
 * Called after value updates or when alarm thresholds change.
 */
void ParameterModel::UpdateAlarmState()
{
    // Determine the alarm state based on the current value and limits
    AlarmState newState = AlarmState::Normal;
    
    if (value_ <= low_critical_) {
        newState = AlarmState::LowCritical;
    } else if (value_ <= low_warning_) {
        newState = AlarmState::LowWarning;
    } else if (value_ >= high_critical_) {
        newState = AlarmState::HighCritical;
    } else if (value_ >= high_warning_) {
        newState = AlarmState::HighWarning;
    }
    
    alarm_state_ = newState;
} 