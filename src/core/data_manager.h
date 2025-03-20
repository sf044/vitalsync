/**
 * @file data_manager.h
 * @brief Definition of the DataManager class
 *
 * This file contains the definition of the DataManager class which
 * serves as the central data coordination component.
 * The DataManager manages data providers (sources of physiological
 * data), waveform models (for ECG, respiration, etc.), and parameter models
 * (for heart rate, blood pressure, etc.). It handles data routing between
 * providers and models, and provides access to all data models for the UI layer.
 */
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "../../include/i_data_manager.h"
#include "../../include/i_data_provider.h"
#include "../../include/i_waveform_model.h"
#include "../../include/i_parameter_model.h"
#include "../../include/vital_sync_types.h"
#include <QObject>
#include <QMap>
#include <QMutex>
#include <memory>
#include <vector>
#include <string>

/**
 * @brief Implementation of the IDataManager interface
 * 
 * This class provides the central data coordination for the application. It manages
 * data providers and models, routes data between them, and handles lifecycle events.
 */
class DataManager : public IDataManager {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit DataManager(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~DataManager() override;

    /**
     * @brief Initialize the data manager
     * @return True if initialization was successful
     */
    bool initialize() override;

    /**
     * @brief Start data acquisition using the current provider
     * @return True if successfully started
     */
    bool startAcquisition() override;

    /**
     * @brief Stop data acquisition
     */
    void stopAcquisition() override;

    /**
     * @brief Get the list of available data providers
     * @return List of available data provider names
     */
    std::vector<std::string> GetAvailableProviders() const override;

    /**
     * @brief Get the current active data provider
     * @return Pointer to the current data provider or nullptr if none
     */
    std::shared_ptr<IDataProvider> GetCurrentProvider() const override;

    /**
     * @brief Set the active data provider by name
     * @param providerName Name of the provider to activate
     * @return True if provider was successfully activated
     */
    bool SetActiveProvider(const std::string& providerName) override;

    /**
     * @brief Configure the current provider
     * @param params Provider-specific configuration parameters
     * @return True if configuration was successful
     */
    bool configureCurrentProvider(const QVariantMap& params) override;

    /**
     * @brief Get a waveform model by ID
     * @param waveformId ID of the waveform model to retrieve
     * @return Shared pointer to the waveform model or nullptr if not found
     */
    std::shared_ptr<IWaveformModel> GetWaveformModel(int waveformId) const override;

    /**
     * @brief Get all available waveform models
     * @return Vector of waveform model pointers
     */
    std::vector<std::shared_ptr<IWaveformModel>> GetAllWaveformModels() const override;

    /**
     * @brief Get a parameter model by ID
     * @param parameterId ID of the parameter model to retrieve
     * @return Shared pointer to the parameter model or nullptr if not found
     */
    std::shared_ptr<IParameterModel> GetParameterModel(int parameterId) const override;

    /**
     * @brief Get all available parameter models
     * @return Vector of parameter model pointers
     */
    std::vector<std::shared_ptr<IParameterModel>> GetAllParameterModels() const override;

private slots:
    /**
     * @brief Handle waveform data received from a provider
     * @param waveformType Type of waveform
     * @param timestamp Timestamp of the data
     * @param data Waveform data points
     */
    void HandleWaveformData(int waveformType, qint64 timestamp, const QVector<float>& data);

    /**
     * @brief Handle parameter data received from a provider
     * @param parameterType Type of parameter
     * @param timestamp Timestamp of the data
     * @param value Parameter value
     */
    void HandleParameterData(int parameterType, qint64 timestamp, float value);

    /**
     * @brief Handle provider connection status changes
     * @param status New connection status
     */
    void HandleConnectionStatusChanged(ConnectionStatus status);

    /**
     * @brief Handle provider errors
     * @param errorCode Error code
     * @param errorMessage Error message
     */
    void HandleProviderError(int errorCode, const QString& errorMessage);

private:
    /**
     * @brief Register an available data provider
     * @param provider Shared pointer to the provider
     */
    void registerProvider(std::shared_ptr<IDataProvider> provider);

    /**
     * @brief Initialize waveform models
     */
    void initializeWaveformModels();

    /**
     * @brief Initialize parameter models
     */
    void initializeParameterModels();

    /**
     * @brief Connect signals from the current provider to the data manager slots
     */
    void connectProviderSignals();

    /**
     * @brief Disconnect signals from the current provider to the data manager slots
     */
    void disconnectProviderSignals();

    /**
     * @brief Create all available data providers
     */
    void createProviders();

    /**
     * @brief Create all data models
     */
    void createModels();

    /**
     * @brief Save the current provider selection to settings
     */
    void saveCurrentProviderToSettings();

private:
    // Available providers (mapped by name)
    QMap<std::string, std::shared_ptr<IDataProvider>> providers_;  ///< Map of available data providers by provider name

    // Current provider
    std::shared_ptr<IDataProvider> current_provider_;  ///< Currently active data provider

    // Waveform models (mapped by type)
    QMap<int, std::shared_ptr<IWaveformModel>> waveform_models_;  ///< Map of waveform models by waveform type ID

    // Parameter models (mapped by type)
    QMap<int, std::shared_ptr<IParameterModel>> parameter_models_;  ///< Map of parameter models by parameter type ID

    // Thread safety
    mutable QMutex mutex_;  ///< Mutex for thread-safe access to manager state
};

#endif // DATA_MANAGER_H 