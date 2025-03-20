#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

/**
 * @file settings_dialog.h
 * @brief Dialog for application settings
 */

#include <QDialog>
#include <QMap>
#include <QColor>
#include <QCheckBox>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QLineEdit>

#include "../../include/config_manager.h"
#include "../../include/vital_sync_types.h"

namespace Ui {
class SettingsDialog;
}

/**
 * @class SettingsDialog
 * @brief Dialog for configuring application settings
 *
 * This dialog allows users to configure various application settings including
 * display preferences, waveform parameters, and alarm thresholds.
 * 
 * The dialog is organized into multiple tabs:
 * - General: Basic display settings like sweep speed and colors
 * - Waveforms: Configuration for each supported waveform type
 * - Parameters: Configuration for each supported vital sign parameter
 * - Alarms: Threshold settings for different alarm conditions
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for the settings dialog
     * 
     * Creates and initializes the settings dialog with current configuration values
     * from the ConfigManager.
     *
     * @param parent Parent widget for this dialog
     */
    explicit SettingsDialog(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     * 
     * Cleans up any resources allocated by the dialog
     */
    ~SettingsDialog();

    /**
     * @brief Get the configured sweep speed
     * 
     * Retrieves the currently configured sweep speed for waveform displays,
     * which determines how quickly the waveform scrolls across the screen.
     *
     * @return Sweep speed in pixels per second
     */
    double GetSweepSpeed() const;
    
    /**
     * @brief Set the sweep speed
     * 
     * Sets the sweep speed for all waveform displays. The sweep speed determines
     * how quickly the waveform scrolls across the screen.
     *
     * @param pixelsPerSecond Sweep speed in pixels per second
     */
    void SetSweepSpeed(double pixelsPerSecond);
    
    /**
     * @brief Get the configured grid color
     * 
     * Retrieves the currently configured grid color for waveform displays.
     *
     * @return Grid color as a QColor object
     */
    QColor GetGridColor() const;
    
    /**
     * @brief Set the grid color
     * 
     * Sets the grid color for all waveform displays.
     *
     * @param color Grid color to use
     */
    void SetGridColor(const QColor& color);
    
    /**
     * @brief Get the configured background color
     * 
     * Retrieves the currently configured background color for waveform displays.
     *
     * @return Background color as a QColor object
     */
    QColor GetBackgroundColor() const;
    
    /**
     * @brief Set the background color
     * 
     * Sets the background color for all waveform displays.
     *
     * @param color Background color to use
     */
    void SetBackgroundColor(const QColor& color);
    
    /**
     * @brief Get the configured text color
     * 
     * Retrieves the currently configured text color for display elements.
     *
     * @return Text color as a QColor object
     */
    QColor GetTextColor() const;
    
    /**
     * @brief Set the text color
     * 
     * Sets the text color for labels and other display elements.
     *
     * @param color Text color to use
     */
    void SetTextColor(const QColor& color);

private slots:
    /**
     * @brief Save all settings and close the dialog
     * 
     * Validates and saves all current settings to the configuration manager,
     * then closes the dialog with an accept result.
     */
    void OnAccept();

    /**
     * @brief Discard changes and close the dialog
     * 
     * Discards any unsaved changes and closes the dialog with a reject result.
     */
    void OnReject();

    /**
     * @brief Reset all settings to default values
     * 
     * Resets all settings in the dialog to their default values without saving.
     * This does not affect saved settings until the user accepts the changes.
     */
    void OnResetToDefaults();

    /**
     * @brief Open color dialog for background color selection
     * 
     * Opens a color picker dialog to allow the user to select a new background color
     * for waveform displays.
     */
    void OnBackgroundColorClicked();

    /**
     * @brief Open color dialog for grid color selection
     * 
     * Opens a color picker dialog to allow the user to select a new grid color
     * for waveform displays.
     */
    void OnGridColorClicked();

    /**
     * @brief Handle waveform tab index change
     * 
     * Updates the UI when the user selects a different waveform type
     * in the waveform configuration tab.
     *
     * @param index New tab index corresponding to the selected waveform type
     */
    void OnWaveformTabChanged(int index);

    /**
     * @brief Handle parameter tab index change
     * 
     * Updates the UI when the user selects a different parameter type
     * in the parameter configuration tab.
     *
     * @param index New tab index corresponding to the selected parameter type
     */
    void OnParameterTabChanged(int index);

private:
    /**
     * @brief Set up the user interface
     * 
     * Creates and arranges all dialog components, including tabs, form layouts,
     * input fields, and buttons.
     */
    void SetupUi();

    /**
     * @brief Create general settings tab
     * 
     * Creates the tab for configuring general display settings like
     * sweep speed, grid options, and colors.
     *
     * @return Widget containing general settings controls
     */
    QWidget* CreateGeneralTab();

    /**
     * @brief Create waveform settings tab
     * 
     * Creates the tab for configuring settings specific to each waveform type,
     * including colors and amplitude ranges.
     *
     * @return Widget containing waveform settings controls
     */
    QWidget* CreateWaveformsTab();

    /**
     * @brief Create parameter settings tab
     * 
     * Creates the tab for configuring settings specific to each parameter type,
     * including colors and display formats.
     *
     * @return Widget containing parameter settings controls
     */
    QWidget* CreateParametersTab();

    /**
     * @brief Create alarm settings tab
     * 
     * Creates the tab for configuring alarm thresholds for different parameters,
     * including warning and critical limits.
     *
     * @return Widget containing alarm settings controls
     */
    QWidget* CreateAlarmsTab();

    /**
     * @brief Load current settings from ConfigManager
     * 
     * Retrieves the current configuration values from the ConfigManager
     * and applies them to the dialog controls.
     */
    void LoadSettings();

    /**
     * @brief Save settings to ConfigManager
     * 
     * Saves the current dialog settings to the ConfigManager to make them
     * persistent across application sessions.
     */
    void SaveSettings();

    /**
     * @brief UI elements for general settings
     */
    QDoubleSpinBox* sweep__speed_spin_box;      /**< Control for setting waveform sweep speed */
    QPushButton* background__color_button;      /**< Button for selecting background color */
    QPushButton* grid__color_button;            /**< Button for selecting grid color */
    QCheckBox* show__grid_checkBox;             /**< Option to show/hide the grid on waveforms */
    QCheckBox* show__time_scale_check_box;      /**< Option to show/hide time scale on waveforms */
    QCheckBox* show__amplitude_scale_check_box; /**< Option to show/hide amplitude scale on waveforms */

    /**
     * @brief Waveform configuration UI elements
     */
    QTabWidget* wavefortab__widget_;                                 /**< Tab widget for different waveform types */
    QMap<VitalSync::WaveformType, QPushButton*> waveforcolor__buttons_;    /**< Color selection buttons for each waveform type */
    QMap<VitalSync::WaveformType, QDoubleSpinBox*> waveformin__spin_boxes_; /**< Min range settings for each waveform type */
    QMap<VitalSync::WaveformType, QDoubleSpinBox*> waveformax__spin_boxes_; /**< Max range settings for each waveform type */

    /**
     * @brief Parameter configuration UI elements
     */
    QTabWidget* parameter_tab_widget_;                                      /**< Tab widget for different parameter types */
    QMap<VitalSync::ParameterType, QPushButton*> parameter_color_buttons_;       /**< Color selection buttons for each parameter type */
    QMap<VitalSync::ParameterType, QDoubleSpinBox*> parameter_low_critical_spin_boxes_; /**< Critical low limit settings for each parameter */
    QMap<VitalSync::ParameterType, QDoubleSpinBox*> parameter_low_warning_spin_boxes_;  /**< Warning low limit settings for each parameter */
    QMap<VitalSync::ParameterType, QDoubleSpinBox*> parameter_high_warning_spin_boxes_; /**< Warning high limit settings for each parameter */
    QMap<VitalSync::ParameterType, QDoubleSpinBox*> parameter_high_critical_spin_boxes_; /**< Critical high limit settings for each parameter */

    /**
     * @brief Color settings for various display elements
     */
    QColor background_color_;                                /**< Background color for waveform displays */
    QColor grid_color_;                                     /**< Grid color for waveform displays */
    QMap<VitalSync::WaveformType, QColor> waveforcolors__; /**< Colors for different waveform types */
    QMap<VitalSync::ParameterType, QColor> parameter_colors_; /**< Colors for different parameter types */

    /**
     * @brief Currently selected types for editing
     */
    VitalSync::WaveformType current_wavefortype__;     /**< Currently selected waveform type in the UI */
    VitalSync::ParameterType current_parameter_type_; /**< Currently selected parameter type in the UI */
};

#endif
