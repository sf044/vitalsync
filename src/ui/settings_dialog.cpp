/**
 * @file settings_dialog.cpp
 * @brief Implementation of the SettingsDialog class
 * 
 * This file contains the implementation of the SettingsDialog class, which provides
 * a user interface for configuring;
 * including display settings, waveform appearances, parameter configurations, and
 * alarm thresholds.
 */

#include "settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTabWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QPixmap>
#include <QIcon>
#include "../../include/config_manager.h"

/**
 * @brief Helper function to update a button's appearance with a color
 * 
 * Sets the button's background color and icon to match the specified color,
 * providing visual feedback for color selection buttons.
 * 
 * @param button The button to update
 * @param color The color to apply to the button
 */
void UpdateButtonColor(QPushButton* button, const QColor& color)
{
    if (!button) return;
    
    // Create a pixmap for the button icon
    QPixmap pixmap(button->size());
    pixmap.fill(color);
    
    // Set the icon
    button->setIcon(QIcon(pixmap));
    button->setIconSize(button->size());
    
    // Set a style sheet for the button background
    button->setStyleSheet(QString("QPushButton { background-color: %1; border: 1px solid #888; }")
                         .arg(color.name()));
}

/**
 * @brief Constructs the settings dialog
 * 
 * Initializes the dialog UI components, sets up the tab layout,
 * and loads the current settings from the configuration manager.
 * 
 * @param parent The parent widget for this dialog
 */
SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , current_wavefortype__(VitalSync::WaveformType::ECG_II)
    , current_parameter_type_(VitalSync::ParameterType::HR)
{
    setWindowTitle(tr("Application Settings"));
    setMinimumSize(800, 600);
    
    SetupUi();
    LoadSettings();
}

/**
 * @brief Destroys the settings dialog
 * 
 * Cleans up any resources allocated by the dialog.
 */
SettingsDialog::~SettingsDialog()
{
    // Clean up if needed
}

/**
 * @brief Sets up the user interface
 * 
 * Creates and arranges all dialog components, including the tab widget
 * with tabs for general settings, waveforms, parameters, and alarms.
 * Also sets up the dialog buttons and their signal connections.
 */
void SettingsDialog::SetupUi()
{
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Tab widget for settings categories
    QTabWidget* tabWidget = new QTabWidget();
    tabWidget->addTab(CreateGeneralTab(), tr("General"));
    tabWidget->addTab(CreateWaveformsTab(), tr("Waveforms"));
    tabWidget->addTab(CreateParametersTab(), tr("Parameters"));
    tabWidget->addTab(CreateAlarmsTab(), tr("Alarms"));
    
    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | 
                                                     QDialogButtonBox::Cancel | 
                                                     QDialogButtonBox::Reset);
    
    // Connect signals
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::OnAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::OnReject);
    connect(buttonBox, &QDialogButtonBox::clicked, [this, buttonBox](QAbstractButton* button) {
        if (button == buttonBox->button(QDialogButtonBox::Reset)) {
            OnResetToDefaults();
        }
    });
    
    // Add widgets to main layout
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
}

/**
 * @brief Creates the general settings tab
 * 
 * Builds the UI components for configuring general display settings such as
 * sweep speed, grid visibility, and color options.
 * 
 * @return A widget containing the general settings UI elements
 */
QWidget* SettingsDialog::CreateGeneralTab()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Create form layout for settings
    QFormLayout* formLayout = new QFormLayout();
    
    // Sweep speed
    sweep__speed_spin_box = new QDoubleSpinBox();
    sweep__speed_spin_box->setRange(5.0, 50.0);
    sweep__speed_spin_box->setSingleStep(1.0);
    sweep__speed_spin_box->setSuffix(tr(" mm/sec"));
    sweep__speed_spin_box->setToolTip(tr("Waveform sweep speed in mm per second"));
    formLayout->addRow(tr("Sweep Speed:"), sweep__speed_spin_box);
    
    // Grid options
    QGroupBox* gridGroup = new QGroupBox(tr("Grid Options"));
    QVBoxLayout* gridLayout = new QVBoxLayout(gridGroup);
    
    show__grid_checkBox = new QCheckBox(tr("Show Grid"));
    show__time_scale_check_box = new QCheckBox(tr("Show Time Scale"));
    show__amplitude_scale_check_box = new QCheckBox(tr("Show Amplitude Scale"));
    
    gridLayout->addWidget(show__grid_checkBox);
    gridLayout->addWidget(show__time_scale_check_box);
    gridLayout->addWidget(show__amplitude_scale_check_box);
    
    // Grid color
    QHBoxLayout* gridColorLayout = new QHBoxLayout();
    QLabel* gridColorLabel = new QLabel(tr("Grid Color:"));
    grid__color_button = new QPushButton();
    grid__color_button->setFixedSize(60, 30);
    grid__color_button->setToolTip(tr("Click to change grid color"));
    connect(grid__color_button, &QPushButton::clicked, this, &SettingsDialog::OnGridColorClicked);
    
    gridColorLayout->addWidget(gridColorLabel);
    gridColorLayout->addWidget(grid__color_button);
    gridColorLayout->addStretch();
    
    gridLayout->addLayout(gridColorLayout);
    
    // Background color
    QHBoxLayout* bgColorLayout = new QHBoxLayout();
    QLabel* bgColorLabel = new QLabel(tr("Background Color:"));
    background__color_button = new QPushButton();
    background__color_button->setFixedSize(60, 30);
    background__color_button->setToolTip(tr("Click to change background color"));
    connect(background__color_button, &QPushButton::clicked, this, &SettingsDialog::OnBackgroundColorClicked);
    
    bgColorLayout->addWidget(bgColorLabel);
    bgColorLayout->addWidget(background__color_button);
    bgColorLayout->addStretch();
    
    layout->addLayout(formLayout);
    layout->addWidget(gridGroup);
    layout->addLayout(bgColorLayout);
    layout->addStretch();
    
    return widget;
}

QWidget* SettingsDialog::CreateWaveformsTab()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Create tab widget for waveform types
    wavefortab__widget_ = new QTabWidget();
    
    // Create a tab for each waveform type
    const VitalSync::WaveformType waveformTypes[] = {
        VitalSync::WaveformType::ECG_I,
        VitalSync::WaveformType::ECG_II,
        VitalSync::WaveformType::ECG_III,
        VitalSync::WaveformType::RESP,
        VitalSync::WaveformType::PLETH,
        VitalSync::WaveformType::ABP,
        VitalSync::WaveformType::CAPNO
    };
    
    for (const auto& type : waveformTypes) {
        QString displayName = VitalSync::GetWaveformDisplayName(type);
        QWidget* waveformTab = new QWidget();
        QFormLayout* formLayout = new QFormLayout(waveformTab);
        
        // Waveform color
        QHBoxLayout* colorLayout = new QHBoxLayout();
        QLabel* colorLabel = new QLabel(tr("Color:"));
        QPushButton* colorButton = new QPushButton();
        colorButton->setFixedSize(60, 30);
        colorButton->setToolTip(tr("Click to change waveform color"));
        
        // Store the button
        waveforcolor__buttons_[type] = colorButton;
        
        // Create a closure to capture the waveform type
        connect(colorButton, &QPushButton::clicked, [this, type]() {
            QColor color = waveforcolors__[type];
            QColorDialog dialog(color, this);
            if (dialog.exec() == QDialog::Accepted) {
                waveforcolors__[type] = dialog.selectedColor();
                UpdateButtonColor(waveforcolor__buttons_[type], dialog.selectedColor());
            }
        });
        
        colorLayout->addWidget(colorLabel);
        colorLayout->addWidget(colorButton);
        colorLayout->addStretch();
        
        formLayout->addRow(tr("Color:"), colorButton);
        
        // Min/Max values
        QDoubleSpinBox* minSpinBox = new QDoubleSpinBox();
        QDoubleSpinBox* maxSpinBox = new QDoubleSpinBox();
        
        minSpinBox->setRange(-1000.0, 1000.0);
        maxSpinBox->setRange(-1000.0, 1000.0);
        
        // Store the spin boxes
        waveformin__spin_boxes_[type] = minSpinBox;
        waveformax__spin_boxes_[type] = maxSpinBox;
        
        formLayout->addRow(tr("Minimum Value:"), minSpinBox);
        formLayout->addRow(tr("Maximum Value:"), maxSpinBox);
        
        // Add the tab
        wavefortab__widget_->addTab(waveformTab, displayName);
    }
    
    // Connect tab change signal
    connect(wavefortab__widget_, &QTabWidget::currentChanged, this, &SettingsDialog::OnWaveformTabChanged);
    
    layout->addWidget(wavefortab__widget_);
    
    return widget;
}

QWidget* SettingsDialog::CreateParametersTab()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Create tab widget for parameter types
    parameter_tab_widget_ = new QTabWidget();
    
    // Create a tab for each parameter type
    const VitalSync::ParameterType parameterTypes[] = {
        VitalSync::ParameterType::HR,
        VitalSync::ParameterType::RR,
        VitalSync::ParameterType::SPO2,
        VitalSync::ParameterType::NIBP_SYS,
        VitalSync::ParameterType::NIBP_DIA,
        VitalSync::ParameterType::NIBP_MAP,
        VitalSync::ParameterType::ETCO2,
        VitalSync::ParameterType::TEMP1
    };
    
    for (const auto& type : parameterTypes) {
        QString displayName = VitalSync::GetParameterDisplayName(type);
        QWidget* parameterTab = new QWidget();
        QFormLayout* formLayout = new QFormLayout(parameterTab);
        
        // Parameter color
        QHBoxLayout* colorLayout = new QHBoxLayout();
        QLabel* colorLabel = new QLabel(tr("Color:"));
        QPushButton* colorButton = new QPushButton();
        colorButton->setFixedSize(60, 30);
        colorButton->setToolTip(tr("Click to change parameter color"));
        
        // Store the button
        parameter_color_buttons_[type] = colorButton;
        
        // Create a closure to capture the parameter type
        connect(colorButton, &QPushButton::clicked, [this, type]() {
            QColor color = parameter_colors_[type];
            QColorDialog dialog(color, this);
            if (dialog.exec() == QDialog::Accepted) {
                parameter_colors_[type] = dialog.selectedColor();
                UpdateButtonColor(parameter_color_buttons_[type], dialog.selectedColor());
            }
        });
        
        colorLayout->addWidget(colorLabel);
        colorLayout->addWidget(colorButton);
        colorLayout->addStretch();
        
        formLayout->addRow(tr("Color:"), colorButton);
        
        // Add the tab
        parameter_tab_widget_->addTab(parameterTab, displayName);
    }
    
    // Connect tab change signal
    connect(parameter_tab_widget_, &QTabWidget::currentChanged, this, &SettingsDialog::OnParameterTabChanged);
    
    layout->addWidget(parameter_tab_widget_);
    
    return widget;
}

QWidget* SettingsDialog::CreateAlarmsTab()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Tab widget for parameter alarms
    QTabWidget* alarmTabs = new QTabWidget();
    
    // Create a tab for each parameter type
    const VitalSync::ParameterType parameterTypes[] = {
        VitalSync::ParameterType::HR,
        VitalSync::ParameterType::RR,
        VitalSync::ParameterType::SPO2,
        VitalSync::ParameterType::NIBP_SYS,
        VitalSync::ParameterType::NIBP_DIA,
        VitalSync::ParameterType::NIBP_MAP,
        VitalSync::ParameterType::ETCO2,
        VitalSync::ParameterType::TEMP1
    };
    
    for (const auto& type : parameterTypes) {
        QString displayName = VitalSync::GetParameterDisplayName(type);
        QWidget* alarmTab = new QWidget();
        QFormLayout* formLayout = new QFormLayout(alarmTab);
        
        // Set up the spin boxes for alarm limits
        QDoubleSpinBox* lowCriticalSpinBox = new QDoubleSpinBox();
        QDoubleSpinBox* lowWarningSpinBox = new QDoubleSpinBox();
        QDoubleSpinBox* highWarningSpinBox = new QDoubleSpinBox();
        QDoubleSpinBox* highCriticalSpinBox = new QDoubleSpinBox();
        
        // Get default range for this parameter
        auto [min, max] = VitalSync::GetDefaultParameterRange(type);
        
        // Set ranges
        lowCriticalSpinBox->setRange(min, max);
        lowWarningSpinBox->setRange(min, max);
        highWarningSpinBox->setRange(min, max);
        highCriticalSpinBox->setRange(min, max);
        
        // Set units
        QString unit = VitalSync::GetParameterUnit(type);
        if (!unit.isEmpty()) {
            lowCriticalSpinBox->setSuffix(" " + unit);
            lowWarningSpinBox->setSuffix(" " + unit);
            highWarningSpinBox->setSuffix(" " + unit);
            highCriticalSpinBox->setSuffix(" " + unit);
        }
        
        // Store the spin boxes
        parameter_low_critical_spin_boxes_[type] = lowCriticalSpinBox;
        parameter_low_warning_spin_boxes_[type] = lowWarningSpinBox;
        parameter_high_warning_spin_boxes_[type] = highWarningSpinBox;
        parameter_high_critical_spin_boxes_[type] = highCriticalSpinBox;
        
        // Add to form layout
        formLayout->addRow(tr("Low Critical:"), lowCriticalSpinBox);
        formLayout->addRow(tr("Low Warning:"), lowWarningSpinBox);
        formLayout->addRow(tr("High Warning:"), highWarningSpinBox);
        formLayout->addRow(tr("High Critical:"), highCriticalSpinBox);
        
        // Add the tab
        alarmTabs->addTab(alarmTab, displayName);
    }
    
    layout->addWidget(alarmTabs);
    
    return widget;
}

/**
 * @brief Handler for accepting the dialog
 * 
 * Validates the settings, saves them to the ConfigManager, and closes
 * the dialog with an accept result.
 */
void SettingsDialog::OnAccept()
{
    SaveSettings();
    accept();
}

/**
 * @brief Handler for rejecting the dialog
 * 
 * Discards any changes and closes the dialog with a reject result.
 */
void SettingsDialog::OnReject()
{
    reject();
}

/**
 * @brief Handler for resetting settings to defaults
 * 
 * Resets all settings in the dialog to their default values
 * without saving the changes to the ConfigManager.
 */
void SettingsDialog::OnResetToDefaults()
{
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        tr("Reset Settings"),
        tr("Are you sure you want to reset all settings to default values?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (result == QMessageBox::Yes) {
        ConfigManager::GetInstance().resetToDefaults();
        LoadSettings();
    }
}

/**
 * @brief Handler for background color button click
 * 
 * Opens a color dialog to select a new background color
 * for waveform displays.
 */
void SettingsDialog::OnBackgroundColorClicked()
{
    QColorDialog dialog(background_color_, this);
    if (dialog.exec() == QDialog::Accepted) {
        background_color_ = dialog.selectedColor();
        UpdateButtonColor(background__color_button, background_color_);
    }
}

/**
 * @brief Handler for grid color button click
 * 
 * Opens a color dialog to select a new grid color
 * for waveform displays.
 */
void SettingsDialog::OnGridColorClicked()
{
    QColorDialog dialog(grid_color_, this);
    if (dialog.exec() == QDialog::Accepted) {
        grid_color_ = dialog.selectedColor();
        UpdateButtonColor(grid__color_button, grid_color_);
    }
}

/**
 * @brief Handler for waveform tab changes
 * 
 * Updates the UI when the user selects a different waveform type
 * in the waveform configuration tab.
 * 
 * @param index The new tab index (corresponds to waveform type)
 */
void SettingsDialog::OnWaveformTabChanged(int index)
{
    // Update current waveform type based on tab index
    const VitalSync::WaveformType waveformTypes[] = {
        VitalSync::WaveformType::ECG_I,
        VitalSync::WaveformType::ECG_II,
        VitalSync::WaveformType::ECG_III,
        VitalSync::WaveformType::RESP,
        VitalSync::WaveformType::PLETH,
        VitalSync::WaveformType::ABP,
        VitalSync::WaveformType::CAPNO
    };
    
    if (index >= 0 && index < static_cast<int>(std::size(waveformTypes))) {
        current_wavefortype__ = waveformTypes[index];
    }
}

/**
 * @brief Handler for parameter tab changes
 * 
 * Updates the UI when the user selects a different parameter type
 * in the parameter configuration tab.
 * 
 * @param index The new tab index (corresponds to parameter type)
 */
void SettingsDialog::OnParameterTabChanged(int index)
{
    // Update current parameter type based on tab index
    const VitalSync::ParameterType parameterTypes[] = {
        VitalSync::ParameterType::HR,
        VitalSync::ParameterType::RR,
        VitalSync::ParameterType::SPO2,
        VitalSync::ParameterType::NIBP_SYS,
        VitalSync::ParameterType::NIBP_DIA,
        VitalSync::ParameterType::NIBP_MAP,
        VitalSync::ParameterType::ETCO2,
        VitalSync::ParameterType::TEMP1
    };
    
    if (index >= 0 && index < static_cast<int>(std::size(parameterTypes))) {
        current_parameter_type_ = parameterTypes[index];
    }
}

/**
 * @brief Loads settings from the ConfigManager
 * 
 * Retrieves current configuration values from the ConfigManager
 * and updates UI elements with these values.
 */
void SettingsDialog::LoadSettings()
{
    ConfigManager& config = ConfigManager::GetInstance();
    
    // Load general settings
    double sweepSpeed = config.GetDouble("ui/defaultSweepSpeed", 25.0);
    grid_color_ = config.GetColor("ui/defaultGridColor", QColor(30, 30, 30));
    background_color_ = config.GetColor("ui/defaultBackgroundColor", Qt::black);
    
    bool showGrid = config.GetBool("ui/showGrid", true);
    bool showTimeScale = config.GetBool("ui/showTimeScale", true);
    bool showAmplitudeScale = config.GetBool("ui/showAmplitudeScale", true);
    
    // Set general UI values
    sweep__speed_spin_box->setValue(sweepSpeed);
    UpdateButtonColor(grid__color_button, grid_color_);
    UpdateButtonColor(background__color_button, background_color_);
    
    show__grid_checkBox->setChecked(showGrid);
    show__time_scale_check_box->setChecked(showTimeScale);
    show__amplitude_scale_check_box->setChecked(showAmplitudeScale);
    
    // Load waveform settings
    for (const auto& [type, colorButton] : waveforcolor__buttons_.toStdMap()) {
        QVariantMap waveformConfig = config.GetWaveformConfig(type);
        QColor color = waveformConfig.value("color", QColor(0, 255, 0)).value<QColor>();
        waveforcolors__[type] = color;
        UpdateButtonColor(colorButton, color);
        
        double minValue = waveformConfig.value("minValue", -1.0).toDouble();
        double maxValue = waveformConfig.value("maxValue", 1.0).toDouble();
        
        if (waveformin__spin_boxes_.contains(type)) {
            waveformin__spin_boxes_[type]->setValue(minValue);
        }
        
        if (waveformax__spin_boxes_.contains(type)) {
            waveformax__spin_boxes_[type]->setValue(maxValue);
        }
    }
    
    // Load parameter settings
    for (const auto& [type, colorButton] : parameter_color_buttons_.toStdMap()) {
        QVariantMap parameterConfig = config.GetParameterConfig(type);
        QColor color = parameterConfig.value("color", QColor(255, 255, 255)).value<QColor>();
        parameter_colors_[type] = color;
        UpdateButtonColor(colorButton, color);
        
        // Load alarm limits
        auto [lowCritical, lowWarning, highWarning, highCritical] = VitalSync::GetDefaultAlarmLimits(type);
        
        double lowCriticalValue = parameterConfig.value("lowCritical", lowCritical).toDouble();
        double lowWarningValue = parameterConfig.value("lowWarning", lowWarning).toDouble();
        double highWarningValue = parameterConfig.value("highWarning", highWarning).toDouble();
        double highCriticalValue = parameterConfig.value("highCritical", highCritical).toDouble();
        
        if (parameter_low_critical_spin_boxes_.contains(type)) {
            parameter_low_critical_spin_boxes_[type]->setValue(lowCriticalValue);
        }
        
        if (parameter_low_warning_spin_boxes_.contains(type)) {
            parameter_low_warning_spin_boxes_[type]->setValue(lowWarningValue);
        }
        
        if (parameter_high_warning_spin_boxes_.contains(type)) {
            parameter_high_warning_spin_boxes_[type]->setValue(highWarningValue);
        }
        
        if (parameter_high_critical_spin_boxes_.contains(type)) {
            parameter_high_critical_spin_boxes_[type]->setValue(highCriticalValue);
        }
    }
}

/**
 * @brief Saves settings to the ConfigManager
 * 
 * Retrieves values from UI elements and saves them to the ConfigManager
 * to make them persistent across application sessions.
 */
void SettingsDialog::SaveSettings()
{
    ConfigManager& config = ConfigManager::GetInstance();
    
    // Save general settings
    config.SetDouble("ui/defaultSweepSpeed", sweep__speed_spin_box->value());
    config.setColor("ui/defaultGridColor", grid_color_);
    config.setColor("ui/defaultBackgroundColor", background_color_);
    
    config.SetBool("ui/showGrid", show__grid_checkBox->isChecked());
    config.SetBool("ui/showTimeScale", show__time_scale_check_box->isChecked());
    config.SetBool("ui/showAmplitudeScale", show__amplitude_scale_check_box->isChecked());
    
    // Save waveform settings
    for (const auto& [type, colorButton] : waveforcolor__buttons_.toStdMap()) {
        QVariantMap waveformConfig = config.GetWaveformConfig(type);
        waveformConfig["color"] = waveforcolors__[type];
        
        if (waveformin__spin_boxes_.contains(type) && waveformax__spin_boxes_.contains(type)) {
            waveformConfig["minValue"] = waveformin__spin_boxes_[type]->value();
            waveformConfig["maxValue"] = waveformax__spin_boxes_[type]->value();
        }
        
        config.SetWaveformConfig(type, waveformConfig);
    }
    
    // Save parameter settings
    for (const auto& [type, colorButton] : parameter_color_buttons_.toStdMap()) {
        QVariantMap parameterConfig = config.GetParameterConfig(type);
        parameterConfig["color"] = parameter_colors_[type];
        
        if (parameter_low_critical_spin_boxes_.contains(type) && 
            parameter_low_warning_spin_boxes_.contains(type) &&
            parameter_high_warning_spin_boxes_.contains(type) &&
            parameter_high_critical_spin_boxes_.contains(type)) {
            
            parameterConfig["lowCritical"] = parameter_low_critical_spin_boxes_[type]->value();
            parameterConfig["lowWarning"] = parameter_low_warning_spin_boxes_[type]->value();
            parameterConfig["highWarning"] = parameter_high_warning_spin_boxes_[type]->value();
            parameterConfig["highCritical"] = parameter_high_critical_spin_boxes_[type]->value();
        }
        
        config.SetParameterConfig(type, parameterConfig);
    }
    
    // Save configuration to disk
    config.save();
}

/**
 * @brief Gets the configured sweep speed
 * 
 * @return The sweep speed in pixels per second
 */
double SettingsDialog::GetSweepSpeed() const
{
    return sweep__speed_spin_box->value();
}

/**
 * @brief Sets the sweep speed
 * 
 * @param pixelsPerSecond The sweep speed in pixels per second
 */
void SettingsDialog::SetSweepSpeed(double pixelsPerSecond)
{
    sweep__speed_spin_box->setValue(pixelsPerSecond);
}

/**
 * @brief Gets the configured grid color
 * 
 * @return The grid color
 */
QColor SettingsDialog::GetGridColor() const
{
    return grid_color_;
}

/**
 * @brief Sets the grid color
 * 
 * @param color The grid color to use
 */
void SettingsDialog::SetGridColor(const QColor& color)
{
    grid_color_ = color;
    UpdateButtonColor(grid__color_button, color);
}

/**
 * @brief Gets the configured background color
 * 
 * @return The background color
 */
QColor SettingsDialog::GetBackgroundColor() const
{
    return background_color_;
}

/**
 * @brief Sets the background color
 * 
 * @param color The background color to use
 */
void SettingsDialog::SetBackgroundColor(const QColor& color)
{
    background_color_ = color;
    UpdateButtonColor(background__color_button, color);
}

/**
 * @brief Gets the configured text color
 * 
 * @return The text color
 */
QColor SettingsDialog::GetTextColor() const
{
    // This is typically the inverse of the background color
    // but could be configurable separately
    return QColor(255 - background_color_.red(),
                  255 - background_color_.green(),
                  255 - background_color_.blue());
}

/**
 * @brief Sets the text color
 * 
 * @param color The text color to use
 */
void SettingsDialog::SetTextColor(const QColor& color)
{
    // This might be used in the future if text color becomes independently configurable
    Q_UNUSED(color);
} 