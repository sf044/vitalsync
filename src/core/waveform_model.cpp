/**
 * @file waveform_model.cpp
 * @brief Implementation of the WaveformModel class
 *
 * This file implements the WaveformModel class responsible for storing,
 * managing, and providing access to physiological waveform data. The implementation
 * handles data buffering, scaling, color management, and other properties
 * required for proper display and processing of different types of physiological
 * waveforms (ECG, respiration, plethysmograph, etc.).
 */
#include "waveform_model.h"
#include "../../include/config_manager.h"
#include "../../include/vital_sync_types.h"
#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>
#include <cmath>

/**
 * @brief Constants used in the WaveformModel implementation
 */
namespace {
    const int DEFAULT_BUFFER_SIZE = 1000;  ///< Default number of samples to store in the waveform buffer
}

/**
 * @brief Constructs a WaveformModel for the specified waveform type
 * @param waveformType The type of physiological waveform
 * @param parent The parent QObject for memory management
 * 
 * Initializes a new waveform model with appropriate default values for the
 * specified waveform type, including color, scaling range, and buffer size.
 * Also loads any saved configuration for this waveform type from the ConfigManager.
 */
WaveformModel::WaveformModel(VitalSync::WaveformType waveformType, QObject* parent)
    : IWaveformModel(parent)
    , waveform_type_(waveformType)
    , display_name_(VitalSync::GetWaveformDisplayName(waveformType))
    , max_buffer_size_(DEFAULT_BUFFER_SIZE)
    , active_(true)
    , is_demo_(true)  // Default to using demo data
    , last_update_timestamp_(0)
    , last_timestamp_(0)
{
    // Set default min/max based on waveform type
    auto [min, max] = VitalSync::GetDefaultWaveformRange(waveformType);
    min_value_ = min;
    max_value_ = max;
    
    // Set default color based on waveform type
    switch (waveformType) {
        case VitalSync::WaveformType::ECG_I:
        case VitalSync::WaveformType::ECG_II:
        case VitalSync::WaveformType::ECG_III:
            color_ = QColor(0, 255, 0);  // Bright Green for ECG
            break;
        case VitalSync::WaveformType::RESP:
            color_ = QColor(255, 255, 0);  // Bright Yellow for respiration
            break;
        case VitalSync::WaveformType::PLETH:
            color_ = QColor(0, 255, 255);  // Bright Cyan for pleth
            break;
        case VitalSync::WaveformType::ABP:
            color_ = QColor(255, 0, 0);  // Bright Red for arterial blood pressure
            break;
        case VitalSync::WaveformType::CAPNO:
            color_ = QColor(255, 255, 255);  // White for capnography
            break;
        default:
            color_ = QColor(255, 255, 255);  // White for default
            break;
    }
    
    // Reserve space for data buffer
    data_.reserve(max_buffer_size_);

    // Initialize with a sinusoidal pattern rather than flatline to ensure visibility
    data_.resize(max_buffer_size_);
    for (int i = 0; i < max_buffer_size_; ++i) {
        // Create a simple sine wave pattern for initial display
        double phase = (double)i / max_buffer_size_ * 2 * M_PI;
        data_[i] = 0.5f * sinf(phase); // Small amplitude sine wave
    }
    
    // Load configuration if available
    auto& config = ConfigManager::GetInstance();
    QVariantMap waveformConfig = config.GetWaveformConfig(waveformType);
    
    if (!waveformConfig.isEmpty()) {
        if (waveformConfig.contains("active")) {
            active_ = waveformConfig["active"].toBool();
        }
        
        if (waveformConfig.contains("color")) {
            color_ = waveformConfig["color"].value<QColor>();
        }
        
        if (waveformConfig.contains("minValue")) {
            min_value_ = waveformConfig["minValue"].toFloat();
        }
        
        if (waveformConfig.contains("maxValue")) {
            max_value_ = waveformConfig["maxValue"].toFloat();
        }
        
        if (waveformConfig.contains("bufferSize")) {
            max_buffer_size_ = waveformConfig["bufferSize"].toInt();
            data_.resize(max_buffer_size_);
        }
    }
}

/**
 * @brief Destroys the WaveformModel and saves its configuration
 * 
 * Saves the current configuration (active state, color, scaling range, buffer size)
 * to the ConfigManager before destruction.
 */
WaveformModel::~WaveformModel()
{
    // Save configuration
    QVariantMap config;
    config["active"] = active_;
    config["color"] = color_;
    config["minValue"] = min_value_;
    config["maxValue"] = max_value_;
    config["bufferSize"] = max_buffer_size_;
    
    ConfigManager::GetInstance().SetWaveformConfig(waveform_type_, config);
}

/**
 * @brief Gets the numeric ID of this waveform
 * @return Waveform ID
 * 
 * Returns the numeric ID of this waveform, which corresponds
 * to the VitalSync::WaveformType enum value.
 */
int WaveformModel::GetWaveformId() const
{
    return static_cast<int>(waveform_type_);
}

/**
 * @brief Gets the display name of this waveform
 * @return Display name
 * 
 * Returns the human-readable display name for this waveform.
 */
QString WaveformModel::GetDisplayName() const
{
    return display_name_;
}

/**
 * @brief Sets the display name of this waveform
 * @param name New display name
 * 
 * Updates the display name of the waveform and emits the
 * propertiesChanged signal if the name was changed.
 */
void WaveformModel::SetDisplayName(const QString& name)
{
    if (display_name_ != name) {
        display_name_ = name;
        emit propertiesChanged();
    }
}

/**
 * @brief Gets the color of this waveform
 * @return Waveform color
 * 
 * Returns the color used to display this waveform.
 */
QColor WaveformModel::GetColor() const
{
    QMutexLocker locker(&mutex_);
    return color_;
}

/**
 * @brief Sets the color of this waveform
 * @param color New waveform color
 * 
 * Updates the color used to display this waveform and emits
 * the propertiesChanged signal if the color was changed.
 */
void WaveformModel::setColor(const QColor& color)
{
    {
        QMutexLocker locker(&mutex_);
        if (color_ == color) {
            return;
        }
        color_ = color;
    }
    
    emit propertiesChanged();
}

/**
 * @brief Gets the minimum expected value for this waveform
 * @return Minimum value
 * 
 * Returns the minimum expected value for scaling this waveform.
 */
float WaveformModel::GetMinValue() const
{
    QMutexLocker locker(&mutex_);
    return min_value_;
}

/**
 * @brief Gets the maximum expected value for this waveform
 * @return Maximum value
 * 
 * Returns the maximum expected value for scaling this waveform.
 */
float WaveformModel::GetMaxValue() const
{
    QMutexLocker locker(&mutex_);
    return max_value_;
}

/**
 * @brief Gets the stored waveform data
 * @return Vector of waveform data points
 * 
 * Returns a reference to the vector containing the waveform
 * data points.
 */
const QVector<float>& WaveformModel::GetData() const
{
    QMutexLocker locker(&mutex_);
    return data_;
}

/**
 * @brief Sets the scaling range for this waveform
 * @param min Minimum value
 * @param max Maximum value
 * 
 * Updates the minimum and maximum scaling values for the waveform
 * and emits the propertiesChanged signal to notify of the change.
 */
void WaveformModel::SetScalingRange(float min, float max)
{
    {
        QMutexLocker locker(&mutex_);
        if (min_value_ == min && max_value_ == max) {
            return;
        }
        min_value_ = min;
        max_value_ = max;
    }
    
    emit propertiesChanged();
}

/**
 * @brief Sets the maximum buffer size
 * @param size New maximum buffer size
 * 
 * Updates the maximum number of data points stored in the buffer
 * and resizes the buffer accordingly. Emits the propertiesChanged
 * signal if the size was changed.
 */
void WaveformModel::SetMaxBufferSize(int size)
{
    {
        QMutexLocker locker(&mutex_);
        if (max_buffer_size_ == size) {
            return;
        }
        
        max_buffer_size_ = size;
        data_.resize(size);
    }
    
    emit propertiesChanged();
}

/**
 * @brief Adds new waveform data to the buffer
 * @param timestamp Timestamp of the data
 * @param data Vector of new data points
 * 
 * Adds new waveform data points to the circular buffer, handling
 * timestamp validation and buffer management. When data is added,
 * the dataUpdated signal is emitted to notify views.
 */
void WaveformModel::addWaveformData(qint64 timestamp, const QVector<float>& data)
{
    if (data.isEmpty() || !active_) {
        return;
    }
    
    {
        QMutexLocker locker(&mutex_);
        
        // Ensure timestamp is increasing (keeping this check for data integrity)
        if (timestamp <= last_timestamp_ && last_timestamp_ != 0) {
            qWarning() << "Received out-of-order waveform data for " << GetDisplayName()
                       << ". Expected timestamp > " << last_timestamp_
                       << ", got " << timestamp << ". Ignoring.";
            return;
        }
        
        last_timestamp_ = timestamp;
        
        // Add new data to the buffer, shifting out old data if necessary
        int numToAdd = data.size();
        int bufferSize = data_.size();
        
        // Print actual data for debugging
        qDebug() << "WaveformModel::addWaveformData - ID:" << GetWaveformId() 
                 << "Type:" << static_cast<int>(waveform_type_)
                 << "Data size:" << data.size() 
                 << "First 3 values:" << (data.size() > 0 ? data[0] : 0.0)
                 << (data.size() > 1 ? data[1] : 0.0)
                 << (data.size() > 2 ? data[2] : 0.0);
        
        if (numToAdd >= bufferSize) {
            // If new data is larger than buffer, just take the most recent points
            data_ = data.mid(data.size() - bufferSize);
        } else {
            // Shift existing data and add new data
            for (int i = 0; i < bufferSize - numToAdd; ++i) {
                data_[i] = data_[i + numToAdd];
            }
            
            for (int i = 0; i < numToAdd; ++i) {
                data_[bufferSize - numToAdd + i] = data[i];
            }
        }
    }
    
    emit dataUpdated();
}

/**
 * @brief Checks if this waveform is active
 * @return True if active, false otherwise
 * 
 * Returns whether this waveform is currently active and
 * being displayed.
 */
bool WaveformModel::isActive() const
{
    QMutexLocker locker(&mutex_);
    return active_;
}

/**
 * @brief Sets the active state of this waveform
 * @param active New active state
 * 
 * Updates the active state of the waveform and emits the appropriate
 * signals to notify of the change.
 */
void WaveformModel::SetActive(bool active)
{
    {
        QMutexLocker locker(&mutex_);
        if (active_ == active) {
            return;
        }
        active_ = active;
    }
    
    emit activeStateChanged(active);
    emit propertiesChanged();
}

/**
 * @brief Gets the timestamp of the last update
 * @return Timestamp as QDateTime
 * 
 * Returns the timestamp of the last data update as a QDateTime.
 */
QDateTime WaveformModel::GetLastUpdateTime() const
{
    QMutexLocker locker(&mutex_);
    return QDateTime::fromMSecsSinceEpoch(last_timestamp_);
}

/**
 * @brief Checks if this waveform is using demo data
 * @return True if using demo data, false otherwise
 * 
 * Returns whether this waveform is using simulated demo data
 * rather than real patient data.
 */
bool WaveformModel::GetIsDemo() const
{
    QMutexLocker locker(&mutex_);
    return is_demo_;
}

/**
 * @brief Sets whether this waveform is using demo data
 * @param isDemo New demo data state
 * 
 * Updates the flag indicating whether this waveform is using
 * simulated demo data and emits the propertiesChanged signal
 * if the state was changed.
 */
void WaveformModel::SetIsDemo(bool isDemo)
{
    QMutexLocker locker(&mutex_);
    
    if (is_demo_ != isDemo) {
        is_demo_ = isDemo;
        emit propertiesChanged();
    }
} 