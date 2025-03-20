/**
 * @file i_waveform_model.h
 * @brief Interface for waveform data models
 * 
 * This interface defines the contract that all waveform data models must implement.
 * Waveform models are responsible for storing, processing, and providing access
 * to waveform data to be displayed by UI components.
 * 
 * Waveform models handle various aspects of physiological waveforms including:
 * - Data buffering and management
 * - Scaling and normalization
 * - Metadata (display name, units)
 * - Visual properties (color)
 * - Active/inactive state management
 */

#ifndef I_WAVEFORM_MODEL_H
#define I_WAVEFORM_MODEL_H

#include <QObject>
#include <QVector>
#include <QColor>
#include <QDateTime>

/**
 * @brief Interface for waveform data models
 * 
 * This interface defines the contract for classes that provide
 * waveform data for display and processing. Waveform models manage
 * continuous physiological signals such as ECG, respiration, and other
 * time-series data. They receive raw data from data providers, maintain
 * a buffer of recent data points, and notify views when new data is available.
 * 
 * Waveform models are typically created by the DataManager and associated
 * with views to create complete waveform display components. The models
 * handle data buffering, scaling, and metadata management, allowing views
 * to focus on visualization aspects.
 */
class IWaveformModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     * 
     * Creates a new waveform model. The parent QObject takes ownership
     * of this model for memory management purposes.
     */
    explicit IWaveformModel(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Virtual destructor
     * 
     * Ensures proper cleanup when derived classes are destroyed through
     * a pointer to this interface.
     */
    virtual ~IWaveformModel() {}

    /**
     * @brief Get the numeric ID of this waveform
     * @return Waveform ID
     * 
     * Returns the numeric identifier for this waveform, which corresponds
     * to a VitalSync::WaveformType value. This ID is used to identify the
     * waveform type (ECG, respiration, etc.) throughout the system.
     */
    virtual int GetWaveformId() const = 0;
    
    /**
     * @brief Get the display name of this waveform
     * @return Display name
     * 
     * Returns the human-readable name for this waveform, such as "ECG" or
     * "Respiration". This name is typically used for labels in the user interface.
     */
    virtual QString GetDisplayName() const = 0;
    
    /**
     * @brief Get the color of this waveform
     * @return Waveform color
     * 
     * Returns the color associated with this waveform for consistent
     * visual representation across different displays and components.
     */
    virtual QColor GetColor() const = 0;
    
    /**
     * @brief Set the color of this waveform
     * @param color New waveform color
     * 
     * Sets the color to be used when displaying this waveform.
     * This allows for consistent color coding of waveforms across
     * the application interface.
     */
    virtual void setColor(const QColor& color) = 0;
    
    /**
     * @brief Get the minimum expected value for this waveform
     * @return Minimum value
     * 
     * Returns the minimum expected value for this waveform type.
     * This is used for scaling displays and represents the lower bound
     * of the normal physiological range for this signal.
     */
    virtual float GetMinValue() const = 0;
    
    /**
     * @brief Get the maximum expected value for this waveform
     * @return Maximum value
     * 
     * Returns the maximum expected value for this waveform type.
     * This is used for scaling displays and represents the upper bound
     * of the normal physiological range for this signal.
     */
    virtual float GetMaxValue() const = 0;
    
    /**
     * @brief Set the scaling range for this waveform
     * @param min Minimum value
     * @param max Maximum value
     * 
     * Configures the amplitude scaling range for this waveform.
     * This affects how the waveform is displayed vertically in views,
     * but does not affect the raw data. Typical ranges depend on the
     * waveform type (e.g., -1 to 1 mV for ECG).
     */
    virtual void SetScalingRange(float min, float max) = 0;
    
    /**
     * @brief Get the stored waveform data
     * @return Vector of waveform data points
     * 
     * Returns the current buffer of waveform data points. This is typically
     * called by views when rendering the waveform. The size of this buffer
     * is limited by the maximum buffer size.
     */
    virtual const QVector<float>& GetData() const = 0;
    
    /**
     * @brief Get the maximum buffer size
     * @return Maximum number of data points stored
     * 
     * Returns the maximum number of data points that this model will store.
     * Once this limit is reached, older data points are removed as new ones
     * are added, creating a rolling buffer.
     */
    virtual int GetMaxBufferSize() const = 0;
    
    /**
     * @brief Set the maximum buffer size
     * @param size New maximum buffer size
     * 
     * Sets the maximum number of data points that this model will store.
     * Larger buffer sizes allow for displaying more history but consume
     * more memory. The buffer operates as a FIFO queue, discarding the
     * oldest data points when full.
     */
    virtual void SetMaxBufferSize(int size) = 0;
    
    /**
     * @brief Add new waveform data to the buffer
     * @param timestamp Timestamp in milliseconds when the data was captured
     * @param data Vector of new data points to add to the buffer
     * 
     * Adds new waveform data to the buffer. This method is typically called by
     * data providers when new waveform samples are available. If adding this data
     * would exceed the maximum buffer size, the oldest data is removed to make room.
     * The model will emit the dataUpdated signal after processing the new data.
     */
    virtual void addWaveformData(qint64 timestamp, const QVector<float>& data) = 0;
    
    /**
     * @brief Get the timestamp of the last update
     * @return Timestamp as QDateTime
     * 
     * Returns the time when the most recent data was added to this model.
     * This can be used to determine the age of the data and handle stale waveforms.
     */
    virtual QDateTime GetLastUpdateTime() const = 0;
    
    /**
     * @brief Check if this waveform is active
     * @return True if active, false otherwise
     * 
     * Returns whether this waveform is currently active and should be
     * updated with new values. Inactive waveforms may still display their
     * existing data but are typically shown as disabled in the UI.
     */
    virtual bool isActive() const = 0;
    
    /**
     * @brief Set the active state of this waveform
     * @param active New active state
     * 
     * Enables or disables this waveform. When a waveform is inactive,
     * it may not receive updates. This is used when a particular waveform
     * is not available from the current data source or has been disabled
     * by user configuration.
     */
    virtual void SetActive(bool active) = 0;

    /**
     * @brief Check if this waveform is using demo data
     * @return True if using demo data, false otherwise
     * 
     * Returns whether this waveform is currently using simulated demo data
     * rather than data from a real physiological sensor. This can be used
     * by views to indicate that the data is not from a real patient.
     */
    virtual bool GetIsDemo() const = 0;

signals:
    /**
     * @brief Signal emitted when new data is available
     * 
     * This signal is emitted whenever new waveform data is added to the model.
     * Views can connect to this signal to update their display when new
     * waveform samples are available.
     */
    void dataUpdated();
    
    /**
     * @brief Signal emitted when properties change
     * 
     * This signal is emitted when waveform properties such as color,
     * name, or scaling range are modified. Views can connect to this
     * signal to update their configuration based on the new properties.
     */
    void propertiesChanged();
    
    /**
     * @brief Signal emitted when active state changes
     * @param active New active state
     * 
     * This signal is emitted when the waveform's active state changes.
     * Views can connect to this signal to update their appearance or
     * behavior based on whether the waveform is active or inactive.
     */
    void activeStateChanged(bool active);
};

#endif // I_WAVEFORM_MODEL_H 