/**
 * @file config_manager.cpp
 * @brief Implementation of the ConfigManager class
 *
 * This file implements the ConfigManager class which manages application settings
 * and configuration data. It provides a central singleton instance for accessing
 * and modifying application settings, including provider configurations, waveform
 * settings, parameter configurations, and default display options. The ConfigManager
 * handles persistence of settings between application sessions using Qt's QSettings
 * framework and emits signals when settings are changed.
 */
#include "../../include/config_manager.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

// Singleton instance
static ConfigManager* s_instance = nullptr;

// Constants for default values
namespace {
    // Common setting keys
    const QString KEY_LAST_PROVIDER = "lastProvider";
    const QString KEY_DEFAULT_SWEEP_SPEED = "defaultSweepSpeed";
    const QString KEY_DEFAULT_GRID_COLOR = "defaultGridColor";
    const QString KEY_DEFAULT_BG_COLOR = "defaultBackgroundColor";
    
    // Default values
    const QString DEFAULT_PROVIDER = "Demo";
    const double DEFAULT_SWEEP_SPEED = VitalSync::DEFAULT_SWEEP_SPEED;
    const QColor DEFAULT_GRID_COLOR = QColor(0, 128, 0);  // Green
    const QColor DEFAULT_BG_COLOR = QColor(0, 0, 0);      // Black
}

/**
 * @brief Private constructor for singleton pattern
 * 
 * Initializes the configuration manager with empty maps for provider,
 * waveform, and parameter configurations. Sets the dirty flag to false
 * to indicate no unsaved changes initially.
 */
ConfigManager::ConfigManager()
    : dirty_(false)
{
    // Initialize maps
    provider_configs_ = QMap<QString, QVariantMap>();
    waveform_configs_ = QMap<int, QVariantMap>();
    parameter_configs_ = QMap<int, QVariantMap>();
}

/**
 * @brief Destructor
 * 
 * Saves any unsaved configuration changes to persistent storage
 * before destroying the configuration manager instance.
 */
ConfigManager::~ConfigManager()
{
    // Save if dirty
    if (dirty_) {
        save();
    }
}

/**
 * @brief Gets the singleton instance of the ConfigManager
 * @return Reference to the singleton ConfigManager instance
 * 
 * Creates the ConfigManager instance if it doesn't exist yet,
 * following the Singleton design pattern.
 */
ConfigManager& ConfigManager::GetInstance()
{
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    
    return *s_instance;
}

/**
 * @brief Initializes the configuration manager
 * @param organization Organization name used for settings storage
 * @param application Application name used for settings storage
 * @return True if initialization was successful, false otherwise
 * 
 * Creates the QSettings object with the provided organization and application names.
 * Loads previously saved provider, waveform, and parameter configurations from
 * persistent storage into memory for access during the application session.
 */
bool ConfigManager::Initialize(const QString& organization, const QString& application)
{
    try {
        // Create settings object
        settings_ = std::make_unique<QSettings>(organization, application);
        
        // Load provider configurations
        int providerCount = settings_->beginReadArray("Providers");
        for (int i = 0; i < providerCount; ++i) {
            settings_->setArrayIndex(i);
            QString name = settings_->value("Name").toString();
            provider_configs_[name] = settings_->value("Config").toMap();
        }
        settings_->endArray();
        
        // Load waveform configurations
        int waveformCount = settings_->beginReadArray("Waveforms");
        for (int i = 0; i < waveformCount; ++i) {
            settings_->setArrayIndex(i);
            int type = settings_->value("Type").toInt();
            waveform_configs_[type] = settings_->value("Config").toMap();
        }
        settings_->endArray();
        
        // Load parameter configurations
        int parameterCount = settings_->beginReadArray("Parameters");
        for (int i = 0; i < parameterCount; ++i) {
            settings_->setArrayIndex(i);
            int type = settings_->value("Type").toInt();
            parameter_configs_[type] = settings_->value("Config").toMap();
        }
        settings_->endArray();
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "Failed to initialize ConfigManager: " << e.what();
        return false;
    }
}

/**
 * @brief Saves the current configuration to persistent storage
 * @return True if the save operation was successful, false otherwise
 * 
 * Writes all provider, waveform, and parameter configurations to persistent
 * storage using the QSettings framework. Updates the dirty flag to indicate
 * all changes have been saved.
 */
bool ConfigManager::save()
{
    try {
        // Save provider configurations
        settings_->beginWriteArray("Providers", provider_configs_.size());
        int index = 0;
        for (auto it = provider_configs_.begin(); it != provider_configs_.end(); ++it, ++index) {
            settings_->setArrayIndex(index);
            settings_->setValue("Name", it.key());
            settings_->setValue("Config", it.value());
        }
        settings_->endArray();
        
        // Save waveform configurations
        settings_->beginWriteArray("Waveforms", waveform_configs_.size());
        index = 0;
        for (auto it = waveform_configs_.begin(); it != waveform_configs_.end(); ++it, ++index) {
            settings_->setArrayIndex(index);
            settings_->setValue("Type", it.key());
            settings_->setValue("Config", it.value());
        }
        settings_->endArray();
        
        // Save parameter configurations
        settings_->beginWriteArray("Parameters", parameter_configs_.size());
        index = 0;
        for (auto it = parameter_configs_.begin(); it != parameter_configs_.end(); ++it, ++index) {
            settings_->setArrayIndex(index);
            settings_->setValue("Type", it.key());
            settings_->setValue("Config", it.value());
        }
        settings_->endArray();
        
        // Sync to disk
        settings_->sync();
        dirty_ = false;
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "Failed to save ConfigManager: " << e.what();
        return false;
    }
}

/**
 * @brief Resets all settings to default values
 * 
 * Clears all existing configurations and sets default values for provider,
 * waveform, and parameter settings. Activates a subset of waveforms and parameters
 * by default. Emits the settingsChanged signal to notify listeners of the reset.
 */
void ConfigManager::resetToDefaults()
{
    // Clear all settings
    settings_->clear();
    
    // Clear caches
    provider_configs_.clear();
    waveform_configs_.clear();
    parameter_configs_.clear();
    
    // Set default values
    SetLastProvider(DEFAULT_PROVIDER);
    SetDefaultSweepSpeed(DEFAULT_SWEEP_SPEED);
    SetDefaultGridColor(DEFAULT_GRID_COLOR);
    SetDefaultBackgroundColor(DEFAULT_BG_COLOR);
    
    // Initialize default waveform configurations
    for (int i = 0; i < 13; ++i) {
        VitalSync::WaveformType type = static_cast<VitalSync::WaveformType>(i);
        auto range = VitalSync::GetDefaultWaveformRange(type);
        
        QVariantMap config;
        config["active"] = (i < 5); // Activate the first 5 waveforms by default
        config["color"] = QColor(Qt::green); // Default color for waveforms
        config["minValue"] = range.first;
        config["maxValue"] = range.second;
        config["bufferSize"] = VitalSync::DEFAULT_SAMPLE_RATE * VitalSync::DEFAULT_BUFFER_SECONDS;
        
        SetWaveformConfig(type, config);
    }
    
    // Initialize default parameter configurations
    for (int i = 0; i < 18; ++i) {
        VitalSync::ParameterType type = static_cast<VitalSync::ParameterType>(i);
        auto range = VitalSync::GetDefaultParameterRange(type);
        auto alarmLimits = VitalSync::GetDefaultAlarmLimits(type);
        
        QVariantMap config;
        config["active"] = (i < 8); // Activate the first 8 parameters by default
        config["color"] = QColor(Qt::yellow); // Default color for parameters
        config["minValue"] = range.first;
        config["maxValue"] = range.second;
        config["lowCritical"] = std::get<0>(alarmLimits);
        config["lowWarning"] = std::get<1>(alarmLimits);
        config["highWarning"] = std::get<2>(alarmLimits);
        config["highCritical"] = std::get<3>(alarmLimits);
        
        SetParameterConfig(type, config);
    }
    
    // Set dirty flag and emit signals
    dirty_ = true;
    emit settingsChanged();
}

/**
 * @brief Gets a string value from settings
 * @param key The setting key to retrieve
 * @param defaultValue Default value to return if key is not found
 * @return The stored string value or defaultValue if not found
 * 
 * Retrieves a string value from the application settings using the provided key.
 * If the key does not exist, returns the specified default value.
 */
QString ConfigManager::GetString(const QString& key, const QString& defaultValue) const
{
    return settings_->value(key, defaultValue).toString();
}

/**
 * @brief Sets a string value in settings
 * @param key The setting key to set
 * @param value The string value to store
 * 
 * Stores a string value in the application settings under the provided key.
 * Sets the dirty flag and emits the settingsChanged signal if the value changes.
 */
void ConfigManager::SetString(const QString& key, const QString& value)
{
    if (GetString(key) != value) {
        settings_->setValue(key, value);
        dirty_ = true;
        emit settingsChanged();
    }
}

/**
 * @brief Gets an integer value from settings
 * @param key The setting key to retrieve
 * @param defaultValue Default value to return if key is not found
 * @return The stored integer value or defaultValue if not found
 * 
 * Retrieves an integer value from the application settings using the provided key.
 * If the key does not exist, returns the specified default value.
 */
int ConfigManager::GetInt(const QString& key, int defaultValue) const
{
    return settings_->value(key, defaultValue).toInt();
}

/**
 * @brief Sets an integer value in settings
 * @param key The setting key to set
 * @param value The integer value to store
 * 
 * Stores an integer value in the application settings under the provided key.
 * Sets the dirty flag and emits the settingsChanged signal if the value changes.
 */
void ConfigManager::SetInt(const QString& key, int value)
{
    if (GetInt(key) != value) {
        settings_->setValue(key, value);
        dirty_ = true;
        emit settingsChanged();
    }
}

/**
 * @brief Gets a double value from settings
 * @param key The setting key to retrieve
 * @param defaultValue Default value to return if key is not found
 * @return The stored double value or defaultValue if not found
 * 
 * Retrieves a double value from the application settings using the provided key.
 * If the key does not exist, returns the specified default value.
 */
double ConfigManager::GetDouble(const QString& key, double defaultValue) const
{
    return settings_->value(key, defaultValue).toDouble();
}

/**
 * @brief Sets a double value in settings
 * @param key The setting key to set
 * @param value The double value to store
 * 
 * Stores a double value in the application settings under the provided key.
 * Sets the dirty flag and emits the settingsChanged signal if the value changes.
 */
void ConfigManager::SetDouble(const QString& key, double value)
{
    if (GetDouble(key) != value) {
        settings_->setValue(key, value);
        dirty_ = true;
        emit settingsChanged();
    }
}

/**
 * @brief Gets a boolean value from settings
 * @param key The setting key to retrieve
 * @param defaultValue Default value to return if key is not found
 * @return The stored boolean value or defaultValue if not found
 * 
 * Retrieves a boolean value from the application settings using the provided key.
 * If the key does not exist, returns the specified default value.
 */
bool ConfigManager::GetBool(const QString& key, bool defaultValue) const
{
    return settings_->value(key, defaultValue).toBool();
}

/**
 * @brief Sets a boolean value in settings
 * @param key The setting key to set
 * @param value The boolean value to store
 * 
 * Stores a boolean value in the application settings under the provided key.
 * Sets the dirty flag and emits the settingsChanged signal if the value changes.
 */
void ConfigManager::SetBool(const QString& key, bool value)
{
    if (GetBool(key) != value) {
        settings_->setValue(key, value);
        dirty_ = true;
        emit settingsChanged();
    }
}

/**
 * @brief Gets a color value from settings
 * @param key The setting key to retrieve
 * @param defaultValue Default color to return if key is not found
 * @return The stored color value or defaultValue if not found
 * 
 * Retrieves a QColor value from the application settings using the provided key.
 * If the key does not exist, returns the specified default color.
 */
QColor ConfigManager::GetColor(const QString& key, const QColor& defaultValue) const
{
    return settings_->value(key, defaultValue).value<QColor>();
}

/**
 * @brief Sets a color value in settings
 * @param key The setting key to set
 * @param value The color value to store
 * 
 * Stores a QColor value in the application settings under the provided key.
 * Sets the dirty flag and emits the settingsChanged signal if the color changes.
 */
void ConfigManager::setColor(const QString& key, const QColor& value)
{
    if (GetColor(key) != value) {
        settings_->setValue(key, value);
        dirty_ = true;
        emit settingsChanged();
    }
}

/**
 * @brief Gets configuration for a specific provider
 * @param providerName The name of the provider
 * @return Map of configuration values for the provider
 * 
 * Retrieves the configuration map for the specified provider.
 * Returns an empty map if no configuration exists for the provider.
 */
QVariantMap ConfigManager::GetProviderConfig(const QString& providerName) const
{
    return provider_configs_.value(providerName, QVariantMap());
}

/**
 * @brief Sets configuration for a specific provider
 * @param providerName The name of the provider
 * @param config Map of configuration values for the provider
 * 
 * Stores the configuration map for the specified provider.
 * Sets the dirty flag and emits the providerConfigChanged and
 * settingsChanged signals.
 */
void ConfigManager::SetProviderConfig(const QString& providerName, const QVariantMap& config)
{
    provider_configs_[providerName] = config;
    dirty_ = true;
    emit providerConfigChanged(providerName);
    emit settingsChanged();
}

/**
 * @brief Gets configuration for a specific waveform type
 * @param waveformType The type of waveform
 * @return Map of configuration values for the waveform
 * 
 * Retrieves the configuration map for the specified waveform type.
 * Returns an empty map if no configuration exists for the waveform type.
 */
QVariantMap ConfigManager::GetWaveformConfig(VitalSync::WaveformType waveformType) const
{
    return waveform_configs_.value(static_cast<int>(waveformType), QVariantMap());
}

/**
 * @brief Sets configuration for a specific waveform type
 * @param waveformType The type of waveform
 * @param config Map of configuration values for the waveform
 * 
 * Stores the configuration map for the specified waveform type.
 * Sets the dirty flag and emits the waveformConfigChanged and
 * settingsChanged signals.
 */
void ConfigManager::SetWaveformConfig(VitalSync::WaveformType waveformType, const QVariantMap& config)
{
    waveform_configs_[static_cast<int>(waveformType)] = config;
    dirty_ = true;
    emit waveformConfigChanged(waveformType);
    emit settingsChanged();
}

/**
 * @brief Gets configuration for a specific parameter type
 * @param parameterType The type of parameter
 * @return Map of configuration values for the parameter
 * 
 * Retrieves the configuration map for the specified parameter type.
 * Returns an empty map if no configuration exists for the parameter type.
 */
QVariantMap ConfigManager::GetParameterConfig(VitalSync::ParameterType parameterType) const
{
    return parameter_configs_.value(static_cast<int>(parameterType), QVariantMap());
}

/**
 * @brief Sets configuration for a specific parameter type
 * @param parameterType The type of parameter
 * @param config Map of configuration values for the parameter
 * 
 * Stores the configuration map for the specified parameter type.
 * Sets the dirty flag and emits the parameterConfigChanged and
 * settingsChanged signals.
 */
void ConfigManager::SetParameterConfig(VitalSync::ParameterType parameterType, const QVariantMap& config)
{
    parameter_configs_[static_cast<int>(parameterType)] = config;
    dirty_ = true;
    emit parameterConfigChanged(parameterType);
    emit settingsChanged();
}

/**
 * @brief Gets the name of the last used provider
 * @return Name of the last used provider
 * 
 * Retrieves the name of the last provider that was active.
 * Returns the default provider name if none was previously set.
 */
QString ConfigManager::GetLastProvider() const
{
    return GetString(KEY_LAST_PROVIDER, DEFAULT_PROVIDER);
}

/**
 * @brief Sets the name of the last used provider
 * @param providerName Name of the provider to save
 * 
 * Stores the name of the currently active provider for restoration
 * when the application is next launched.
 */
void ConfigManager::SetLastProvider(const QString& providerName)
{
    SetString(KEY_LAST_PROVIDER, providerName);
}

/**
 * @brief Gets the default sweep speed for waveform displays
 * @return Default sweep speed in millimeters per second
 * 
 * Retrieves the configured default sweep speed for waveform displays.
 * Returns the system default if none was previously set.
 */
double ConfigManager::GetDefaultSweepSpeed() const
{
    return GetDouble(KEY_DEFAULT_SWEEP_SPEED, DEFAULT_SWEEP_SPEED);
}

/**
 * @brief Sets the default sweep speed for waveform displays
 * @param speed Sweep speed in millimeters per second
 * 
 * Stores the default sweep speed for waveform displays.
 * This affects how quickly waveforms scroll across the display.
 */
void ConfigManager::SetDefaultSweepSpeed(double speed)
{
    SetDouble(KEY_DEFAULT_SWEEP_SPEED, speed);
}

/**
 * @brief Gets the default grid color for waveform displays
 * @return Default grid color
 * 
 * Retrieves the configured default grid color for waveform displays.
 * Returns the system default color if none was previously set.
 */
QColor ConfigManager::GetDefaultGridColor() const
{
    return GetColor(KEY_DEFAULT_GRID_COLOR, DEFAULT_GRID_COLOR);
}

/**
 * @brief Sets the default grid color for waveform displays
 * @param color Grid color
 * 
 * Stores the default grid color for waveform displays.
 * This affects the appearance of grid lines on waveform views.
 */
void ConfigManager::SetDefaultGridColor(const QColor& color)
{
    setColor(KEY_DEFAULT_GRID_COLOR, color);
}

/**
 * @brief Gets the default background color for waveform displays
 * @return Default background color
 * 
 * Retrieves the configured default background color for waveform displays.
 * Returns the system default color if none was previously set.
 */
QColor ConfigManager::GetDefaultBackgroundColor() const
{
    return GetColor(KEY_DEFAULT_BG_COLOR, DEFAULT_BG_COLOR);
}

/**
 * @brief Sets the default background color for waveform displays
 * @param color Background color
 * 
 * Stores the default background color for waveform displays.
 * This affects the appearance of the background on waveform views.
 */
void ConfigManager::SetDefaultBackgroundColor(const QColor& color)
{
    setColor(KEY_DEFAULT_BG_COLOR, color);
} 