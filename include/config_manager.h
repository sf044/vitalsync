/**
 * @file config_manager.h
 * @brief Definition of the ConfigManager class for application settings
 * 
 * ConfigManager class, responsible for managing all application settings and configuration data.
 * It provides an interface for accessing and modifying
 * configuration values, including provider settings, waveform display options,
 * parameter settings, and application preferences. The class uses Singleton
 * pattern to ensure a single point of access to configuration.
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QColor>
#include <QMap>
#include <QString>
#include <memory>

#include "vital_sync_types.h"

/**
 * @brief Configuration manager for all application settings
 * 
 * The ConfigManager is responsible for loading, saving, and providing access to
 * all application settings and configuration data. It uses QSettings internally
 * for persistent storage and provides typed accessors for various configuration
 * options, including:
 * 
 * - Basic type settings (string, int, double, bool, color)
 * - Provider-specific configurations
 * - Waveform display settings
 * - Parameter display and alarm settings
 * - Default display options (sweep speed, colors)
 * 
 * The class follows the Singleton pattern to ensure a single point of access
 * to configuration throughout the application, and emits signals when settings
 * are changed to allow components to react to configuration updates.
 */
class ConfigManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance of the configuration manager
     * @return Reference to the singleton instance
     * 
     * Creates and returns a reference to the singleton ConfigManager instance.
     * If the instance doesn't exist yet, it will be created.
     */
    static ConfigManager& GetInstance();

    /**
     * @brief Initialize the configuration manager
     * @param organization Organization name for settings storage
     * @param application Application name for settings storage
     * @return True if initialization was successful
     * 
     * Creates the QSettings object with the provided organization and application names.
     * Loads previously saved settings from persistent storage.
     */
    bool Initialize(const QString& organization, const QString& application);

    /**
     * @brief Save the current configuration
     * @return True if save was successful
     * 
     * Writes all settings to persistent storage and clears the dirty flag.
     */
    bool save();

    /**
     * @brief Reset all settings to default values
     * 
     * Clears all settings and restores default values for all configuration options.
     * Emits the settingsChanged signal to notify listeners of the changes.
     */
    void resetToDefaults();

    /**
     * @brief Get a string setting value
     * @param key Setting key to retrieve
     * @param defaultValue Default value if the key is not found
     * @return The stored string value or defaultValue if not found
     * 
     * Retrieves a string value from the application settings using the provided key.
     * If the key does not exist, returns the specified default value.
     */
    QString GetString(const QString& key, const QString& defaultValue = QString()) const;

    /**
     * @brief Set a string setting value
     * @param key Setting key to set
     * @param value New string value to store
     * 
     * Stores a string value in the application settings under the provided key.
     * Sets the dirty flag and emits the settingsChanged signal if the value changes.
     */
    void SetString(const QString& key, const QString& value);

    /**
     * @brief Get an integer setting value
     * @param key Setting key to retrieve
     * @param defaultValue Default value if the key is not found
     * @return The stored integer value or defaultValue if not found
     * 
     * Retrieves an integer value from the application settings using the provided key.
     * If the key does not exist, returns the specified default value.
     */
    int GetInt(const QString& key, int defaultValue = 0) const;

    /**
     * @brief Set an integer setting value
     * @param key Setting key to set
     * @param value New integer value to store
     * 
     * Stores an integer value in the application settings under the provided key.
     * Sets the dirty flag and emits the settingsChanged signal if the value changes.
     */
    void SetInt(const QString& key, int value);

    /**
     * @brief Get a double setting value
     * @param key Setting key to retrieve
     * @param defaultValue Default value if the key is not found
     * @return The stored double value or defaultValue if not found
     * 
     * Retrieves a double value from the application settings using the provided key.
     * If the key does not exist, returns the specified default value.
     */
    double GetDouble(const QString& key, double defaultValue = 0.0) const;

    /**
     * @brief Set a double setting value
     * @param key Setting key to set
     * @param value New double value to store
     * 
     * Stores a double value in the application settings under the provided key.
     * Sets the dirty flag and emits the settingsChanged signal if the value changes.
     */
    void SetDouble(const QString& key, double value);

    /**
     * @brief Get a boolean setting value
     * @param key Setting key to retrieve
     * @param defaultValue Default value if the key is not found
     * @return The stored boolean value or defaultValue if not found
     * 
     * Retrieves a boolean value from the application settings using the provided key.
     * If the key does not exist, returns the specified default value.
     */
    bool GetBool(const QString& key, bool defaultValue = false) const;

    /**
     * @brief Set a boolean setting value
     * @param key Setting key to set
     * @param value New boolean value to store
     * 
     * Stores a boolean value in the application settings under the provided key.
     * Sets the dirty flag and emits the settingsChanged signal if the value changes.
     */
    void SetBool(const QString& key, bool value);

    /**
     * @brief Get a color setting value
     * @param key Setting key to retrieve
     * @param defaultValue Default color if the key is not found
     * @return The stored color value or defaultValue if not found
     * 
     * Retrieves a QColor value from the application settings using the provided key.
     * If the key does not exist, returns the specified default color.
     */
    QColor GetColor(const QString& key, const QColor& defaultValue = Qt::black) const;

    /**
     * @brief Set a color setting value
     * @param key Setting key to set
     * @param value New color value to store
     * 
     * Stores a QColor value in the application settings under the provided key.
     * Sets the dirty flag and emits the settingsChanged signal if the color changes.
     */
    void setColor(const QString& key, const QColor& value);

    /**
     * @brief Get the data provider configuration
     * @param providerName Name of the provider to retrieve configuration for
     * @return Map of configuration values for the provider
     * 
     * Retrieves the configuration map for the specified provider.
     * Returns an empty map if no configuration exists for the provider.
     */
    QVariantMap GetProviderConfig(const QString& providerName) const;

    /**
     * @brief Set the data provider configuration
     * @param providerName Name of the provider to configure
     * @param config Map of configuration values for the provider
     * 
     * Stores the configuration map for the specified provider.
     * Sets the dirty flag and emits the providerConfigChanged and
     * settingsChanged signals.
     */
    void SetProviderConfig(const QString& providerName, const QVariantMap& config);

    /**
     * @brief Get the waveform configuration
     * @param waveformType Type of waveform to retrieve configuration for
     * @return Map of configuration values for the waveform
     * 
     * Retrieves the configuration map for the specified waveform type.
     * Returns an empty map if no configuration exists for the waveform type.
     */
    QVariantMap GetWaveformConfig(VitalSync::WaveformType waveformType) const;

    /**
     * @brief Set the waveform configuration
     * @param waveformType Type of waveform to configure
     * @param config Map of configuration values for the waveform
     * 
     * Stores the configuration map for the specified waveform type.
     * Sets the dirty flag and emits the waveformConfigChanged and
     * settingsChanged signals.
     */
    void SetWaveformConfig(VitalSync::WaveformType waveformType, const QVariantMap& config);

    /**
     * @brief Get the parameter configuration
     * @param parameterType Type of parameter to retrieve configuration for
     * @return Map of configuration values for the parameter
     * 
     * Retrieves the configuration map for the specified parameter type.
     * Returns an empty map if no configuration exists for the parameter type.
     */
    QVariantMap GetParameterConfig(VitalSync::ParameterType parameterType) const;

    /**
     * @brief Set the parameter configuration
     * @param parameterType Type of parameter to configure
     * @param config Map of configuration values for the parameter
     * 
     * Stores the configuration map for the specified parameter type.
     * Sets the dirty flag and emits the parameterConfigChanged and
     * settingsChanged signals.
     */
    void SetParameterConfig(VitalSync::ParameterType parameterType, const QVariantMap& config);

    /**
     * @brief Get the last used provider name
     * @return Name of the last used provider
     * 
     * Retrieves the name of the last provider that was active.
     * Returns the default provider name if none was previously set.
     */
    QString GetLastProvider() const;

    /**
     * @brief Set the last used provider name
     * @param providerName Name of the provider to save
     * 
     * Stores the name of the currently active provider for restoration
     * when the application is next launched.
     */
    void SetLastProvider(const QString& providerName);

    /**
     * @brief Get the default sweep speed for waveform displays
     * @return Default sweep speed in millimeters per second
     * 
     * Retrieves the configured default sweep speed for waveform displays.
     * Returns the system default if none was previously set.
     */
    double GetDefaultSweepSpeed() const;

    /**
     * @brief Get the default grid color for waveform displays
     * @return Default grid color
     * 
     * Retrieves the configured default grid color for waveform displays.
     * Returns the system default color if none was previously set.
     */
    QColor GetDefaultGridColor() const;

    /**
     * @brief Set the default grid color for waveform displays
     * @param color Grid color
     * 
     * Stores the default grid color for waveform displays.
     * This affects the appearance of grid lines on waveform views.
     */
    void SetDefaultGridColor(const QColor& color);

    /**
     * @brief Get the default background color for waveform displays
     * @return Default background color
     * 
     * Retrieves the configured default background color for waveform displays.
     * Returns the system default color if none was previously set.
     */
    QColor GetDefaultBackgroundColor() const;

    /**
     * @brief Set the default background color for waveform displays
     * @param color Background color
     * 
     * Stores the default background color for waveform displays.
     * This affects the appearance of the background on waveform views.
     */
    void SetDefaultBackgroundColor(const QColor& color);

    /**
     * @brief Set the default sweep speed for waveform displays
     * @param speed Sweep speed in millimeters per second
     * 
     * Stores the default sweep speed for waveform displays.
     * This affects how quickly waveforms scroll across the display.
     */
    void SetDefaultSweepSpeed(double speed);

signals:
    /**
     * @brief Signal emitted when any settings change
     * 
     * This signal is emitted whenever any setting is modified, including
     * basic settings, provider configurations, waveform settings, and
     * parameter settings. Components that need to react to any change
     * in configuration should connect to this signal.
     */
    void settingsChanged();

    /**
     * @brief Signal emitted when provider configuration changes
     * @param providerName Name of the provider that was modified
     * 
     * This signal is emitted when a specific provider's configuration is
     * modified. Components that need to react to changes in a particular
     * provider's settings should connect to this signal.
     */
    void providerConfigChanged(const QString& providerName);

    /**
     * @brief Signal emitted when waveform configuration changes
     * @param waveformType Type of waveform that was modified
     * 
     * This signal is emitted when a specific waveform's configuration is
     * modified. Components that need to react to changes in a particular
     * waveform's settings should connect to this signal.
     */
    void waveformConfigChanged(VitalSync::WaveformType waveformType);

    /**
     * @brief Signal emitted when parameter configuration changes
     * @param parameterType Type of parameter that was modified
     * 
     * This signal is emitted when a specific parameter's configuration is
     * modified. Components that need to react to changes in a particular
     * parameter's settings should connect to this signal.
     */
    void parameterConfigChanged(VitalSync::ParameterType parameterType);

private:
    /**
     * @brief Private constructor for singleton pattern
     * 
     * Initializes the configuration manager with empty maps for provider,
     * waveform, and parameter configurations. Sets the dirty flag to false
     * to indicate no unsaved changes initially.
     */
    ConfigManager();

    /**
     * @brief Private destructor for singleton pattern
     * 
     * Saves any unsaved configuration changes to persistent storage
     * before destroying the configuration manager instance.
     */
    ~ConfigManager();

    /**
     * @brief Delete copy constructor
     * 
     * Prevents copying of the singleton instance to maintain
     * the singleton pattern integrity.
     */
    ConfigManager(const ConfigManager&) = delete;

    /**
     * @brief Delete assignment operator
     * 
     * Prevents assignment of the singleton instance to maintain
     * the singleton pattern integrity.
     */
    ConfigManager& operator=(const ConfigManager&) = delete;

    // Settings object for storing configuration
    std::unique_ptr<QSettings> settings_;

    // Cache of provider configurations
    QMap<QString, QVariantMap> provider_configs_;

    // Cache of waveform configurations
    QMap<int, QVariantMap> waveform_configs_;

    // Cache of parameter configurations
    QMap<int, QVariantMap> parameter_configs_;

    // Flag indicating if configuration has been modified
    bool dirty_;
};

#endif // CONFIG_MANAGER_H 