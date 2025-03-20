/**
 * @file waveform_model.h
 * @brief Definition of the WaveformModel class
 *
 * This file contains the definition of the WaveformModel class which implements
 * the IWaveformModel interface and provides functionality for storing, managing,
 * and accessing physiological waveform data.
 * The class handles data buffering, scaling, and property management for various
 * types of physiological waveforms such as ECG, respiration, and plethysmograph.
 */
#ifndef WAVEFORM_MODEL_H
#define WAVEFORM_MODEL_H

#include "../../include/i_waveform_model.h"
#include "../../include/vital_sync_types.h"
#include <QObject>
#include <QVector>
#include <QColor>
#include <QDateTime>
#include <QMutex>
#include <QString>

/**
 * @brief Model for waveform data
 * 
 * Manages waveform data storage, scaling and other properties
 * for display and processing.
 */
class WaveformModel : public IWaveformModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param waveformType Type of waveform
     * @param parent Parent QObject
     */
    explicit WaveformModel(VitalSync::WaveformType waveformType, QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~WaveformModel() override;
    
    /**
     * @brief Get the numeric ID of this waveform
     * @return Waveform ID
     */
    int GetWaveformId() const override;
    
    /**
     * @brief Get the display name of this waveform
     * @return Display name
     */
    QString GetDisplayName() const override;
    
    /**
     * @brief Set the display name of this waveform
     * @param name New display name
     */
    void SetDisplayName(const QString& name);
    
    /**
     * @brief Get the color of this waveform
     * @return Waveform color
     */
    QColor GetColor() const override;
    
    /**
     * @brief Set the color of this waveform
     * @param color New waveform color
     */
    void setColor(const QColor& color) override;
    
    /**
     * @brief Get the minimum expected value for this waveform
     * @return Minimum value
     */
    float GetMinValue() const override;
    
    /**
     * @brief Get the maximum expected value for this waveform
     * @return Maximum value
     */
    float GetMaxValue() const override;
    
    /**
     * @brief Set the scaling range for this waveform
     * @param min Minimum value
     * @param max Maximum value
     */
    void SetScalingRange(float min, float max) override;
    
    /**
     * @brief Get the stored waveform data
     * @return Vector of waveform data points
     */
    const QVector<float>& GetData() const override;
    
    /**
     * @brief Get the maximum buffer size
     * @return Maximum number of data points stored
     */
    int GetMaxBufferSize() const override { return max_buffer_size_; }
    
    /**
     * @brief Set the maximum buffer size
     * @param size New maximum buffer size
     */
    void SetMaxBufferSize(int size) override;
    
    /**
     * @brief Add new waveform data to the buffer
     * @param timestamp Timestamp of the data
     * @param data Vector of new data points
     */
    void addWaveformData(qint64 timestamp, const QVector<float>& data) override;
    
    /**
     * @brief Get the timestamp of the last update
     * @return Timestamp as QDateTime
     */
    QDateTime GetLastUpdateTime() const override;
    
    /**
     * @brief Check if this waveform is active
     * @return True if active, false otherwise
     */
    bool isActive() const override;
    
    /**
     * @brief Set the active state of this waveform
     * @param active New active state
     */
    void SetActive(bool active) override;

    /**
     * @brief Check if this waveform is using demo data
     * @return True if using demo data, false otherwise
     */
    bool GetIsDemo() const override;

    /**
     * @brief Set whether this waveform is using demo data
     * @param isDemo New demo data state
     */
    void SetIsDemo(bool isDemo);

private:
    VitalSync::WaveformType waveform_type_;  ///< Type of waveform
    QString display_name_;                   ///< Display name
    QColor color_;                          ///< Waveform color
    float min_value_;                        ///< Minimum expected value
    float max_value_;                        ///< Maximum expected value
    int max_buffer_size_;                     ///< Maximum buffer size
    bool active_;                           ///< Active state
    bool is_demo_;                           ///< Whether using demo data
    qint64 last_update_timestamp_;            ///< Last update timestamp
    QVector<float> data_;                   ///< Waveform data buffer
    mutable QMutex mutex_;                  ///< Mutex for thread safety
    qint64 last_timestamp_;                  ///< Last timestamp
};

#endif // WAVEFORM_MODEL_H 