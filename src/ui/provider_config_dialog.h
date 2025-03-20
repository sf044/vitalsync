#ifndef PROVIDER_CONFIG_DIALOG_H
#define PROVIDER_CONFIG_DIALOG_H

/**
 * @file provider_config_dialog.h
 * @brief Dialog for configuring data provider settings
 * 
 * This file defines the ProviderConfigDialog class which provides a user interface
 * for configuring settings specific to each data provider type (Demo, Network, File).
 * The dialog dynamically adjusts its UI based on the provider type.
 */

#include <QDialog>
#include <QVariantMap>
#include <QString>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <unordered_map>

/**
 * @class ProviderConfigDialog
 * @brief Dialog for configuring data provider settings
 * 
 * This dialog provides an interface for users to configure provider-specific settings.
 * It adapts to the type of provider being configured (Demo, Network, File) and 
 * displays appropriate configuration options for each type.
 * 
 * For the Demo provider, it exposes settings like simulated vital signs,
 * waveform characteristics, and update intervals.
 * 
 * For the Network provider, it shows connection settings like host, port,
 * credentials, and protocol options.
 * 
 * For the File provider, it allows configuration of data file sources
 * and playback settings.
 */
class ProviderConfigDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the provider configuration dialog
     * 
     * Creates and initializes the dialog with controls specific to the given provider type,
     * and applies the current configuration values to the UI elements.
     *
     * @param providerName Name of the provider being configured (Demo, Network, File)
     * @param config Current provider configuration as a key-value map
     * @param parent Parent widget for this dialog
     */
    explicit ProviderConfigDialog(const QString& providerName, const QVariantMap& config, QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up any resources allocated by the dialog.
     */
    ~ProviderConfigDialog() override;
    
    /**
     * @brief Get the updated configuration
     * 
     * Retrieves the configuration settings as modified by the user
     * through the dialog interface.
     *
     * @return Configuration map with updated values
     */
    QVariantMap GetConfig() const;

private slots:
    /**
     * @brief Handle OK button click
     * 
     * Validates the input values, updates the configuration map,
     * and closes the dialog with an accept result.
     */
    void OnAccept();
    
    /**
     * @brief Handle Cancel button click
     * 
     * Discards any changes and closes the dialog with a reject result.
     */
    void OnReject();
    
    /**
     * @brief Handle demo waveform type selection change
     * 
     * Updates the available waveform configuration options based on
     * the selected waveform type in the Demo provider settings.
     *
     * @param index Index of the newly selected waveform type
     */
    void OnDemoWaveformTypeChanged(int index);
    
    /**
     * @brief Handle demo update interval change
     * 
     * Adjusts the update-related UI elements when the user changes
     * the update interval for the Demo provider.
     *
     * @param value New update interval value in milliseconds
     */
    void OnDemoUpdateIntervalChanged(int value);

private:
    /**
     * @brief Set up the user interface
     * 
     * Creates the dialog layout and initializes provider-specific content
     * based on the provider type.
     */
    void SetupUi();
    
    /**
     * @brief Create controls based on provider type
     * 
     * Initializes all UI control pointers to nullptr, then creates the
     * appropriate control set based on the provider type.
     */
    void CreateControls();
    
    /**
     * @brief Create controls for demo provider
     * 
     * Builds the control set specific to the Demo provider, including
     * settings for simulated vital signs and waveform characteristics.
     *
     * @return Widget containing the Demo provider control set
     */
    QWidget* CreateDemoProviderControls();
    
    /**
     * @brief Create controls for network provider
     * 
     * Builds the control set specific to the Network provider, including
     * connection settings and protocol options.
     *
     * @return Widget containing the Network provider control set
     */
    QWidget* CreateNetworkProviderControls();
    
    /**
     * @brief Create controls for file provider
     * 
     * Builds the control set specific to the File provider, including
     * file path and playback options.
     *
     * @return Widget containing the File provider control set
     */
    QWidget* CreateFileProviderControls();
    
    /**
     * @brief Update configuration from UI controls
     * 
     * Reads the current values from all UI controls and updates
     * the configuration map accordingly.
     */
    void UpdateConfigFromControls();
    
    /**
     * @brief Apply configuration to UI controls
     * 
     * Sets the values of all UI controls based on the
     * current configuration map.
     */
    void ApplyConfigToControls();
    
    /**
     * @brief Update UI elements based on waveform type
     * 
     * Adjusts available settings and control ranges when the
     * waveform type changes in the Demo provider settings.
     *
     * @param index Index of the newly selected waveform type
     */
    void UpdateDemoWaveformControls(int index);
    
    /**
     * @brief Provider information
     */
    QString provider_name_;    /**< Name of the provider being configured */
    QVariantMap config_;       /**< Configuration map with provider settings */
    
    /**
     * @brief Demo provider controls
     */
    QSpinBox* heart_rate_spin_box_;          /**< Control for simulated heart rate */
    QSpinBox* respiration_rate_spin_box_;    /**< Control for simulated respiration rate */
    QSpinBox* spo2_spin_box_;                /**< Control for simulated SpO2 percentage */
    QSpinBox* systolic_bp_spin_box_;         /**< Control for simulated systolic blood pressure */
    QSpinBox* diastolic_bp_spin_box_;        /**< Control for simulated diastolic blood pressure */
    QDoubleSpinBox* temperature_spin_box_;   /**< Control for simulated temperature */
    QSpinBox* etco2_spin_box_;               /**< Control for simulated end-tidal CO2 */
    QSpinBox* update_interval_spin_box_;     /**< Control for data update interval */
    QComboBox* waveform_type_combo_box_;     /**< Control for selecting waveform type */
    QDoubleSpinBox* amplitude_spin_box_;     /**< Control for waveform amplitude */
    QDoubleSpinBox* frequency_spin_box_;     /**< Control for waveform frequency */
    QDoubleSpinBox* noise_spin_box_;         /**< Control for simulated noise level */
    QCheckBox* artifacts_check_box_;         /**< Control for enabling simulated artifacts */
    
    /**
     * @brief Network provider controls
     */
    QLineEdit* host_line_edit_;              /**< Control for server hostname or IP */
    QSpinBox* port_spin_box_;                /**< Control for server port */
    QLineEdit* username_line_edit_;          /**< Control for connection username */
    QLineEdit* password_line_edit_;          /**< Control for connection password */
    QComboBox* protocol_combo_box_;          /**< Control for connection protocol */
    
    /**
     * @brief File provider controls
     */
    QLineEdit* file_path_line_edit_;         /**< Control for data file path */
    QDoubleSpinBox* playback_speed_spin_box_; /**< Control for playback speed multiplier */
    QCheckBox* loop_check_box_;              /**< Control for enabling loop playback */
};

#endif // PROVIDER_CONFIG_DIALOG_H 