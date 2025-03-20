/**
 * @file data_manager.cpp
 * @brief Implementation of the DataManager class
 *
 * This file implements the DataManager class that serves as the central
 * coordination point for all physiological data.
 * It manages data providers (sources of data), routes data to the appropriate models,
 * handles provider switching and configuration, and maintains the collection of
 * waveform and parameter models that represent the patient's physiological state.
 */
#include "data_manager.h"
#include "waveform_model.h"
#include "parameter_model.h"
#include "../providers/demo_data_provider.h"
#include "../../include/config_manager.h"
#include <QDebug>

/**
 * @brief Constructs a DataManager instance
 * @param parent The parent QObject for memory management
 * 
 * Initializes an empty DataManager without any providers or models.
 * The initialize method must be called to set up providers and models.
 */
DataManager::DataManager(QObject* parent)
    : IDataManager(parent)
{
}

/**
 * @brief Destroys the DataManager instance
 * 
 * Stops any active data acquisition and disconnects provider signals
 * before destroying the manager.
 */
DataManager::~DataManager()
{
    // Stop acquisition if active
    stopAcquisition();
    
    // Disconnect current provider signals
    if (current_provider_) {
        disconnectProviderSignals();
    }
}

/**
 * @brief Initializes the DataManager with providers and models
 * @return True if initialization was successful, false otherwise
 * 
 * Creates and registers data providers, initializes waveform and parameter models,
 * and attempts to restore the last used provider from configuration.
 */
bool DataManager::initialize()
{
    try {
        // Initialize the available providers
        auto demoProvider = std::make_shared<DemoDataProvider>(this);
        registerProvider(demoProvider);
        
        // Initialize waveform models
        initializeWaveformModels();
        
        // Initialize parameter models
        initializeParameterModels();
        
        // Set the default provider from configuration
        auto& config = ConfigManager::GetInstance();
        std::string lastProvider = config.GetLastProvider().toStdString();
        
        if (!lastProvider.empty()) {
            SetActiveProvider(lastProvider);
        } else if (!providers_.empty()) {
            // Use the first available provider if no last provider is set
            SetActiveProvider(providers_.begin().key());
        }
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "Failed to initialize DataManager: " << e.what();
        return false;
    }
}

/**
 * @brief Starts data acquisition using the current provider
 * @return True if acquisition was successfully started, false otherwise
 * 
 * Activates the current data provider to begin generating or receiving
 * physiological data. Ensures all parameter models are activated to
 * receive the incoming data. Fails if no provider is active.
 * 
 * This method may emit the errorOccurred signal if there is no active provider,
 * and indirectly causes the connection status and data signals to be emitted
 * from the active provider as data acquisition begins.
 */
bool DataManager::startAcquisition()
{
    QMutexLocker locker(&mutex_);
    
    qDebug() << "DataManager: Starting data acquisition...";
    
    if (!current_provider_) {
        qWarning() << "Cannot start acquisition: No active provider";
        emit errorOccurred(static_cast<int>(VitalSync::ErrorCode::ConfigurationError),
                        "Cannot start acquisition: No active provider");
        return false;
    }
    
    // Check if the provider is already active
    if (current_provider_->isActive()) {
        qDebug() << "DataManager: Provider already active, restarting it";
        current_provider_->stop();
    }
    
    // Force active state for all parameter models
    for (auto& model : parameter_models_) {
        if (!model->isActive()) {
            qDebug() << "DataManager: Activating parameter model" << model->GetDisplayName();
            model->SetActive(true);
        }
    }
    
    // Start the provider
    qDebug() << "DataManager: Starting provider:" << QString::fromStdString(current_provider_->GetName());
    bool success = current_provider_->start();
    
    if (success) {
        qDebug() << "DataManager: Provider started successfully";
    } else {
        qDebug() << "DataManager: Failed to start provider";
    }
    
    return success;
}

/**
 * @brief Stops data acquisition
 * 
 * Stops the current provider from generating or receiving data.
 * This method is safe to call even if no provider is active.
 */
void DataManager::stopAcquisition()
{
    QMutexLocker locker(&mutex_);
    
    if (current_provider_) {
        current_provider_->stop();
    }
}

/**
 * @brief Gets a list of available data provider names
 * @return Vector of provider name strings
 * 
 * Returns the names of all registered data providers that can be
 * used as sources of physiological data.
 */
std::vector<std::string> DataManager::GetAvailableProviders() const
{
    QMutexLocker locker(&mutex_);
    
    std::vector<std::string> result;
    for (auto it = providers_.begin(); it != providers_.end(); ++it) {
        result.push_back(it.key());
    }
    
    return result;
}

/**
 * @brief Gets the currently active data provider
 * @return Shared pointer to the current provider, or nullptr if none
 * 
 * Returns a pointer to the currently active data provider, which is the
 * source of physiological data used by the application.
 */
std::shared_ptr<IDataProvider> DataManager::GetCurrentProvider() const
{
    QMutexLocker locker(&mutex_);
    return current_provider_;
}

/**
 * @brief Sets the active data provider by name
 * @param providerName Name of the provider to activate
 * @return True if provider was successfully activated, false otherwise
 * 
 * Switches the active provider to the one specified by name. If a provider
 * is already active, it is stopped first. Signals are disconnected from the
 * old provider and connected to the new one.
 */
bool DataManager::SetActiveProvider(const std::string& providerName)
{
    QMutexLocker locker(&mutex_);
    
    if (providerName == "" && current_provider_) {
        // Deactivate current provider
        disconnectProviderSignals();
        current_provider_->stop();
        current_provider_ = nullptr;
        
        // Save the setting
        saveCurrentProviderToSettings();
        
        // Emit signal to inform UI and other components about provider change
        emit activeProviderChanged(providerName);
        
        return true;
    }
    
    if (!providers_.contains(providerName)) {
        qWarning() << "Unknown provider: " << QString::fromStdString(providerName);
        return false;
    }
    
    // First, stop and disconnect old provider
    if (current_provider_) {
        disconnectProviderSignals();
        current_provider_->stop();
    }
    
    // Activate new provider
    current_provider_ = providers_[providerName];
    
    // Connect new provider signals
    connectProviderSignals();
    
    // Save the provider name in configuration
    ConfigManager::GetInstance().SetLastProvider(QString::fromStdString(providerName));
    
    // Emit signal to inform UI and other components about provider change
    emit activeProviderChanged(providerName);
    
    return true;
}

/**
 * @brief Configures the current data provider with new settings
 * @param params Map of configuration parameters
 * @return True if configuration was successful, false otherwise
 * 
 * Passes configuration parameters to the current provider to
 * modify its behavior. The specific parameters depend on the
 * provider type.
 */
bool DataManager::configureCurrentProvider(const QVariantMap& params)
{
    QMutexLocker locker(&mutex_);
    
    if (!current_provider_) {
        qWarning() << "Cannot configure provider: No active provider";
        return false;
    }
    
    return current_provider_->configure(params);
}

/**
 * @brief Gets a specific waveform model by ID
 * @param waveformId The ID of the waveform model to retrieve
 * @return Shared pointer to the waveform model, or nullptr if not found
 * 
 * Retrieves a specific waveform model (ECG, respiration, etc.) by its
 * numeric identifier, which corresponds to a VitalSync::WaveformType value.
 */
std::shared_ptr<IWaveformModel> DataManager::GetWaveformModel(int waveformId) const
{
    QMutexLocker locker(&mutex_);
    
    if (waveform_models_.contains(waveformId)) {
        return waveform_models_[waveformId];
    }
    
    return nullptr;
}

/**
 * @brief Gets all available waveform models
 * @return Vector containing all waveform model pointers
 * 
 * Returns a collection of all waveform models managed by the
 * DataManager, regardless of their current state.
 */
std::vector<std::shared_ptr<IWaveformModel>> DataManager::GetAllWaveformModels() const
{
    QMutexLocker locker(&mutex_);
    
    std::vector<std::shared_ptr<IWaveformModel>> result;
    for (auto it = waveform_models_.begin(); it != waveform_models_.end(); ++it) {
        result.push_back(it.value());
    }
    
    return result;
}

/**
 * @brief Gets a specific parameter model by ID
 * @param parameterId The ID of the parameter model to retrieve
 * @return Shared pointer to the parameter model, or nullptr if not found
 * 
 * Retrieves a specific parameter model (heart rate, blood pressure, etc.) by its
 * numeric identifier, which corresponds to a VitalSync::ParameterType value.
 */
std::shared_ptr<IParameterModel> DataManager::GetParameterModel(int parameterId) const
{
    QMutexLocker locker(&mutex_);
    
    if (parameter_models_.contains(parameterId)) {
        return parameter_models_[parameterId];
    }
    
    return nullptr;
}

/**
 * @brief Gets all available parameter models
 * @return Vector containing all parameter model pointers
 * 
 * Returns a collection of all parameter models managed by the
 * DataManager, regardless of their current state.
 */
std::vector<std::shared_ptr<IParameterModel>> DataManager::GetAllParameterModels() const
{
    QMutexLocker locker(&mutex_);
    
    std::vector<std::shared_ptr<IParameterModel>> result;
    for (auto it = parameter_models_.begin(); it != parameter_models_.end(); ++it) {
        result.push_back(it.value());
    }
    
    return result;
}

/**
 * @brief Handles incoming waveform data from the active provider
 * @param waveformType The type of waveform data (ECG, respiration, etc.)
 * @param timestamp Timestamp in milliseconds when the data was captured
 * @param data Vector of floating-point values representing the waveform samples
 * 
 * This slot receives waveform data from the active provider and routes it
 * to the appropriate waveform model for processing and display.
 */
void DataManager::HandleWaveformData(int waveformType, qint64 timestamp, const QVector<float>& data)
{
    // Get the waveform model for this type
    auto model = GetWaveformModel(waveformType);
    
    if (model && model->isActive()) {
        // Update the model with the new data
        model->addWaveformData(timestamp, data);
    }
}

/**
 * @brief Handles incoming parameter data from the active provider
 * @param parameterType The type of parameter data (heart rate, SpO2, etc.)
 * @param timestamp Timestamp in milliseconds when the data was captured
 * @param value The parameter value
 * 
 * This slot receives parameter data from the active provider and routes it
 * to the appropriate parameter model for processing and display.
 */
void DataManager::HandleParameterData(int parameterType, qint64 timestamp, float value)
{
    // Get the parameter model for this type
    auto model = GetParameterModel(parameterType);
    
    if (model && model->isActive()) {
        // Debug output
        qDebug() << "DataManager: Updating parameter" << model->GetDisplayName() 
                 << "with value" << value << model->GetUnit()
                 << "at timestamp" << QDateTime::fromMSecsSinceEpoch(timestamp).toString("hh:mm:ss.zzz");
        
        // Update the model with the new value
        model->UpdateValue(timestamp, value);
    } else if (model) {
        qDebug() << "DataManager: Parameter" << model->GetDisplayName() << "is inactive, not updating";
    } else {
        qDebug() << "DataManager: No model found for parameter type" << parameterType;
    }
}

/**
 * @brief Handles connection status changes from the active provider
 * @param status The new connection status
 * 
 * This slot receives connection status updates from the active provider and
 * forwards them to the application via the connectionStatusChanged signal.
 */
void DataManager::HandleConnectionStatusChanged(ConnectionStatus status)
{
    // Forward the status change to UI and other components
    emit connectionStatusChanged(status);
}

/**
 * @brief Handles error conditions reported by the active provider
 * @param errorCode Numeric identifier for the error type
 * @param errorMessage Human-readable description of the error
 * 
 * This slot receives error reports from the active provider and forwards
 * them to the application via the errorOccurred signal for display to the user.
 */
void DataManager::HandleProviderError(int errorCode, const QString& errorMessage)
{
    // Forward the error to UI and other components
    emit errorOccurred(errorCode, errorMessage);
}

/**
 * @brief Registers a data provider in the providers collection
 * @param provider Shared pointer to the provider instance to register
 * 
 * Adds the provided data provider to the providers_ map, indexed by its name.
 * This makes the provider available for use in the application.
 */
void DataManager::registerProvider(std::shared_ptr<IDataProvider> provider)
{
    if (!provider) {
        return;
    }
    
    std::string name = provider->GetName();
    providers_[name] = provider;
    
    qDebug() << "Registered provider: " << QString::fromStdString(name);
}

/**
 * @brief Initializes waveform models for all supported waveform types
 * 
 * Creates a WaveformModel instance for each supported waveform type
 * (ECG, respiration, etc.) and stores it in the waveform_models_ map
 * indexed by its numeric type identifier.
 */
void DataManager::initializeWaveformModels()
{
    // Create models for all waveform types
    for (int i = 0; i < 13; ++i) {
        VitalSync::WaveformType type = static_cast<VitalSync::WaveformType>(i);
        auto model = std::make_shared<WaveformModel>(type, this);
        waveform_models_[i] = model;
    }
}

/**
 * @brief Initializes parameter models for all supported parameter types
 * 
 * Creates a ParameterModel instance for each supported parameter type
 * (heart rate, blood pressure, etc.) and stores it in the parameter_models_
 * map indexed by its numeric type identifier.
 */
void DataManager::initializeParameterModels()
{
    // Create models for all parameter types
    for (int i = 0; i < 18; ++i) {
        VitalSync::ParameterType type = static_cast<VitalSync::ParameterType>(i);
        auto model = std::make_shared<ParameterModel>(type, this);
        parameter_models_[i] = model;
    }
}

/**
 * @brief Connects signals from the current provider to data manager slots
 * 
 * Establishes signal-slot connections between the current data provider
 * and the DataManager to enable data flow from the provider to the models.
 * This includes connections for waveform data, parameter data, connection
 * status updates, and error reports.
 */
void DataManager::connectProviderSignals()
{
    if (!current_provider_) {
        return;
    }
    
    // Connect signals from provider to data manager slots
    connect(current_provider_.get(), &IDataProvider::waveformDataReceived,
            this, &DataManager::HandleWaveformData);
    
    connect(current_provider_.get(), &IDataProvider::parameterDataReceived,
            this, &DataManager::HandleParameterData);
    
    connect(current_provider_.get(), &IDataProvider::connectionStatusChanged,
            this, &DataManager::HandleConnectionStatusChanged);
    
    connect(current_provider_.get(), &IDataProvider::errorOccurred,
            this, &DataManager::HandleProviderError);
}

/**
 * @brief Disconnects signals from the current provider
 * 
 * Removes signal-slot connections between the current data provider and
 * the DataManager. This is called when changing providers or shutting down
 * to prevent dangling signal connections.
 */
void DataManager::disconnectProviderSignals()
{
    if (!current_provider_) {
        return;
    }
    
    // Disconnect signals from provider to data manager slots
    disconnect(current_provider_.get(), &IDataProvider::waveformDataReceived,
               this, &DataManager::HandleWaveformData);
    
    disconnect(current_provider_.get(), &IDataProvider::parameterDataReceived,
               this, &DataManager::HandleParameterData);
    
    disconnect(current_provider_.get(), &IDataProvider::connectionStatusChanged,
               this, &DataManager::HandleConnectionStatusChanged);
    
    disconnect(current_provider_.get(), &IDataProvider::errorOccurred,
               this, &DataManager::HandleProviderError);
}

/**
 * @brief Creates and initializes data providers
 * 
 * Instantiates all supported data provider types (Demo, Network, File)
 * and registers them in the providers_ map by name. Each provider is
 * initialized with a connection to the appropriate signal handler.
 */
void DataManager::createProviders()
{
    // Create the demo provider (always available)
    auto demoProvider = std::make_shared<DemoDataProvider>();
    registerProvider(demoProvider);
    
    // Create the network provider
    // TODO: Implement NetworkDataProvider
    // auto networkProvider = std::make_shared<NetworkDataProvider>();
    // registerProvider(networkProvider);
    
    // Create the file provider
    // TODO: Implement FileDataProvider
    // auto fileProvider = std::make_shared<FileDataProvider>();
    // registerProvider(fileProvider);
}

/**
 * @brief Creates and initializes waveform and parameter models
 * 
 * Instantiates models for all supported waveforms (ECG, respiration, etc.) and
 * physiological parameters (heart rate, blood pressure, etc.). These models
 * will be updated with data from the active provider during acquisition.
 */
void DataManager::createModels()
{
    // Initialize the waveform and parameter models
    initializeWaveformModels();
    initializeParameterModels();
}

/**
 * @brief Saves the currently active provider name to application settings
 * 
 * Persists the name of the currently active provider in the application's
 * QSettings storage, allowing it to be restored on subsequent application
 * startups.
 */
void DataManager::saveCurrentProviderToSettings()
{
    if (current_provider_) {
        ConfigManager::GetInstance().SetLastProvider(
            QString::fromStdString(current_provider_->GetName()));
    }
} 