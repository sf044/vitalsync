/**
 * @file vital_sync_types.h
 * @brief Common types, constants, and enumerations
 * 
 * This file defines common data types, constants, and enumerations that are used
 * These include waveform and parameter IDs, error codes, and other shared constants.
 * 
 * The VitalSync namespace contains:
 * - Enumerations for waveform types and parameter types
 * - Connection status and error code definitions
 * - Utility functions for getting display names and units
 * - Functions that define default ranges, alarm limits, and other constants
 *   for different physiological signals
 */

#ifndef VITAL_SYNC_TYPES_H
#define VITAL_SYNC_TYPES_H

#include <QString>
#include <QColor>
#include <tuple>

namespace VitalSync {

/**
 * @brief Types of waveforms supported by the system
 * 
 * Defines the different types of continuous physiological waveforms
 * that can be displayed and processed.
 * Each waveform type represents a different physiological signal
 * with unique characteristics and scaling requirements.
 */
enum class WaveformType {
    ECG_I = 0,      // ECG Lead I
    ECG_II = 1,     // ECG Lead II
    ECG_III = 2,    // ECG Lead III
    RESP = 3,       // Respiration
    PLETH = 4,      // Plethysmograph
    ABP = 5,        // Arterial Blood Pressure
    CVP = 6,        // Central Venous Pressure
    CAPNO = 7,      // Capnograph
    EEG = 8         // Electroencephalogram
};

/**
 * @brief Types of parameters supported by the system
 * 
 * Defines the different types of discrete physiological parameters
 * that can be displayed and processed.
 * Parameters are typically derived from waveforms or provided directly
 * by monitoring devices. Each parameter has its own units, normal ranges,
 * and alarm limits.
 */
enum class ParameterType {
    HR = 0,       // Heart Rate
    RR = 1,       // Respiratory Rate
    SPO2 = 2,     // Oxygen Saturation
    NIBP_SYS = 3, // Non-Invasive Blood Pressure (Systolic)
    NIBP_DIA = 4, // Non-Invasive Blood Pressure (Diastolic)
    NIBP_MAP = 5, // Non-Invasive Blood Pressure (Mean)
    TEMP1 = 6,    // Temperature 1
    TEMP2 = 7,    // Temperature 2
    ETCO2 = 8,    // End-Tidal CO2
    IBP1_SYS = 9, // Invasive Blood Pressure 1 (Systolic)
    IBP1_DIA = 10,// Invasive Blood Pressure 1 (Diastolic)
    IBP1_MAP = 11,// Invasive Blood Pressure 1 (Mean)
    IBP2_SYS = 12,// Invasive Blood Pressure 2 (Systolic)
    IBP2_DIA = 13,// Invasive Blood Pressure 2 (Diastolic)
    IBP2_MAP = 14 // Invasive Blood Pressure 2 (Mean)
};

/**
 * @brief Connection status for data providers
 * 
 * Defines the possible states of connection for data providers.
 * This is used to track the current status of data sources and
 * provide appropriate feedback to users in the UI.
 */
enum class ConnectionStatus {
    Disconnected,  ///< Provider is disconnected
    Connecting,    ///< Provider is trying to connect
    Connected,     ///< Provider is connected and ready
    Error          ///< Provider encountered an error
};

/**
 * @brief Error codes for the system
 * 
 * Defines standardized error codes for different types of errors
 * that can occur in the system. These codes are used in error reporting
 * and logging to provide consistent error handling throughout the application.
 */
enum class ErrorCode {
    NoError = 0,
    ConnectionError = 100,
    ConfigurationError = 200,
    DataError = 300,
    HardwareError = 400,
    CriticalError = 500,
    UnknownError = 999
};

/**
 * @brief Get display name for a waveform type
 * @param type Waveform type
 * @return Display name
 * 
 * Returns a human-readable name for the specified waveform type.
 * These names are used for display in the user interface, such as
 * in waveform labels and configuration menus.
 */
inline QString GetWaveformDisplayName(WaveformType type) {
    switch (type) {
        case WaveformType::ECG_I:
            return "ECG I";
        case WaveformType::ECG_II:
            return "ECG II";
        case WaveformType::ECG_III:
            return "ECG III";
        case WaveformType::RESP:
            return "Resp";
        case WaveformType::PLETH:
            return "SpO2";
        case WaveformType::ABP:
            return "ABP";
        case WaveformType::CVP:
            return "CVP";
        case WaveformType::CAPNO:
            return "ETCO2";
        case WaveformType::EEG:
            return "EEG";
        default:
            return "Unknown";
    }
}

/**
 * @brief Get display name for a parameter type
 * @param type Parameter type
 * @return Display name
 * 
 * Returns a human-readable name for the specified parameter type.
 * These names are used for display in the user interface, such as
 * in parameter labels and configuration menus. The names are typically
 * abbreviated forms that are standard in medical displays.
 */
inline QString GetParameterDisplayName(ParameterType type) {
    switch (type) {
        case ParameterType::HR:
            return "HR";
        case ParameterType::RR:
            return "RR";
        case ParameterType::SPO2:
            return "SpO2";
        case ParameterType::NIBP_SYS:
            return "NIBP-S";
        case ParameterType::NIBP_DIA:
            return "NIBP-D";
        case ParameterType::NIBP_MAP:
            return "NIBP-M";
        case ParameterType::TEMP1:
            return "Temp";
        case ParameterType::TEMP2:
            return "Temp 2";
        case ParameterType::ETCO2:
            return "ETCO2";
        case ParameterType::IBP1_SYS:
            return "ABP-S";
        case ParameterType::IBP1_DIA:
            return "ABP-D";
        case ParameterType::IBP1_MAP:
            return "ABP-M";
        case ParameterType::IBP2_SYS:
            return "CVP-S";
        case ParameterType::IBP2_DIA:
            return "CVP-D";
        case ParameterType::IBP2_MAP:
            return "CVP-M";
        default:
            return "Unknown";
    }
}

/**
 * @brief Get unit for a parameter type
 * @param type Parameter type
 * @return Unit string
 * 
 * Returns the unit of measurement for the specified parameter type.
 * These units are used in the user interface to indicate the scale
 * of parameter values. For example, heart rate is measured in beats
 * per minute (bpm) and temperature in degrees Celsius (°C).
 */
inline QString GetParameterUnit(ParameterType type) {
    switch (type) {
        case ParameterType::HR:
            return "bpm";
        case ParameterType::RR:
            return "br/min";
        case ParameterType::SPO2:
            return "%";
        case ParameterType::NIBP_SYS:
        case ParameterType::NIBP_DIA:
        case ParameterType::NIBP_MAP:
        case ParameterType::IBP1_SYS:
        case ParameterType::IBP1_DIA:
        case ParameterType::IBP1_MAP:
        case ParameterType::IBP2_SYS:
        case ParameterType::IBP2_DIA:
        case ParameterType::IBP2_MAP:
            return "mmHg";
        case ParameterType::TEMP1:
        case ParameterType::TEMP2:
            return "°C";
        case ParameterType::ETCO2:
            return "mmHg";
        default:
            return "";
    }
}

/**
 * @brief Get default amplitude range for a waveform type
 * @param type Waveform type
 * @return Pair of (min, max) values
 * 
 * Returns the default amplitude scaling range for the specified waveform type.
 * These ranges define the expected minimum and maximum values for each
 * waveform type and are used for initial scaling when displaying waveforms.
 * The values represent the typical physiological range for each signal type.
 */
inline std::pair<float, float> GetDefaultWaveformRange(WaveformType type) {
    switch (type) {
        case WaveformType::ECG_I:
        case WaveformType::ECG_II:
        case WaveformType::ECG_III:
            return {-1.5f, 1.5f}; // mV
        case WaveformType::RESP:
            return {-1.0f, 1.0f}; // Arbitrary units
        case WaveformType::PLETH:
            return {0.0f, 1.0f};  // Normalized 0-1
        case WaveformType::ABP:
        case WaveformType::CVP:
            return {0.0f, 2.0f};  // Normalized 0-2 (to handle pressure values)
        case WaveformType::CAPNO:
            return {0.0f, 1.0f};  // Normalized 0-1
        case WaveformType::EEG:
            return {-50.0f, 50.0f}; // μV
        default:
            return {-1.0f, 1.0f};  // Default range
    }
}

/**
 * @brief Get default range for a parameter type
 * @param type Parameter type
 * @return Pair of (min, max) values
 * 
 * Returns the default display range for the specified parameter type.
 * These ranges define the expected minimum and maximum values for each
 * parameter type and are used for scaling parameter displays and trends.
 * The values represent the typical physiological range for each parameter.
 */
inline std::pair<float, float> GetDefaultParameterRange(ParameterType type) {
    switch (type) {
        case ParameterType::HR:
            return {30.0f, 240.0f}; // bpm
        case ParameterType::RR:
            return {4.0f, 40.0f};   // breaths/min
        case ParameterType::SPO2:
            return {70.0f, 100.0f}; // %
        case ParameterType::NIBP_SYS:
        case ParameterType::IBP1_SYS:
        case ParameterType::IBP2_SYS:
            return {60.0f, 240.0f}; // mmHg
        case ParameterType::NIBP_DIA:
        case ParameterType::IBP1_DIA:
        case ParameterType::IBP2_DIA:
            return {30.0f, 140.0f}; // mmHg
        case ParameterType::NIBP_MAP:
        case ParameterType::IBP1_MAP:
        case ParameterType::IBP2_MAP:
            return {40.0f, 160.0f}; // mmHg
        case ParameterType::TEMP1:
        case ParameterType::TEMP2:
            return {30.0f, 42.0f};  // °C
        case ParameterType::ETCO2:
            return {0.0f, 100.0f};  // mmHg
        default:
            return {0.0f, 100.0f};  // Default range
    }
}

/**
 * @brief Get default alarm limits for a parameter type
 * @param type Parameter type
 * @return Tuple of (low critical, low warning, high warning, high critical) limits
 * 
 * Returns the default alarm limits for the specified parameter type.
 * These limits define the thresholds at which parameters are considered
 * to be in warning or critical alarm states. The values are based on 
 * typical clinical guidelines for adult patients but can be customized
 * through the application's configuration.
 */
inline std::tuple<float, float, float, float> GetDefaultAlarmLimits(ParameterType type) {
    switch (type) {
        case ParameterType::HR:
            return {40.0f, 50.0f, 120.0f, 150.0f}; // bpm
        case ParameterType::RR:
            return {6.0f, 8.0f, 25.0f, 30.0f};     // breaths/min
        case ParameterType::SPO2:
            return {85.0f, 90.0f, 100.0f, 100.0f}; // %
        case ParameterType::NIBP_SYS:
        case ParameterType::IBP1_SYS:
            return {80.0f, 90.0f, 160.0f, 180.0f}; // mmHg
        case ParameterType::NIBP_DIA:
        case ParameterType::IBP1_DIA:
            return {40.0f, 50.0f, 90.0f, 110.0f};  // mmHg
        case ParameterType::NIBP_MAP:
        case ParameterType::IBP1_MAP:
            return {50.0f, 60.0f, 110.0f, 130.0f}; // mmHg
        case ParameterType::IBP2_SYS:
            return {0.0f, 2.0f, 15.0f, 20.0f};     // mmHg (CVP)
        case ParameterType::IBP2_DIA:
            return {0.0f, 0.0f, 8.0f, 12.0f};      // mmHg (CVP)
        case ParameterType::IBP2_MAP:
            return {0.0f, 1.0f, 10.0f, 15.0f};     // mmHg (CVP)
        case ParameterType::TEMP1:
        case ParameterType::TEMP2:
            return {35.0f, 36.0f, 38.0f, 39.0f};   // °C
        case ParameterType::ETCO2:
            return {20.0f, 25.0f, 45.0f, 50.0f};   // mmHg
        default:
            return {0.0f, 0.0f, 100.0f, 100.0f};   // Default limits
    }
}

/**
 * @brief Default sample rate for waveforms in samples per second
 * 
 * Defines the standard sampling rate for waveform data in the system.
 * This value is used for buffer sizing, time calculations, and when
 * generating demo data. A rate of 250 samples per second is sufficient
 * for most physiological waveforms while maintaining reasonable performance.
 */
const int DEFAULT_SAMPLE_RATE = 250;

/**
 * @brief Default buffer size in seconds
 * 
 * Defines the default amount of waveform history to maintain in memory.
 * This determines how far back in time waveform displays can show data
 * and affects memory usage. Combined with the sample rate, this determines
 * the total number of samples stored for each waveform.
 */
const int DEFAULT_BUFFER_SECONDS = 10;

/**
 * @brief Default sweep speed in pixels per second
 * 
 * Defines the default horizontal scrolling speed for waveform displays.
 * This value determines how many horizontal pixels represent one second
 * of waveform data. Higher values show more detail but less history on
 * the screen at once.
 */
const double DEFAULT_SWEEP_SPEED = 25.0;

} // namespace VitalSync

#endif // VITAL_SYNC_TYPES_H 
