/**
 * @file i_data_manager.h
 * @brief Interface for the data manager
 * 
 * IDataManager interface, serves as the contract
 * for the central data coordination component.
 * The data manager is responsible for managing data providers (sources of physiological
 * data), waveform models (for ECG, respiration, etc.), and parameter models
 * (for heart rate, blood pressure, etc.). It handles provider switching, configuration,
 * data acquisition, and routing data between providers and models.
 */

#ifndef I_DATA_MANAGER_H
#define I_DATA_MANAGER_H

#include <QObject>
#include <memory>
#include <vector>
#include <string>

#include "i_data_provider.h"
#include "i_waveform_model.h"
#include "i_parameter_model.h"

using namespace VitalSync;

/**
 * @brief Interface for the central data coordination component
 * 
 * The IDataManager interface defines the contract for the central data manager
 * component. It is responsible for:
 * 
 * - Managing the lifecycle of data providers (demo, network, file)
 * - Coordinating data acquisition and provider states
 * - Routing data from providers to appropriate waveform and parameter models
 * - Providing access to all waveform and parameter models for the UI layer
 * - Handling provider switching and configuration
 * - Emitting signals for connection status changes and error handling
 * 
 * Implementations of this interface serve as the bridge between data sources
 * (providers) and data consumers (UI components showing waveforms and parameters).
 */
class IDataManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent Parent QObject for memory management
     */
    explicit IDataManager(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Virtual destructor for proper inheritance cleanup
     */
    virtual ~IDataManager() = default;

    /**
     * @brief Initialize the data manager
     * @return True if initialization was successful
     * 
     * Sets up the data manager by creating data providers, initializing waveform
     * and parameter models, and attempting to restore the last used provider
     * from configuration. This method must be called before any other methods
     * can be used successfully.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Start data acquisition using the current provider
     * @return True if successfully started
     * 
     * Activates the current data provider to begin data acquisition.
     * This will cause the provider to begin sending waveform and parameter
     * data, which will update the connected models. Returns false if
     * no provider is active or if the provider fails to start.
     */
    virtual bool startAcquisition() = 0;

    /**
     * @brief Stop data acquisition
     * 
     * Stops the current provider from generating or receiving data.
     * This method is safe to call even if no provider is active or if
     * acquisition has not been started.
     */
    virtual void stopAcquisition() = 0;

    /**
     * @brief Get the list of available data providers
     * @return List of available data provider names
     * 
     * Returns a list of names for all registered data providers that can be
     * used as sources of physiological data. These names can be used with
     * the SetActiveProvider method to activate a specific provider.
     */
    virtual std::vector<std::string> GetAvailableProviders() const = 0;

    /**
     * @brief Get the current active data provider
     * @return Pointer to the current data provider or nullptr if none
     * 
     * Returns a shared pointer to the currently active data provider,
     * which is the source of physiological data used by the application.
     * Returns nullptr if no provider is currently active.
     */
    virtual std::shared_ptr<IDataProvider> GetCurrentProvider() const = 0;

    /**
     * @brief Set the active data provider by name
     * @param providerName Name of the provider to activate
     * @return True if provider was successfully activated
     * 
     * Switches the active provider to the one specified by name.
     * If a provider is already active, it is stopped first. Signals are
     * disconnected from the old provider and connected to the new one.
     * Returns false if the specified provider name is not found.
     */
    virtual bool SetActiveProvider(const std::string& providerName) = 0;

    /**
     * @brief Configure the current provider
     * @param params Provider-specific configuration parameters
     * @return True if configuration was successful
     * 
     * Passes configuration parameters to the current provider to modify
     * its behavior. The specific parameters depend on the provider type:
     * - Demo provider: waveform type, update interval, etc.
     * - Network provider: host, port, etc.
     * - File provider: file path, playback speed, etc.
     * 
     * Returns false if no provider is active or if the configuration is invalid.
     */
    virtual bool configureCurrentProvider(const QVariantMap& params) = 0;

    /**
     * @brief Get a waveform model by ID
     * @param waveformId ID of the waveform model to retrieve
     * @return Shared pointer to the waveform model or nullptr if not found
     * 
     * Retrieves a specific waveform model (ECG, respiration, etc.) by its
     * numeric identifier, which corresponds to a VitalSync::WaveformType value.
     * Returns nullptr if the specified waveform ID is not found.
     */
    virtual std::shared_ptr<IWaveformModel> GetWaveformModel(int waveformId) const = 0;

    /**
     * @brief Get all available waveform models
     * @return Vector of waveform model pointers
     * 
     * Returns a collection of all waveform models managed by the
     * data manager, regardless of their current state. This can be
     * used to iterate through all available waveforms for display
     * or configuration purposes.
     */
    virtual std::vector<std::shared_ptr<IWaveformModel>> GetAllWaveformModels() const = 0;

    /**
     * @brief Get a parameter model by ID
     * @param parameterId ID of the parameter model to retrieve
     * @return Shared pointer to the parameter model or nullptr if not found
     * 
     * Retrieves a specific parameter model (heart rate, blood pressure, etc.) by its
     * numeric identifier, which corresponds to a VitalSync::ParameterType value.
     * Returns nullptr if the specified parameter ID is not found.
     */
    virtual std::shared_ptr<IParameterModel> GetParameterModel(int parameterId) const = 0;

    /**
     * @brief Get all available parameter models
     * @return Vector of parameter model pointers
     * 
     * Returns a collection of all parameter models managed by the
     * data manager, regardless of their current state. This can be
     * used to iterate through all available parameters for display
     * or configuration purposes.
     */
    virtual std::vector<std::shared_ptr<IParameterModel>> GetAllParameterModels() const = 0;

signals:
    /**
     * @brief Signal emitted when the active provider changes
     * @param providerName Name of the new active provider
     * 
     * This signal is emitted whenever the active provider is changed through
     * the SetActiveProvider method. UI components can connect to this signal
     * to update their display or behavior based on the new provider.
     */
    void activeProviderChanged(const std::string& providerName);

    /**
     * @brief Signal emitted when the connection status changes
     * @param status New connection status
     * 
     * This signal is emitted whenever the connection status of the active
     * provider changes. This can happen when a provider connects to a data
     * source, disconnects, or encounters connection issues. UI components
     * can connect to this signal to display the current connection state.
     */
    void connectionStatusChanged(ConnectionStatus status);

    /**
     * @brief Signal emitted when an error occurs
     * @param errorCode Error code
     * @param errorMessage Human-readable error message
     * 
     * This signal is emitted when an error occurs in the data manager or
     * the active provider. UI components can connect to this signal to
     * display error messages to the user or take corrective action.
     */
    void errorOccurred(int errorCode, const QString& errorMessage);
};

#endif // I_DATA_MANAGER_H 
