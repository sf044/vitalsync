/**
 * @file main_window.h
 * @brief Main application window
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <memory>

#include "../../include/i_data_manager.h"
#include "../../include/i_waveform_view.h"
#include "../../include/i_parameter_view.h"
#include "../../include/vital_sync_types.h"

namespace Ui {
class MainWindow;
}

/**
 * @class MainWindow
 * @brief Main application window that displays the patient monitor UI
 * 
 * This class serves as the main UI, integrating
 * waveform views, parameter displays, and controls for managing data providers.
 * It handles the main application lifecycle, user interactions with the monitoring
 * display, and connections to data providers.
 * 
 * The window displays:
 * - Multiple physiological waveforms (ECG, respiration, etc.)
 * - Numerical vital sign parameters (heart rate, blood pressure, etc.)
 * - Controls for selecting and configuring data providers
 * - Status information about the current connection
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the main application window
     * 
     * Creates the main window, initializes the data manager, sets up the UI components,
     * connects signals and slots, and applies default settings.
     * 
     * @param parent Parent widget, defaults to nullptr
     */
    explicit MainWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up resources, stops data acquisition if active,
     * and destroys UI components.
     */
    ~MainWindow();

protected:
    /**
     * @brief Handle window close event
     * 
     * Performs clean shutdown operations when the window is closed,
     * including stopping data acquisition and saving configuration.
     * 
     * @param event Close event containing information about the close operation
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    /**
     * @brief Handle start/stop acquisition button click
     * 
     * Toggles data acquisition state. If currently acquiring, stops acquisition.
     * If not acquiring, starts acquisition with the currently selected provider.
     * Updates UI elements to reflect the new acquisition state.
     */
    void OnStartStopButtonClicked();

    /**
     * @brief Handle provider selection change
     * 
     * Updates the active data provider when the user selects a different provider
     * from the dropdown. Configures the buttons based on the selected provider's
     * availability and current status.
     * 
     * @param index Index of the selected provider in the combo box
     */
    void OnProviderSelectionChanged(int index);

    /**
     * @brief Handle connection status changes
     * 
     * Updates the UI to reflect changes in the connection status of the
     * current data provider.
     * 
     * @param status New connection status (Connected, Disconnected, Connecting, Error)
     */
    void OnConnectionStatusChanged(VitalSync::ConnectionStatus status);

    /**
     * @brief Handle provider errors
     * 
     * Displays error messages from the data provider and takes appropriate action
     * based on the error severity.
     * 
     * @param errorCode Numeric error code indicating the type of error
     * @param errorMessage Descriptive error message to be displayed to the user
     */
    void OnErrorOccurred(int errorCode, const QString& errorMessage);

    /**
     * @brief Handle configuration button click
     * 
     * Opens the provider configuration dialog for the currently selected provider,
     * allowing the user to modify provider-specific settings.
     */
    void OnConfigureProviderClicked();

    /**
     * @brief Show settings dialog
     * 
     * Opens the application settings dialog, allowing the user to customize
     * display settings, alarm limits, and other application configurations.
     */
    void OnSettingsButtonClicked();

private:
    /**
     * @brief Initialize the user interface
     * 
     * Creates and configures all UI components including the toolbar, status bar,
     * central widget, and layouts for waveforms and parameters. Sets up initial
     * visual appearance and widget hierarchies.
     */
    void SetupUi();

    /**
     * @brief Create and add waveform views
     * 
     * Initializes all waveform visualization components and adds them to the
     * main layout. Creates views for ECG, respiration, SpO2, and other waveforms.
     */
    void SetupWaveformViews();

    /**
     * @brief Create and add parameter views
     * 
     * Initializes all parameter display components and adds them to the
     * main layout. Creates views for heart rate, blood pressure, temperature,
     * and other numerical parameters.
     */
    void SetupParameterViews();

    /**
     * @brief Connect signals and slots
     * 
     * Establishes connections between UI elements, data manager, models, and views.
     * Sets up the event handling chain for user interactions and data updates.
     */
    void connectSignals();

    /**
     * @brief Update UI elements based on connection status
     * 
     * Updates button states, status indicators, and other UI elements
     * to reflect the current connection status.
     * 
     * @param status Current connection status to be displayed
     */
    void UpdateConnectionStatus(VitalSync::ConnectionStatus status);

    /**
     * @brief Connect waveform models to views
     * 
     * Establishes the connections between waveform data models and their
     * corresponding visualization components. Sets up signal-slot connections
     * for real-time data updates.
     */
    void connectWaveformModels();

    /**
     * @brief Connect parameter models to views
     * 
     * Establishes the connections between parameter models and their
     * corresponding display components. Sets up signal-slot connections
     * for real-time value updates.
     */
    void connectParameterModels();

    /**
     * @brief Apply default settings to views
     * 
     * Configures all views with default appearance settings from the
     * configuration manager, including colors, ranges, and sweep speeds.
     */
    void ApplyDefaultSettings();

    /**
     * @brief Get view for a waveform type
     * 
     * Retrieves the waveform view component associated with a specific waveform type.
     * 
     * @param type Waveform type enumeration value
     * @return Shared pointer to the waveform view or nullptr if not found
     */
    std::shared_ptr<IWaveformView> GetWaveformView(VitalSync::WaveformType type) const;

    /**
     * @brief Get view for a parameter type
     * 
     * Retrieves the parameter view component associated with a specific parameter type.
     * 
     * @param type Parameter type enumeration value
     * @return Shared pointer to the parameter view or nullptr if not found
     */
    std::shared_ptr<IParameterView> GetParameterView(VitalSync::ParameterType type) const;

    /**
     * @brief UI elements for provider control and status display
     */
    QComboBox* provider_selector_;     /**< Dropdown for selecting the data provider */
    QPushButton* start_stop_button_;   /**< Button to start/stop data acquisition */
    QPushButton* configure_button_;    /**< Button to open provider configuration dialog */
    QPushButton* settings_button_;     /**< Button to open application settings dialog */
    QLabel* status_label_;             /**< Label for displaying status messages */
    QLabel* connection_status_label_;  /**< Label for displaying connection status */

    /**
     * @brief Core components for data management
     */
    std::shared_ptr<IDataManager> data_manager_;  /**< Manager for handling data providers and models */

    /**
     * @brief View component collections
     */
    QMap<VitalSync::WaveformType, std::shared_ptr<IWaveformView>> waveform_views_;       /**< Map of waveform views by type */
    QMap<VitalSync::ParameterType, std::shared_ptr<IParameterView>> parameter_views_;    /**< Map of parameter views by type */

    /**
     * @brief State tracking variables
     */
    bool is_acquiring_;                         /**< Flag indicating if data acquisition is active */
    QString current_provider_name_;             /**< Name of the currently selected provider */
    VitalSync::ConnectionStatus connection_status_;  /**< Current connection status */
};

#endif // MAIN_WINDOW_H 