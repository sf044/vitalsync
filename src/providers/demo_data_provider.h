/**
 * @file demo_data_provider.h
 * @brief Definition of the DemoDataProvider class
 *
 * This file contains the definition of the DemoDataProvider class which 
 * generates simulated physiological data.
 * It provides realistic waveforms and parameter values for ECG, respiration,
 * blood pressure, SpO2, temperature, and more without requiring an actual
 * connection to physical monitoring equipment.
 */
#ifndef DEMO_DATA_PROVIDER_H
#define DEMO_DATA_PROVIDER_H

#include "../../include/i_data_provider.h"
#include "../../include/vital_sync_types.h"
#include <QTimer>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QVariantMap>
#include <QMutex>
#include <unordered_map>
#include <functional>
#include <QMap>
#include <QObject>
#include <QString>
#include <QDateTime>

/**
 * @brief Provider for simulated patient data
 *
 * This provider generates simulated waveform and parameter data
 * for testing and demonstration purposes.
 */
class DemoDataProvider : public IDataProvider {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit DemoDataProvider(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~DemoDataProvider() override;

    /**
     * @brief Start generating data
     * @return True if successful
     */
    bool start() override;

    /**
     * @brief Stop generating data
     */
    void stop() override;

    /**
     * @brief Get the current connection status
     * @return Current connection status
     */
    VitalSync::ConnectionStatus GetConnectionStatus() const override;

    /**
     * @brief Get provider name
     * @return Name of the provider
     */
    std::string GetName() const override;

    /**
     * @brief Check if this provider is currently in use
     * @return True if this provider is active
     */
    bool isActive() const override;

    /**
     * @brief Configure the provider with connection parameters
     * @param params Provider-specific connection parameters
     * @return True if configuration was successful
     */
    bool configure(const QVariantMap& params) override;

private slots:
    /**
     * @brief Generate and emit waveform data
     */
    void generateWaveformData();

    /**
     * @brief Generate and emit parameter data
     */
    void generateParameterData();

private:
    /**
     * @brief Initialize the provider
     */
    void initialize();

    /**
     * @brief Generate ECG waveform
     * @param time Current time in seconds
     * @param points Number of points to generate
     * @return Vector of generated data points
     */
    QVector<float> generateECG(double time, int points);

    /**
     * @brief Generate respiration waveform
     * @param time Current time in seconds
     * @param points Number of points to generate
     * @return Vector of generated data points
     */
    QVector<float> generateRespiration(double time, int points);

    /**
     * @brief Generate SPO2 plethysmograph waveform
     * @param time Current time in seconds
     * @param points Number of points to generate
     * @return Vector of generated data points
     */
    QVector<float> generatePlethysmograph(double time, int points);

    /**
     * @brief Generate arterial blood pressure waveform
     * @param time Current time in seconds
     * @param points Number of points to generate
     * @return Vector of generated data points
     */
    QVector<float> generateArterialPressure(double time, int points);

    /**
     * @brief Generate CO2 capnograph waveform
     * @param time Current time in seconds
     * @param points Number of points to generate
     * @return Vector of generated data points
     */
    QVector<float> generateCapnograph(double time, int points);

    /**
     * @brief Add normal variation to a parameter
     * @param baseValue Base parameter value
     * @param variationPct Percentage variation (0-1)
     * @return Parameter with normal variation
     */
    double addVariation(double baseValue, double variationPct);

    /**
     * @brief Occasionally generate values outside normal range for a parameter
     * @param baseValue Base parameter value
     * @param minValue Normal minimum value for the parameter
     * @param maxValue Normal maximum value for the parameter
     * @param cycleCount Current cycle counter (for timing)
     * @return Possibly modified value that might exceed normal limits
     */
    double generateExtremeValue(double baseValue, double minValue, double maxValue, int cycleCount);

    /**
     * @brief Frequently generate values outside normal range for critical parameters
     * @param baseValue Base parameter value
     * @param minValue Normal minimum value for the parameter
     * @param maxValue Normal maximum value for the parameter
     * @param cycleCount Current cycle counter (for timing)
     * @return Possibly modified value that often exceeds normal limits with greater deviation
     */
    double generateCriticalExtremeValue(double baseValue, double minValue, double maxValue, int cycleCount);

private:
    // Connection status
    VitalSync::ConnectionStatus status_;
    
    // Active flag
    bool active_;
    
    // Timer for generating waveform data
    QTimer waveform_timer_;
    
    // Timer for generating parameter data
    QTimer parameter_timer_;
    
    // Elapsed timer for tracking time
    QElapsedTimer elapsed_timer_;
    
    // Cycle counter for periodic variations
    int cycleCounter = 0;
    
    // Configuration parameters
    int waveform_update_interval_ms_;      // Update interval for waveforms
    int parameter_update_interval_ms_;     // Update interval for parameters
    int samples_per_update_;              // Number of samples per waveform update
    double heart_rate_;                  // Current heart rate
    double respiration_rate_;            // Current respiration rate
    double spo2_;                       // Current SpO2 percentage
    double systolic_bp_;                 // Current systolic BP
    double diastolic_bp_;                // Current diastolic BP
    double mean_bp_;                     // Current mean BP
    double etco2_;                      // Current ETCO2
    double temperature_;                // Current temperature (TEMP1)
    double temperature2_;               // Current second temperature (TEMP2)
    double ibp1_systolic_;              // Current IBP1 systolic
    double ibp1_diastolic_;             // Current IBP1 diastolic
    double ibp1_mean_;                  // Current IBP1 mean
    double ibp2_systolic_;              // Current IBP2 systolic (CVP)
    double ibp2_diastolic_;             // Current IBP2 diastolic (CVP)
    double ibp2_mean_;                  // Current IBP2 mean (CVP)
    
    // Waveform generation parameters
    double amplitude_;                  // Amplitude scaling factor
    double frequency_;                  // Frequency scaling factor
    double noise_;                      // Noise level (0-1)
    bool artifacts_;                    // Whether to include artifacts
    
    // Random number generator
    QRandomGenerator random_;
    
    // Thread safety
    mutable QMutex mutex_;
    
    // Map of waveform generator functions
    std::unordered_map<int, std::function<QVector<float>(double, int)>> waveform_generators_;
};

#endif // DEMO_DATA_PROVIDER_H 