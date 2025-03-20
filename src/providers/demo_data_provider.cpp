/**
 * @file demo_data_provider.cpp
 * @brief Implementation of the DemoDataProvider class
 *
 * This file implements the DemoDataProvider class responsible for generating
 * simulated physiological waveforms and parameter data. The implementation includes
 * realistic waveform generation algorithms for various physiological signals (ECG,
 * respiration, plethysmograph, arterial pressure, capnograph) and parameter value
 * generation with appropriate clinical correlations and variability.
 */
#include "demo_data_provider.h"
#include "../../include/config_manager.h"
#include <QDateTime>
#include <QDebug>
#include <QtMath>
#include <cmath>

/**
 * @brief Constants for waveform and parameter generation
 *
 * This anonymous namespace contains constants used for generating
 * realistic physiological waveforms and helper functions.
 */
namespace {
    // ECG waveform constants
    const double ECG_P_AMPLITUDE = 0.25;    ///< P-wave amplitude
    const double ECG_P_WIDTH = 0.08;        ///< P-wave width
    const double ECG_P_OFFSET = 0.16;       ///< P-wave offset in cardiac cycle
    const double ECG_Q_AMPLITUDE = -0.1;    ///< Q-wave amplitude
    const double ECG_Q_WIDTH = 0.03;        ///< Q-wave width
    const double ECG_Q_OFFSET = 0.31;       ///< Q-wave offset in cardiac cycle
    const double ECG_R_AMPLITUDE = 1.0;     ///< R-wave amplitude
    const double ECG_R_WIDTH = 0.05;        ///< R-wave width
    const double ECG_R_OFFSET = 0.34;       ///< R-wave offset in cardiac cycle
    const double ECG_S_AMPLITUDE = -0.25;   ///< S-wave amplitude
    const double ECG_S_WIDTH = 0.03;        ///< S-wave width
    const double ECG_S_OFFSET = 0.37;       ///< S-wave offset in cardiac cycle
    const double ECG_T_AMPLITUDE = 0.35;    ///< T-wave amplitude
    const double ECG_T_WIDTH = 0.1;         ///< T-wave width
    const double ECG_T_OFFSET = 0.5;        ///< T-wave offset in cardiac cycle
    
    // Pleth waveform constants
    const double PLETH_SYSTOLIC_UPSTROKE = 0.1;     ///< Time to systolic upstroke
    const double PLETH_SYSTOLIC_PEAK = 0.3;         ///< Time to systolic peak
    const double PLETH_DICROTIC_NOTCH = 0.45;       ///< Time to dicrotic notch
    const double PLETH_DIASTOLE_END = 0.9;          ///< Time to end of diastole
    const double PLETH_AMPLITUDE = 1.0;             ///< Amplitude scaling factor
    const double PLETH_DICROTIC_AMPLITUDE = 0.05;   ///< Dicrotic notch amplitude
    
    // ABP waveform constants
    const double ABP_SYSTOLIC_UPSTROKE = 0.1;       ///< Time to systolic upstroke
    const double ABP_SYSTOLIC_PEAK = 0.2;           ///< Time to systolic peak
    const double ABP_DICROTIC_NOTCH = 0.35;         ///< Time to dicrotic notch
    const double ABP_DIASTOLE_END = 0.9;            ///< Time to end of diastole
    
    // Capnography constants
    const double CAPNO_INSPIRATION_END = 0.3;       ///< Time to end of inspiration
    const double CAPNO_PLATEAU_START = 0.5;         ///< Time to start of plateau
    const double CAPNO_PLATEAU_END = 0.8;           ///< Time to end of plateau
    const double CAPNO_EXPIRATION_END = 0.9;        ///< Time to end of expiration
    
    // Default parameter update interval in milliseconds - reduced for more frequent updates
    const int DEFAULT_PARAMETER_UPDATE_MS = 1000;   ///< Default parameter update interval (1 second)
    
    // Default waveform update interval in milliseconds
    const int DEFAULT_WAVEFORM_UPDATE_MS = 40;      ///< Default waveform update interval (40ms = 25Hz)
    
    /**
     * @brief Generate a random double value within a bounded range
     * @param generator Reference to the random number generator
     * @param min Minimum value (inclusive)
     * @param max Maximum value (exclusive)
     * @return Random double value between min and max
     */
    double boundedDouble(QRandomGenerator& generator, double min, double max) {
        // Scale to integer range (1000 steps), perform bounded, then scale back
        const int SCALE = 1000;
        int minScaled = static_cast<int>(min * SCALE);
        int maxScaled = static_cast<int>(max * SCALE);
        
        // Ensure maxScaled is greater than minScaled to avoid assertion failure
        if (maxScaled <= minScaled) {
            // Ensure at least 1 difference between min and max
            maxScaled = minScaled + 1;
        }
        
        int result = generator.bounded(minScaled, maxScaled);
        return static_cast<double>(result) / SCALE;
    }
}

/**
 * @brief Constructs the DemoDataProvider with default parameter values
 * @param parent Parent QObject for memory management
 * 
 * Initializes the provider with default vital sign values within the normal range
 * and sets up the timers for generating waveform and parameter data.
 */
DemoDataProvider::DemoDataProvider(QObject* parent)
    : IDataProvider(parent)
    , active_(false)
    , status_(VitalSync::ConnectionStatus::Disconnected)
    , heart_rate_(70)
    , respiration_rate_(15)
    , spo2_(98)
    , systolic_bp_(120)
    , diastolic_bp_(80)
    , mean_bp_(93)
    , temperature_(37.0)
    , temperature2_(36.5)  // Slightly lower than core temperature
    , ibp1_systolic_(125)  // Slightly higher than NIBP (typical for arterial line)
    , ibp1_diastolic_(75)  // Slightly lower than NIBP
    , ibp1_mean_(92)       // Calculated mean
    , ibp2_systolic_(15)   // CVP systolic (central venous pressure)
    , ibp2_diastolic_(5)   // CVP diastolic
    , ibp2_mean_(8)        // CVP mean
    , etco2_(35)
    , waveform_update_interval_ms_(40)  // 40ms = 25Hz, which is more realistic for clinical monitors
    , parameter_update_interval_ms_(DEFAULT_PARAMETER_UPDATE_MS)
    , amplitude_(1.0)
    , frequency_(1.0)
    , noise_(0.02)  // Reduced noise for cleaner waveforms
    , artifacts_(false)
{
    random_.seed(QDateTime::currentMSecsSinceEpoch());
    
    // Initialize waveform generators
    initialize();
    
    // Start timers
    connect(&waveform_timer_, &QTimer::timeout, this, &DemoDataProvider::generateWaveformData);
    waveform_timer_.setInterval(waveform_update_interval_ms_);
    
    connect(&parameter_timer_, &QTimer::timeout, this, &DemoDataProvider::generateParameterData);
    parameter_timer_.setInterval(parameter_update_interval_ms_);
    
    // Load configuration
    auto& config = ConfigManager::GetInstance();
    QVariantMap providerConfig = config.GetProviderConfig("Demo");
    
    if (!providerConfig.isEmpty()) {
        configure(providerConfig);
    }
}

DemoDataProvider::~DemoDataProvider()
{
    stop();
}

/**
 * @brief Applies configuration parameters to the demo provider
 * @param params QVariantMap containing configuration parameters
 * @return True if configuration was successful
 * 
 * Configures the demo provider with the specified parameters, which can
 * include heart rate, respiration rate, SpO2, blood pressure values,
 * temperature, update intervals, and simulation parameters such as 
 * amplitude, frequency, noise level, and artifact presence.
 */
bool DemoDataProvider::configure(const QVariantMap& params)
{
    QMutexLocker locker(&mutex_);
    
    // Update parameter values if present in params
    if (params.contains("heartRate"))
        heart_rate_ = params["heartRate"].toInt();
        
    if (params.contains("respirationRate"))
        respiration_rate_ = params["respirationRate"].toInt();
        
    if (params.contains("spo2"))
        spo2_ = params["spo2"].toInt();
        
    if (params.contains("systolicBP"))
        systolic_bp_ = params["systolicBP"].toInt();
        
    if (params.contains("diastolicBP"))
        diastolic_bp_ = params["diastolicBP"].toInt();
    
    // Calculate mean BP
    mean_bp_ = diastolic_bp_ + (systolic_bp_ - diastolic_bp_) / 3;
        
    if (params.contains("temperature"))
        temperature_ = params["temperature"].toDouble();
        
    if (params.contains("temperature2"))
        temperature2_ = params["temperature2"].toDouble();
        
    if (params.contains("etco2"))
        etco2_ = params["etco2"].toInt();
        
    if (params.contains("ibp1Systolic"))
        ibp1_systolic_ = params["ibp1Systolic"].toInt();
        
    if (params.contains("ibp1Diastolic"))
        ibp1_diastolic_ = params["ibp1Diastolic"].toInt();
    
    // Calculate IBP1 mean
    ibp1_mean_ = ibp1_diastolic_ + (ibp1_systolic_ - ibp1_diastolic_) / 3;
        
    if (params.contains("ibp2Systolic"))
        ibp2_systolic_ = params["ibp2Systolic"].toInt();
        
    if (params.contains("ibp2Diastolic"))
        ibp2_diastolic_ = params["ibp2Diastolic"].toInt();
    
    // Calculate IBP2 mean
    ibp2_mean_ = ibp2_diastolic_ + (ibp2_systolic_ - ibp2_diastolic_) / 3;
        
    if (params.contains("UpdateInterval"))
        waveform_update_interval_ms_ = params["UpdateInterval"].toInt();
        
    if (params.contains("amplitude"))
        amplitude_ = params["amplitude"].toDouble();
        
    if (params.contains("frequency"))
        frequency_ = params["frequency"].toDouble();
        
    if (params.contains("noise"))
        noise_ = params["noise"].toDouble();
        
    if (params.contains("artifacts"))
        artifacts_ = params["artifacts"].toBool();
    
    // Update timers if running
    if (waveform_timer_.isActive()) {
        waveform_timer_.setInterval(waveform_update_interval_ms_);
    }
    
    // Save configuration
    QVariantMap config;
    config["heartRate"] = heart_rate_;
    config["respirationRate"] = respiration_rate_;
    config["spo2"] = spo2_;
    config["systolicBP"] = systolic_bp_;
    config["diastolicBP"] = diastolic_bp_;
    config["temperature"] = temperature_;
    config["temperature2"] = temperature2_;
    config["etco2"] = etco2_;
    config["ibp1Systolic"] = ibp1_systolic_;
    config["ibp1Diastolic"] = ibp1_diastolic_;
    config["ibp2Systolic"] = ibp2_systolic_;
    config["ibp2Diastolic"] = ibp2_diastolic_;
    config["UpdateInterval"] = waveform_update_interval_ms_;
    config["amplitude"] = amplitude_;
    config["frequency"] = frequency_;
    config["noise"] = noise_;
    config["artifacts"] = artifacts_;
    
    ConfigManager::GetInstance().SetProviderConfig("Demo", config);
    
    return true;
}

/**
 * @brief Initializes the waveform generator function map
 * 
 * Maps each supported waveform type to its corresponding generator function.
 * This allows for direct lookup of the appropriate generator function when
 * producing data for a specific waveform type.
 */
void DemoDataProvider::initialize()
{
    // Map waveform types to generator functions
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::ECG_I)] =
        [this](double time, int points) { return generateECG(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::ECG_II)] =
        [this](double time, int points) { return generateECG(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::ECG_III)] =
        [this](double time, int points) { return generateECG(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::RESP)] =
        [this](double time, int points) { return generateRespiration(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::PLETH)] =
        [this](double time, int points) { return generatePlethysmograph(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::ABP)] =
        [this](double time, int points) { return generateArterialPressure(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::CAPNO)] =
        [this](double time, int points) { return generateCapnograph(time, points); };
}

/**
 * @brief Starts the demo data provider
 * @return True if successfully started
 * 
 * Initiates the generation of simulated physiological data. The provider
 * first transitions to a Connecting state, then after a brief delay to
 * simulate connection time, moves to Connected state and begins generating data.
 */
bool DemoDataProvider::start()
{
    {
        QMutexLocker locker(&mutex_);
        
        if (active_) {
            qDebug() << "DemoDataProvider: Already started, ignoring start request";
            return true; // Already started
        }
        
        // Set the status to connecting immediately
        status_ = VitalSync::ConnectionStatus::Connecting;
    } // Release the mutex before emitting signals
    
    qDebug() << "DemoDataProvider: Starting... (status: connecting)";
    emit connectionStatusChanged(status_);
    
    // Start with a small delay to simulate connection
    QTimer::singleShot(500, this, [this]() {
        qDebug() << "DemoDataProvider: Connection delay completed, starting data generation";
        
        // Start timers - do not hold mutex while starting timers to avoid potential deadlock
        elapsed_timer_.start();
        waveform_timer_.start();
        parameter_timer_.start();
        
        {
            QMutexLocker innerLocker(&mutex_);
            active_ = true;
            status_ = VitalSync::ConnectionStatus::Connected;
        } // Release mutex before emitting signals
        
        qDebug() << "DemoDataProvider: Status changed to Connected";
        emit connectionStatusChanged(status_);
        
        // Generate initial data immediately - but don't hold the mutex while doing so
        QTimer::singleShot(0, this, [this]() {
            qDebug() << "DemoDataProvider: Generating initial parameter data";
            generateParameterData();
            
            qDebug() << "DemoDataProvider: Generating initial waveform data";
            generateWaveformData();
            
            qDebug() << "DemoDataProvider: Started successfully";
        });
    });
    
    return true;
}

/**
 * @brief Stops the demo data provider
 * 
 * Halts the generation of simulated physiological data and transitions
 * the provider to a Disconnected state.
 */
void DemoDataProvider::stop()
{
    bool was_active = false;
    {
        QMutexLocker locker(&mutex_);
        
        was_active = active_;
        
        // Mark as inactive first
        active_ = false;
        status_ = VitalSync::ConnectionStatus::Disconnected;
    } // Release the mutex before stopping timers
    
    if (was_active) {
        qDebug() << "DemoDataProvider: Stopping data generation...";
        
        // Stop timers - don't hold mutex to avoid deadlocks
        waveform_timer_.stop();
        parameter_timer_.stop();
        
        // Emit signal outside of mutex lock
        qDebug() << "DemoDataProvider: Status changed to Disconnected";
        emit connectionStatusChanged(status_);
        
        qDebug() << "DemoDataProvider: Stopped successfully";
    } else {
        qDebug() << "DemoDataProvider: Already stopped, ignoring stop request";
    }
}

/**
 * @brief Get the current connection status
 * @return Current connection status
 * 
 * Returns the current status of the demo provider (Disconnected,
 * Connecting, or Connected).
 */
VitalSync::ConnectionStatus DemoDataProvider::GetConnectionStatus() const
{
    QMutexLocker locker(&mutex_);
    return status_;
}

/**
 * @brief Get provider name
 * @return Name of the provider
 * 
 * Returns the name of this provider ("Demo").
 */
std::string DemoDataProvider::GetName() const
{
    return "Demo";
}

/**
 * @brief Check if this provider is currently in use
 * @return True if this provider is active
 * 
 * Returns whether the demo provider is currently active and
 * generating physiological data.
 */
bool DemoDataProvider::isActive() const
{
    QMutexLocker locker(&mutex_);
    return active_;
}

/**
 * @brief Generates and emits simulated waveform data
 * 
 * This function generates simulated waveform data for all configured waveform types
 * and emits the data to connected slots. It uses the current elapsed time to 
 * determine the phase in each waveform and creates a specified number of data points
 * for each update cycle.
 */
void DemoDataProvider::generateWaveformData()
{
    QMutexLocker locker(&mutex_);
    
    if (!active_) {
        return;
    }
    
    // Calculate elapsed time in seconds
    double elapsedTimeSeconds = elapsed_timer_.elapsed() / 1000.0;
    
    // Number of points to generate per update - for more realistic waveform appearance
    int pointsPerUpdate = 3; // Fewer points per update creates smoother, more realistic waveforms
    
    // Get current timestamp
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    
    // Generate and emit data for each waveform type
    for (auto it = waveform_generators_.begin(); it != waveform_generators_.end(); ++it) {
        int waveformId = it->first;
        auto generatorFunc = it->second;
        
        // Generate waveform data
        QVector<float> data = generatorFunc(elapsedTimeSeconds, pointsPerUpdate);
        
        // Active debug output to see what's being generated
        qDebug() << "DemoDataProvider: Generated waveform" << waveformId
                 << "Points:" << data.size() 
                 << "First 3 values:" << (data.size() > 0 ? data[0] : 0.0)
                 << (data.size() > 1 ? data[1] : 0.0)
                 << (data.size() > 2 ? data[2] : 0.0);
            
        // Emit waveform data
        emit waveformDataReceived(waveformId, timestamp, data);
    }
}

/**
 * @brief Occasionally generates parameter values outside normal range
 * @param baseValue Base parameter value
 * @param minValue Normal minimum value for the parameter
 * @param maxValue Normal maximum value for the parameter
 * @param cycleCount Current cycle counter (for timing)
 * @return Possibly modified value that might exceed normal limits
 * 
 * Periodically generates abnormal parameter values that exceed
 * normal limits to simulate patient deterioration events and
 * test alarm functionality.
 */
double DemoDataProvider::generateExtremeValue(double baseValue, double minValue, double maxValue, int cycleCount)
{
    // Generate extreme values much more frequently - every 8 cycles with 40% probability
    if (cycleCount % 8 == 0 && random_.bounded(100) < 40) {
        // 60% chance to go above normal range, 40% chance to go below
        if (random_.bounded(100) < 60) {
            // Above normal range - exceed by 10-30%
            double exceedFactor = 1.10 + (random_.bounded(20) / 100.0);
            return maxValue * exceedFactor;
        } else {
            // Below normal range - go below by 10-30%
            double exceedFactor = 0.90 - (random_.bounded(20) / 100.0);
            return minValue * exceedFactor;
        }
    }
    
    // Otherwise return the regular value
    return baseValue;
}

/**
 * @brief Frequently generates critical parameter values outside normal range
 * @param baseValue Base parameter value
 * @param minValue Normal minimum value for the parameter
 * @param maxValue Normal maximum value for the parameter
 * @param cycleCount Current cycle counter (for timing)
 * @return Possibly modified value that often exceeds normal limits with greater deviation
 * 
 * Frequently generates significantly abnormal parameter values for
 * critical parameters like heart rate and SpO2 to simulate severe
 * clinical events and test critical alarm functionality.
 */
double DemoDataProvider::generateCriticalExtremeValue(double baseValue, double minValue, double maxValue, int cycleCount)
{
    // For critical parameters, generate extreme values even more frequently
    // Every 5 cycles with 60% probability
    if (cycleCount % 5 == 0 && random_.bounded(100) < 60) {
        // 70% chance to go above normal range, 30% chance to go below
        if (random_.bounded(100) < 70) {
            // Above normal range - exceed by 15-40%
            double exceedFactor = 1.15 + (random_.bounded(25) / 100.0);
            return maxValue * exceedFactor;
        } else {
            // Below normal range - go below by 15-40%
            double exceedFactor = 0.85 - (random_.bounded(25) / 100.0);
            return minValue * exceedFactor;
        }
    }
    
    // Otherwise return the regular value
    return baseValue;
}

/**
 * @brief Generates and emits simulated parameter data
 * 
 * This function generates simulated physiological parameter values such as
 * heart rate, blood pressure, SpO2, etc. It adds appropriate variations and
 * correlations between parameters to simulate realistic patient data, and
 * occasionally generates abnormal values to test alarm functionality.
 */
void DemoDataProvider::generateParameterData()
{
    // Check if we're still active before proceeding
    bool is_connected = false;
    double heart_rate, respiration_rate, spo2_value, systolic_bp, diastolic_bp;
    double temperature_value, temperature2_value, etco2_value;
    double ibp2_sys, ibp2_dia;
    
    {
        QMutexLocker locker(&mutex_);
        
        if (status_ != VitalSync::ConnectionStatus::Connected || !active_) {
            return; // Exit early if not connected or not active
        }
        
        is_connected = true;
        
        // Copy values locally while under mutex lock
        heart_rate = heart_rate_;
        respiration_rate = respiration_rate_;
        spo2_value = spo2_;
        systolic_bp = systolic_bp_;
        diastolic_bp = diastolic_bp_;
        temperature_value = temperature_;
        temperature2_value = temperature2_;
        etco2_value = etco2_;
        ibp2_sys = ibp2_systolic_;
        ibp2_dia = ibp2_diastolic_;
    } // Release mutex for calculations
    
    if (!is_connected) {
        return;
    }
    
    try {
        // Increment cycle counter for cyclic variations
        cycleCounter++;
        
        // Generate heart rate with natural variation and possibly extreme values
        // Use sine wave to create a slow oscillation in the base value
        double hrFactor = sin(cycleCounter * 0.005) * 3.0;  // Slower oscillation of ±3 bpm
        
        // Occasionally generate extreme heart rate values
        double hr_base = heart_rate + hrFactor;
        // Check if we should generate an extreme value (40-150 is normal range for HR)
        hr_base = generateCriticalExtremeValue(hr_base, 40, 150, cycleCounter);
        int heartRate = qRound(addVariation(hr_base, 0.02));
        
        // Generate respiration rate with correlation to heart rate and possibly extreme values
        // Higher heart rates often correlate with higher respiration rates
        double rrFactor = (heartRate > heart_rate) ? 0.2 : -0.2;  // Slight correlation, more subtle
        double rr_base = respiration_rate + rrFactor;
        // Check if we should generate an extreme value (8-30 is normal range for RR)
        rr_base = generateExtremeValue(rr_base, 8, 30, cycleCounter + 3);
        int respirationRate = qRound(addVariation(rr_base, 0.03));
        
        // Generate SpO2 with inverse correlation to heart rate and possibly extreme values
        // Higher heart rates might indicate stress and lower SpO2
        double spo2Factor = (heartRate > heart_rate + 10) ? -0.2 : 0.1;
        double spo2_base = spo2_value + spo2Factor;
        // Check if we should generate an extreme value (94-100 is normal range for SpO2)
        spo2_base = generateCriticalExtremeValue(spo2_base, 94, 100, cycleCounter + 7);
        
        // Special case for SpO2 to create more realistic clinical scenarios
        // Occasionally generate very low SpO2 values (dangerous hypoxemia)
        if (cycleCounter % 30 == 0 && random_.bounded(100) < 25) {
            // Generate severely low SpO2 between 70-85%
            spo2_base = 70.0 + random_.bounded(16);
        }
        
        int spo2 = qRound(addVariation(spo2_base, 0.01));
        // Ensure SpO2 is never above 100%
        spo2 = std::min(spo2, 100);
        
        // Generate blood pressure with correlation to heart rate and possibly extreme values
        // Higher heart rates often result in higher systolic but lower diastolic
        double sysFactor = (heartRate > heart_rate) ? 0.5 : -0.3;
        double diaFactor = (heartRate > heart_rate) ? -0.3 : 0.2;
        double sys_base = systolic_bp + sysFactor;
        double dia_base = diastolic_bp + diaFactor;
        
        // Check if we should generate extreme values (90-140/60-90 is normal range for BP)
        sys_base = generateCriticalExtremeValue(sys_base, 90, 140, cycleCounter + 11);
        dia_base = generateCriticalExtremeValue(dia_base, 60, 90, cycleCounter + 13);
        int systolicBP = qRound(addVariation(sys_base, 0.03));
        int diastolicBP = qRound(addVariation(dia_base, 0.03));
        
        // Ensure systolic is always higher than diastolic
        if (systolicBP <= diastolicBP) {
            systolicBP = diastolicBP + 20;
        }
        
        int meanBP = qRound(diastolicBP + (systolicBP - diastolicBP) / 3.0);
        
        // Arterial pressure is typically slightly higher than non-invasive measurements
        // and can also have extreme values
        double ibp1_sys_base = systolicBP + 5;
        double ibp1_dia_base = diastolicBP - 2;
        
        // Check if we should generate extreme values for IBP
        ibp1_sys_base = generateExtremeValue(ibp1_sys_base, 90, 140, cycleCounter + 17);
        ibp1_dia_base = generateExtremeValue(ibp1_dia_base, 60, 90, cycleCounter + 19);
        int ibp1Systolic = qRound(addVariation(ibp1_sys_base, 0.02));
        int ibp1Diastolic = qRound(addVariation(ibp1_dia_base, 0.02));
        
        // Ensure systolic is always higher than diastolic
        if (ibp1Systolic <= ibp1Diastolic) {
            ibp1Systolic = ibp1Diastolic + 20;
        }
        
        int ibp1Mean = qRound(ibp1Diastolic + (ibp1Systolic - ibp1Diastolic) / 3.0);
        
        // Generate IBP2 (CVP) values - these can fluctuate with respiration, but more subtly
        // and can have extreme values
        double cvpFactor = sin((cycleCounter + 50) * 0.025);  // Slower oscillation
        double ibp2_sys_base = ibp2_sys + (cvpFactor * 2);
        double ibp2_dia_base = ibp2_dia + (cvpFactor * 1.5);
        
        // Check if we should generate extreme values for CVP (2-8 is normal range)
        ibp2_sys_base = generateExtremeValue(ibp2_sys_base, 2, 8, cycleCounter + 23);
        ibp2_dia_base = generateExtremeValue(ibp2_dia_base, 2, 8, cycleCounter + 29);
        int ibp2Systolic = qRound(addVariation(ibp2_sys_base, 0.08));
        int ibp2Diastolic = qRound(addVariation(ibp2_dia_base, 0.08));
        
        // Ensure systolic is always higher than diastolic for CVP
        if (ibp2Systolic <= ibp2Diastolic) {
            ibp2Systolic = ibp2Diastolic + 2;
        }
        
        int ibp2Mean = qRound(ibp2Diastolic + (ibp2Systolic - ibp2Diastolic) / 3.0);
        
        // Generate temperature values - core temperature varies very little
        // but can occasionally have extreme values
        double temp_base = temperature_value;
        // Check if we should generate extreme values (36-38 is normal range)
        temp_base = generateExtremeValue(temp_base, 36.0, 38.0, cycleCounter + 31);
        
        // Special case for temperature to create more realistic fever scenarios
        if (cycleCounter % 25 == 0 && random_.bounded(100) < 30) {
            // Generate high fever (39-41°C)
            temp_base = 39.0 + (random_.bounded(200) / 100.0);
        } else if (cycleCounter % 40 == 0 && random_.bounded(100) < 20) {
            // Generate hypothermia (33-35°C)
            temp_base = 33.0 + (random_.bounded(200) / 100.0);
        }
        
        double temperature = addVariation(temp_base, 0.005);
        
        // Peripheral temperature correlates with cardiac output (heart rate)
        double temp2Factor = (heartRate < 60) ? -0.1 : (heartRate > 100 ? 0.1 : 0);
        double temp2_base = temperature2_value + temp2Factor;
        // Check if we should generate extreme values (35.5-37.5 is normal range)
        temp2_base = generateExtremeValue(temp2_base, 35.5, 37.5, cycleCounter + 37);
        double temperature2 = addVariation(temp2_base, 0.008);
        
        // Generate ETCO2 with relationship to respiration rate - slower changes
        // and can have extreme values
        double etco2Factor = (respirationRate > 20) ? -0.2 * (respirationRate - 20) : 
                              (respirationRate < 10 ? 0.3 * (10 - respirationRate) : 0);
        double etco2_base = etco2_value + etco2Factor;
        // Check if we should generate extreme values (35-45 is normal range)
        etco2_base = generateExtremeValue(etco2_base, 35, 45, cycleCounter + 41);
        
        // Special case for ETCO2 - create clinically relevant scenarios
        if (cycleCounter % 22 == 0 && random_.bounded(100) < 35) {
            if (random_.bounded(100) < 50) {
                // Hypercapnia - high CO2 retention (50-80 mmHg)
                etco2_base = 50 + random_.bounded(31);
            } else {
                // Hypocapnia - low CO2 (15-30 mmHg) - can indicate hyperventilation
                etco2_base = 15 + random_.bounded(16);
            }
        }
        
        int etco2 = qRound(addVariation(etco2_base, 0.04));
        
        // Get current timestamp
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
        
        // Debug output with improved formatting
        qDebug() << "=== DemoDataProvider: Generated parameters at" << QDateTime::fromMSecsSinceEpoch(timestamp).toString("hh:mm:ss.zzz") << "===";
        qDebug() << "  HR:" << heartRate << "bpm (base:" << heart_rate << ")";
        qDebug() << "  RR:" << respirationRate << "br/min (base:" << respiration_rate << ")";
        qDebug() << "  SpO2:" << spo2 << "% (base:" << spo2_value << ")";
        qDebug() << "  NIBP:" << systolicBP << "/" << diastolicBP << "(" << meanBP << ") mmHg (base:" << systolic_bp << "/" << diastolic_bp << ")";
        qDebug() << "  IBP1:" << ibp1Systolic << "/" << ibp1Diastolic << "(" << ibp1Mean << ") mmHg";
        qDebug() << "  IBP2:" << ibp2Systolic << "/" << ibp2Diastolic << "(" << ibp2Mean << ") mmHg (base:" << ibp2_sys << "/" << ibp2_dia << ")";
        qDebug() << "  TEMP:" << temperature << "°C TEMP2:" << temperature2 << "°C (base:" << temperature_value << "/" << temperature2_value << ")";
        qDebug() << "  ETCO2:" << etco2 << "mmHg (base:" << etco2_value << ")";
        qDebug() << "==============================================";
        
        // Emit parameter data
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::HR), timestamp, heartRate);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::RR), timestamp, respirationRate);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::SPO2), timestamp, spo2);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::NIBP_SYS), timestamp, systolicBP);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::NIBP_DIA), timestamp, diastolicBP);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::NIBP_MAP), timestamp, meanBP);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::ETCO2), timestamp, etco2);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::TEMP1), timestamp, temperature);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::TEMP2), timestamp, temperature2);
        
        // Emit IBP1 (Arterial) parameters
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::IBP1_SYS), timestamp, ibp1Systolic);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::IBP1_DIA), timestamp, ibp1Diastolic);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::IBP1_MAP), timestamp, ibp1Mean);
        
        // Emit IBP2 (CVP) parameters
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::IBP2_SYS), timestamp, ibp2Systolic);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::IBP2_DIA), timestamp, ibp2Diastolic);
        emit parameterDataReceived(static_cast<int>(VitalSync::ParameterType::IBP2_MAP), timestamp, ibp2Mean);
    } catch (const std::exception& e) {
        qCritical() << "Exception in generateParameterData:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in generateParameterData";
    }
}

/**
 * @brief Generates ECG waveform data
 * @param time Current time in seconds
 * @param points Number of points to generate
 * @return Vector of generated ECG data points
 * 
 * Creates a realistic ECG waveform with P, Q, R, S, and T waves.
 * The waveform's timing is based on the current heart rate.
 */
QVector<float> DemoDataProvider::generateECG(double time, int points)
{
    QVector<float> result;
    result.reserve(points);
    
    // Calculate time step based on heart rate (in seconds)
    double cycleTime = 60.0 / heart_rate_;
    double timeStep = 1.0 / (1000.0 / waveform_update_interval_ms_);
    
    // Make the ECG more pronounced with higher peaks and clearer features
    double amplitudeMultiplier = 2.2; // Increased amplitude for better visibility
    
    for (int i = 0; i < points; ++i) {
        double t = time + i * timeStep;
        
        // Normalize to cycle phase (0-1)
        double cyclePhase = fmod(t, cycleTime) / cycleTime;
        
        // ECG waveform generation - PQRST complex
        double value = 0.0;
        
        // Add baseline wander using slow sine wave for realism
        double baseline = 0.05 * sin(2.0 * M_PI * (t / 10.0));
        
        // P wave - more distinct with slightly increased width
        if (cyclePhase < ECG_P_OFFSET + ECG_P_WIDTH && cyclePhase > ECG_P_OFFSET - ECG_P_WIDTH) {
            value += ECG_P_AMPLITUDE * 1.2 * exp(-pow((cyclePhase - ECG_P_OFFSET) / (ECG_P_WIDTH / 2.0), 2));
        }
        
        // Q wave - slightly deeper
        if (cyclePhase < ECG_Q_OFFSET + ECG_Q_WIDTH && cyclePhase > ECG_Q_OFFSET - ECG_Q_WIDTH) {
            value += ECG_Q_AMPLITUDE * 1.3 * exp(-pow((cyclePhase - ECG_Q_OFFSET) / (ECG_Q_WIDTH / 2.0), 2));
        }
        
        // R wave - more prominent peak
        if (cyclePhase < ECG_R_OFFSET + ECG_R_WIDTH && cyclePhase > ECG_R_OFFSET - ECG_R_WIDTH) {
            value += ECG_R_AMPLITUDE * 1.4 * exp(-pow((cyclePhase - ECG_R_OFFSET) / (ECG_R_WIDTH / 2.0), 2));
        }
        
        // S wave - slightly deeper
        if (cyclePhase < ECG_S_OFFSET + ECG_S_WIDTH && cyclePhase > ECG_S_OFFSET - ECG_S_WIDTH) {
            value += ECG_S_AMPLITUDE * 1.2 * exp(-pow((cyclePhase - ECG_S_OFFSET) / (ECG_S_WIDTH / 2.0), 2));
        }
        
        // T wave - more distinct shape
        if (cyclePhase < ECG_T_OFFSET + ECG_T_WIDTH && cyclePhase > ECG_T_OFFSET - ECG_T_WIDTH) {
            value += ECG_T_AMPLITUDE * 1.3 * exp(-pow((cyclePhase - ECG_T_OFFSET) / (ECG_T_WIDTH / 2.0), 2));
        }
        
        // Add very subtle U wave occasionally
        if (random_.bounded(100) < 20) { // 20% chance of U wave
            double u_offset = ECG_T_OFFSET + ECG_T_WIDTH + 0.05;
            double u_width = 0.06;
            if (cyclePhase < u_offset + u_width && cyclePhase > u_offset - u_width) {
                value += 0.15 * exp(-pow((cyclePhase - u_offset) / (u_width / 2.0), 2));
            }
        }
        
        // Add baseline wander
        value += baseline;
        
        // Add minimal noise
        if (noise_ > 0) {
            value += boundedDouble(random_, -noise_/3, noise_/3);
        }
        
        // Multiply by amplitude factor for better visibility
        value *= amplitudeMultiplier * amplitude_;
        
        result.append(value);
    }
    
    return result;
}

/**
 * @brief Generates respiration waveform data
 * @param time Current time in seconds
 * @param points Number of points to generate
 * @return Vector of generated respiration data points
 * 
 * Creates a realistic respiration waveform using sinusoidal
 * patterns that simulate inhalation and exhalation phases.
 * The waveform's frequency is based on the current respiration rate.
 */
QVector<float> DemoDataProvider::generateRespiration(double time, int points)
{
    QVector<float> result;
    result.reserve(points);
    
    // Calculate time step based on respiration rate (in seconds)
    double cycleTime = 60.0 / respiration_rate_;
    double timeStep = 1.0 / (1000.0 / waveform_update_interval_ms_);
    
    for (int i = 0; i < points; ++i) {
        double t = time + i * timeStep;
        
        // Simple sinusoidal waveform with asymmetrical inspiration/expiration
        double phase = fmod(t, cycleTime) / cycleTime;
        
        // Use a modified sine wave to create asymmetric inspiration/expiration
        double value = 0.0;
        
        if (phase < 0.4) {
            // Inspiration (faster)
            value = sin(phase * M_PI / 0.4);
        } else {
            // Expiration (slower)
            value = sin(((phase - 0.4) * M_PI / 0.6) + M_PI);
        }
        
        // Add noise
        if (noise_ > 0) {
            value += boundedDouble(random_, -noise_, noise_);
        }
        
        // Scale by amplitude
        value *= amplitude_ * 0.5; // Make respiration smaller than ECG by default
        
        result.append(value);
    }
    
    return result;
}

/**
 * @brief Generates plethysmograph (SpO2) waveform data
 * @param time Current time in seconds
 * @param points Number of points to generate
 * @return Vector of generated plethysmograph data points
 * 
 * Creates a realistic plethysmograph waveform simulating the
 * pulsatile blood flow detected by an SpO2 sensor, including
 * the characteristic systolic peak and dicrotic notch.
 */
QVector<float> DemoDataProvider::generatePlethysmograph(double time, int points)
{
    QVector<float> result;
    result.reserve(points);
    
    // Calculate time step based on heart rate
    double cycleTime = 60.0 / heart_rate_;
    double timeStep = 1.0 / (1000.0 / waveform_update_interval_ms_);
    
    // Increased amplitude factor for better visualization
    double amplitudeMultiplier = 2.5;
    
    // Base SpO2 amplitude scales with the SpO2 value (lower SpO2 = lower amplitude)
    double baseAmplitude = (spo2_ / 100.0) * amplitude_;
    
    for (int i = 0; i < points; ++i) {
        double t = time + i * timeStep;
        
        // Normalize to cycle phase (0-1)
        double cyclePhase = fmod(t, cycleTime) / cycleTime;
        
        double value = 0.0;
        
        // Slight respiratory modulation (slower sinus rhythm affecting baseline)
        double respEffect = 0.15 * sin(2.0 * M_PI * (t / (60.0 / respiration_rate_)));
        
        // Create the primary pulse wave with skewed gaussian for more realistic rise/fall
        if (cyclePhase <= 0.35) {
            // Systolic upstroke - faster rise
            value += baseAmplitude * 0.95 * exp(-pow((cyclePhase - 0.15) / 0.08, 2));
        } else {
            // Diastolic runoff - slower fall
            double fallFactor = 1.0 - pow((cyclePhase - 0.35) / 0.65, 0.7);
            value += baseAmplitude * 0.4 * fallFactor * fallFactor;
        }
        
        // Add dicrotic notch
        if (cyclePhase > 0.35 && cyclePhase < 0.5) {
            // Make the dicrotic notch more pronounced and realistic
            double notchDepth = 0.2 * baseAmplitude; 
            double notchWidth = 0.06;
            double notchCenter = 0.42;
            
            // The notch is a negative gaussian on the downslope
            value -= notchDepth * exp(-pow((cyclePhase - notchCenter) / (notchWidth / 2.0), 2));
            
            // Add a small bump after the notch (dicrotic wave)
            double dicroticWaveCenter = notchCenter + notchWidth;
            if (cyclePhase > dicroticWaveCenter && cyclePhase < dicroticWaveCenter + 0.08) {
                value += 0.1 * baseAmplitude * exp(-pow((cyclePhase - (dicroticWaveCenter + 0.03)) / 0.04, 2));
            }
        }
        
        // Apply respiratory modulation
        value += respEffect * baseAmplitude;
        
        // Add very subtle pulsus paradoxus effect on inspiration (respiratory variation)
        if (respEffect < 0) {  // During "inspiration" phase
            value *= (1.0 + 0.05 * respEffect); // Reduce amplitude slightly
        }
        
        // Add some noise (minimal for clarity)
        if (noise_ > 0) {
            value += boundedDouble(random_, -noise_/3, noise_/3) * baseAmplitude;
        }
        
        // Apply final amplitude scaling
        value *= amplitudeMultiplier;
        
        result.append(value);
    }
    
    return result;
}

/**
 * @brief Generates arterial blood pressure waveform data
 * @param time Current time in seconds
 * @param points Number of points to generate
 * @return Vector of generated arterial pressure data points
 * 
 * Creates a realistic arterial blood pressure waveform with
 * appropriate systolic peak, dicrotic notch, and diastolic
 * decay based on current blood pressure values.
 */
QVector<float> DemoDataProvider::generateArterialPressure(double time, int points)
{
    QVector<float> result;
    result.reserve(points);
    
    // Calculate time step based on heart rate
    double cycleTime = 60.0 / heart_rate_;
    double timeStep = 1.0 / (1000.0 / waveform_update_interval_ms_);
    
    // Use the current systolic and diastolic values for the waveform range
    double systolic = systolic_bp_;
    double diastolic = diastolic_bp_;
    double pressure_range = systolic - diastolic;
    
    // Amplitude multiplier for better visualization
    double amplitudeMultiplier = 1.5;
    
    for (int i = 0; i < points; ++i) {
        double t = time + i * timeStep;
        
        // Normalize to cycle phase (0-1)
        double cyclePhase = fmod(t, cycleTime) / cycleTime;
        
        double value = 0.0;
        
        // Add respiratory variation (affects both baseline and amplitude)
        double respCycle = fmod(t, 60.0 / respiration_rate_) / (60.0 / respiration_rate_);
        double respEffect = 0.05 * sin(2.0 * M_PI * respCycle);
        
        // Base pressure starts at diastolic
        value = diastolic;
        
        // Systolic upstroke - faster rise (25% of cycle)
        if (cyclePhase < 0.15) {
            // Rapid arterial pressure rise with acceleration
            double normalizedPhase = cyclePhase / 0.15;
            double riseCurve = pow(normalizedPhase, 1.8) * (3 - 2 * normalizedPhase);
            value += pressure_range * riseCurve;
        }
        // Systolic peak and early decline (small plateau at peak)
        else if (cyclePhase < 0.2) {
            // Small plateau at peak with slight decay
            double normalizedPhase = (cyclePhase - 0.15) / 0.05;
            value += pressure_range * (1.0 - 0.05 * normalizedPhase);
        }
        // Initial rapid decline
        else if (cyclePhase < 0.3) {
            double normalizedPhase = (cyclePhase - 0.2) / 0.1;
            double declineFactor = 1.0 - normalizedPhase * 0.8;
            value += pressure_range * declineFactor;
        }
        // Dicrotic notch
        else if (cyclePhase < 0.4) {
            // Calculate position in the notch section
            double normalizedPhase = (cyclePhase - 0.3) / 0.1;
            
            // Base decline continues
            double basePressure = diastolic + pressure_range * 0.2 * (1.0 - normalizedPhase);
            
            // Dicrotic notch - more pronounced
            if (normalizedPhase < 0.5) {
                // Notch drop
                double notchDepth = pressure_range * 0.10;
                double notchProgress = normalizedPhase / 0.5;
                basePressure -= notchDepth * sin(notchProgress * M_PI);
            } else {
                // Rebound after notch (dicrotic wave)
                double reboundHeight = pressure_range * 0.08;
                double reboundProgress = (normalizedPhase - 0.5) / 0.5;
                basePressure += reboundHeight * sin(reboundProgress * M_PI);
            }
            
            value = basePressure;
        }
        // Diastolic decay - gradual decline to diastolic pressure
        else {
            double normalizedPhase = (cyclePhase - 0.4) / 0.6;
            double decayFactor = (1.0 - normalizedPhase) * (1.0 - normalizedPhase) * 0.28;
            value += pressure_range * decayFactor;
        }
        
        // Apply respiratory modulation to the waveform
        // Affects both the baseline and the amplitude slightly
        value += diastolic * respEffect;
        value += (pressure_range * respEffect * 0.3);
        
        // Add natural beat-to-beat variation (2-3% variation)
        if (cyclePhase < 0.05) {
            double beatVariation = boundedDouble(random_, -0.03, 0.03);
            systolic += beatVariation * systolic;
            diastolic += beatVariation * diastolic;
            
            // Keep within physiological constraints
            systolic = qBound(70.0, systolic, 200.0);
            diastolic = qBound(40.0, diastolic, 110.0);
            
            // Update range for next calculations
            pressure_range = systolic - diastolic;
        }
        
        // Add minimal noise
        if (noise_ > 0) {
            value += boundedDouble(random_, -noise_/4, noise_/4);
        }
        
        // Apply overall amplitude scaling
        value *= amplitudeMultiplier * amplitude_ / 100.0;
        
        result.append(value);
    }
    
    return result;
}

/**
 * @brief Generates capnograph (CO2) waveform data
 * @param time Current time in seconds
 * @param points Number of points to generate
 * @return Vector of generated capnograph data points
 * 
 * Creates a realistic capnograph waveform showing the 
 * characteristic CO2 levels during the respiratory cycle,
 * with baseline, rapid rise, plateau, and rapid fall.
 */
QVector<float> DemoDataProvider::generateCapnograph(double time, int points)
{
    QVector<float> result;
    result.reserve(points);
    
    // Calculate time step based on respiration rate (in seconds)
    double cycleTime = 60.0 / respiration_rate_;
    double timeStep = 1.0 / (1000.0 / waveform_update_interval_ms_);
    
    // Make the capnograph more visually pronounced
    double maxCO2 = etco2_ / 50.0; // Scale value for display
    
    for (int i = 0; i < points; ++i) {
        double t = time + i * timeStep;
        
        // Normalize to cycle phase (0-1)
        double cyclePhase = fmod(t, cycleTime) / cycleTime;
        
        // CO2 value
        double value = 0.0;
        
        if (cyclePhase < CAPNO_INSPIRATION_END) {
            // Baseline (inspiration) - keep at zero
            value = 0.0;
        }
        else if (cyclePhase < CAPNO_PLATEAU_START) {
            // Rapid rise - exponential for more realistic appearance
            double normalizedPhase = (cyclePhase - CAPNO_INSPIRATION_END) / 
                                   (CAPNO_PLATEAU_START - CAPNO_INSPIRATION_END);
            // More dramatic and rapid rise using exponential curve
            value = maxCO2 * (1.0 - exp(-5.0 * normalizedPhase));
        }
        else if (cyclePhase < CAPNO_PLATEAU_END) {
            // Alveolar plateau with slightly increasing slope - more realistic
            double normalizedPhase = (cyclePhase - CAPNO_PLATEAU_START) / 
                                    (CAPNO_PLATEAU_END - CAPNO_PLATEAU_START);
            // Slight undulation in the plateau for realism
            value = maxCO2 * (1.0 + 0.05 * normalizedPhase + 0.02 * sin(normalizedPhase * 3.0 * M_PI));
        }
        else if (cyclePhase < CAPNO_EXPIRATION_END) {
            // Rapid fall - exponential curve for realism
            double normalizedPhase = (cyclePhase - CAPNO_PLATEAU_END) / 
                                    (CAPNO_EXPIRATION_END - CAPNO_PLATEAU_END);
            // Exponential decay creates more realistic falling edge
            value = maxCO2 * (1.05 * exp(-3.0 * normalizedPhase));
        }
        else {
            // Baseline - maintain zero but add tiny fluctuations for realism
            value = maxCO2 * 0.02 * sin(cyclePhase * 10.0 * M_PI); // Tiny fluctuations
        }
        
        // Add noise
        if (noise_ > 0) {
            value += boundedDouble(random_, -noise_ * maxCO2 * 0.05, noise_ * maxCO2 * 0.05);
        }
        
        // Amplify overall for better display
        value *= 1.5;
        
        result.append(value);
    }
    
    return result;
}

/**
 * @brief Adds natural variation to a physiological parameter
 * @param baseValue The base value of the parameter
 * @param variationPct The percentage of variation to apply (0-1)
 * @return The parameter value with natural variation added
 * 
 * Applies random variation to a physiological parameter to simulate
 * natural fluctuations in real patient data.
 */
double DemoDataProvider::addVariation(double base_value, double variation_pct) {
  // Google style: Always use braces around if statements
  if (variation_pct < 0.0 || variation_pct > 1.0) {
    qWarning() << "Invalid variation percentage (must be 0-1), clamping to valid range";
    variation_pct = std::clamp(variation_pct, 0.0, 1.0);
  }

  // Generate random variation as a percentage of the base value
  double max_variation = base_value * variation_pct;
  
  // Use normal distribution for more natural variations
  // Google style: Use intermediate variables to improve readability
  double random_factor = random_.generateDouble() * 2.0 - 1.0;  // Range -1.0 to 1.0
  double variation = random_factor * max_variation;
  
  // Return base value with random variation
  return base_value + variation;
} 