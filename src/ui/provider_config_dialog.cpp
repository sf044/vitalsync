/**
 * @file provider_config_dialog.cpp
 * @brief Implementation of the provider configuration dialog
 * 
 * This file implements the ProviderConfigDialog class, which provides a user interface
 * for configuring different types of data providers (Demo, Network, File). 
 * The dialog dynamically creates controls based on the provider type being configured.
 */

#include "provider_config_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "../../include/config_manager.h"
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>

/**
 * @brief Constructs the provider configuration dialog
 * 
 * Creates a dialog with controls specific to the given provider type
 * and initializes those controls with values from the configuration map.
 * 
 * @param providerName Name of the provider to configure (Demo, Network, File)
 * @param config Current configuration settings for the provider
 * @param parent Parent widget
 */
ProviderConfigDialog::ProviderConfigDialog(const QString& providerName, const QVariantMap& config, QWidget* parent)
    : QDialog(parent)
    , provider_name_(providerName)
    , config_(config)
{
    setWindowTitle(tr("Configure %1 Provider").arg(providerName));
    SetupUi();
    ApplyConfigToControls();
}

/**
 * @brief Destroys the provider configuration dialog
 */
ProviderConfigDialog::~ProviderConfigDialog()
{
}

/**
 * @brief Returns the updated configuration map
 * 
 * This method is called after the dialog is accepted to retrieve
 * the updated configuration with user modifications.
 * 
 * @return QVariantMap containing the updated provider configuration
 */
QVariantMap ProviderConfigDialog::GetConfig() const
{
    return config_;
}

/**
 * @brief Sets up the user interface
 * 
 * Creates and arranges all dialog components, selecting the appropriate
 * control set based on the provider type.
 */
void ProviderConfigDialog::SetupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create controls specific to the provider type
    CreateControls();
    
    // Provider info label
    QLabel* infoLabel = new QLabel(tr("Configure settings for %1 data provider").arg(provider_name_));
    infoLabel->setStyleSheet("font-weight: bold; margin-bottom: 10px;");
    
    // Button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ProviderConfigDialog::OnAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ProviderConfigDialog::OnReject);
    
    // Create content widget based on provider type
    QWidget* contentWidget = nullptr;
    
    if (provider_name_ == "Demo") {
        contentWidget = CreateDemoProviderControls();
    } else if (provider_name_ == "Network") {
        contentWidget = CreateNetworkProviderControls();
    } else if (provider_name_ == "File") {
        contentWidget = CreateFileProviderControls();
    } else {
        // Generic fallback for unknown providers
        QLabel* label = new QLabel(tr("No specific configuration available for this provider."));
        contentWidget = label;
    }
    
    // Add widgets to layout
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(contentWidget);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
    setMinimumWidth(500);
}

/**
 * @brief Creates empty control pointers
 * 
 * Initializes all UI control pointers to nullptr to prevent
 * accessing uninitialized memory when only some controls are created.
 */
void ProviderConfigDialog::CreateControls()
{
    // Initialize controls to nullptr so we don't try to access uninitialized pointers later
    heart_rate_spin_box_ = nullptr;
    respiration_rate_spin_box_ = nullptr;
    spo2_spin_box_ = nullptr;
    systolic_bp_spin_box_ = nullptr;
    diastolic_bp_spin_box_ = nullptr;
    temperature_spin_box_ = nullptr;
    etco2_spin_box_ = nullptr;
    update_interval_spin_box_ = nullptr;
    waveform_type_combo_box_ = nullptr;
    amplitude_spin_box_ = nullptr;
    frequency_spin_box_ = nullptr;
    noise_spin_box_ = nullptr;
    artifacts_check_box_ = nullptr;
    
    host_line_edit_ = nullptr;
    port_spin_box_ = nullptr;
    username_line_edit_ = nullptr;
    password_line_edit_ = nullptr;
    protocol_combo_box_ = nullptr;
    
    file_path_line_edit_ = nullptr;
    playback_speed_spin_box_ = nullptr;
    loop_check_box_ = nullptr;
}

/**
 * @brief Creates UI controls for the Demo provider
 * 
 * Builds a set of controls specific to the Demo provider, including
 * simulated vital signs, waveform characteristics, and update intervals.
 * 
 * @return Widget containing the Demo provider control set
 */
QWidget* ProviderConfigDialog::CreateDemoProviderControls()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Create tabs for different categories
    QTabWidget* tabWidget = new QTabWidget();
    
    // Parameters tab
    QWidget* parametersTab = new QWidget();
    QFormLayout* parametersLayout = new QFormLayout(parametersTab);
    
    heart_rate_spin_box_ = new QSpinBox();
    heart_rate_spin_box_->setRange(20, 200);
    heart_rate_spin_box_->setSuffix(tr(" bpm"));
    parametersLayout->addRow(tr("Heart Rate:"), heart_rate_spin_box_);
    
    respiration_rate_spin_box_ = new QSpinBox();
    respiration_rate_spin_box_->setRange(5, 40);
    respiration_rate_spin_box_->setSuffix(tr(" breaths/min"));
    parametersLayout->addRow(tr("Respiration Rate:"), respiration_rate_spin_box_);
    
    spo2_spin_box_ = new QSpinBox();
    spo2_spin_box_->setRange(70, 100);
    spo2_spin_box_->setSuffix(tr(" %"));
    parametersLayout->addRow(tr("SpO2:"), spo2_spin_box_);
    
    systolic_bp_spin_box_ = new QSpinBox();
    systolic_bp_spin_box_->setRange(60, 200);
    systolic_bp_spin_box_->setSuffix(tr(" mmHg"));
    parametersLayout->addRow(tr("Systolic BP:"), systolic_bp_spin_box_);
    
    diastolic_bp_spin_box_ = new QSpinBox();
    diastolic_bp_spin_box_->setRange(40, 120);
    diastolic_bp_spin_box_->setSuffix(tr(" mmHg"));
    parametersLayout->addRow(tr("Diastolic BP:"), diastolic_bp_spin_box_);
    
    temperature_spin_box_ = new QDoubleSpinBox();
    temperature_spin_box_->setRange(35.0, 41.0);
    temperature_spin_box_->setSingleStep(0.1);
    temperature_spin_box_->setSuffix(tr(" °C"));
    parametersLayout->addRow(tr("Temperature:"), temperature_spin_box_);
    
    etco2_spin_box_ = new QSpinBox();
    etco2_spin_box_->setRange(20, 60);
    etco2_spin_box_->setSuffix(tr(" mmHg"));
    parametersLayout->addRow(tr("EtCO2:"), etco2_spin_box_);
    
    // Waveform tab
    QWidget* waveformTab = new QWidget();
    QVBoxLayout* waveformLayout = new QVBoxLayout(waveformTab);
    
    QFormLayout* waveformFormLayout = new QFormLayout();
    
    waveform_type_combo_box_ = new QComboBox();
    waveform_type_combo_box_->addItem(tr("ECG"), "ecg");
    waveform_type_combo_box_->addItem(tr("Respiration"), "resp");
    waveform_type_combo_box_->addItem(tr("Plethysmograph"), "pleth");
    waveform_type_combo_box_->addItem(tr("Blood Pressure"), "abp");
    waveform_type_combo_box_->addItem(tr("Capnography"), "capno");
    connect(waveform_type_combo_box_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProviderConfigDialog::OnDemoWaveformTypeChanged);
    waveformFormLayout->addRow(tr("Waveform Type:"), waveform_type_combo_box_);
    
    amplitude_spin_box_ = new QDoubleSpinBox();
    amplitude_spin_box_->setRange(0.1, 5.0);
    amplitude_spin_box_->setSingleStep(0.1);
    waveformFormLayout->addRow(tr("Amplitude:"), amplitude_spin_box_);
    
    frequency_spin_box_ = new QDoubleSpinBox();
    frequency_spin_box_->setRange(0.1, 10.0);
    frequency_spin_box_->setSingleStep(0.1);
    frequency_spin_box_->setSuffix(tr(" Hz"));
    waveformFormLayout->addRow(tr("Frequency:"), frequency_spin_box_);
    
    noise_spin_box_ = new QDoubleSpinBox();
    noise_spin_box_->setRange(0.0, 1.0);
    noise_spin_box_->setSingleStep(0.05);
    waveformFormLayout->addRow(tr("Noise Level:"), noise_spin_box_);
    
    artifacts_check_box_ = new QCheckBox(tr("Include Artifacts"));
    waveformFormLayout->addRow("", artifacts_check_box_);
    
    waveformLayout->addLayout(waveformFormLayout);
    waveformLayout->addStretch();
    
    // Timing tab
    QWidget* timingTab = new QWidget();
    QFormLayout* timingLayout = new QFormLayout(timingTab);
    
    update_interval_spin_box_ = new QSpinBox();
    update_interval_spin_box_->setRange(10, 1000);
    update_interval_spin_box_->setSingleStep(5);
    update_interval_spin_box_->setSuffix(tr(" ms"));
    connect(update_interval_spin_box_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ProviderConfigDialog::OnDemoUpdateIntervalChanged);
    timingLayout->addRow(tr("Update Interval:"), update_interval_spin_box_);
    
    // Add tabs to tab widget
    tabWidget->addTab(parametersTab, tr("Parameters"));
    tabWidget->addTab(waveformTab, tr("Waveform"));
    tabWidget->addTab(timingTab, tr("Timing"));
    
    layout->addWidget(tabWidget);
    
    return widget;
}

/**
 * @brief Creates UI controls for the Network provider
 * 
 * Builds a set of controls specific to the Network provider, including
 * connection settings and protocol options.
 * 
 * @return Widget containing the Network provider control set
 */
QWidget* ProviderConfigDialog::CreateNetworkProviderControls()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Connection group
    QGroupBox* connectionGroup = new QGroupBox(tr("Connection Settings"));
    QFormLayout* connectionLayout = new QFormLayout(connectionGroup);
    
    host_line_edit_ = new QLineEdit();
    connectionLayout->addRow(tr("Host:"), host_line_edit_);
    
    port_spin_box_ = new QSpinBox();
    port_spin_box_->setRange(1, 65535);
    connectionLayout->addRow(tr("Port:"), port_spin_box_);
    
    protocol_combo_box_ = new QComboBox();
    protocol_combo_box_->addItem(tr("TCP"), "tcp");
    protocol_combo_box_->addItem(tr("UDP"), "udp");
    connectionLayout->addRow(tr("Protocol:"), protocol_combo_box_);
    
    // Authentication group
    QGroupBox* authGroup = new QGroupBox(tr("Authentication"));
    QFormLayout* authLayout = new QFormLayout(authGroup);
    
    username_line_edit_ = new QLineEdit();
    authLayout->addRow(tr("Username:"), username_line_edit_);
    
    password_line_edit_ = new QLineEdit();
    password_line_edit_->setEchoMode(QLineEdit::Password);
    authLayout->addRow(tr("Password:"), password_line_edit_);
    
    // Add groups to layout
    layout->addWidget(connectionGroup);
    layout->addWidget(authGroup);
    layout->addStretch();
    
    return widget;
}

/**
 * @brief Creates UI controls for the File provider
 * 
 * Builds a set of controls specific to the File provider, including
 * file path and playback options.
 * 
 * @return Widget containing the File provider control set
 */
QWidget* ProviderConfigDialog::CreateFileProviderControls()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // File selection
    QGroupBox* fileGroup = new QGroupBox(tr("File Settings"));
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    
    QHBoxLayout* filePathLayout = new QHBoxLayout();
    QLabel* filePathLabel = new QLabel(tr("Data File:"));
    file_path_line_edit_ = new QLineEdit();
    file_path_line_edit_->setReadOnly(true);
    QPushButton* browseButton = new QPushButton(tr("Browse..."));
    
    filePathLayout->addWidget(filePathLabel);
    filePathLayout->addWidget(file_path_line_edit_, 1);
    filePathLayout->addWidget(browseButton);
    
    connect(browseButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select Data File"), 
                                                      "", tr("Data Files (*.dat *.bin *.csv);;All Files (*.*)"));
        if (!filePath.isEmpty()) {
            file_path_line_edit_->setText(filePath);
        }
    });
    
    // Playback settings
    QFormLayout* playbackLayout = new QFormLayout();
    
    playback_speed_spin_box_ = new QDoubleSpinBox();
    playback_speed_spin_box_->setRange(0.1, 10.0);
    playback_speed_spin_box_->setSingleStep(0.1);
    playback_speed_spin_box_->setValue(1.0);
    playback_speed_spin_box_->setSuffix(tr("x"));
    playbackLayout->addRow(tr("Playback Speed:"), playback_speed_spin_box_);
    
    loop_check_box_ = new QCheckBox(tr("Loop Playback"));
    playbackLayout->addRow("", loop_check_box_);
    
    fileLayout->addLayout(filePathLayout);
    fileLayout->addLayout(playbackLayout);
    
    layout->addWidget(fileGroup);
    layout->addStretch();
    
    return widget;
}

/**
 * @brief Handles the accept action (OK button)
 * 
 * Validates input, updates the configuration map with values from
 * the UI controls, and accepts the dialog if validation passes.
 */
void ProviderConfigDialog::OnAccept()
{
    UpdateConfigFromControls();
    accept();
}

/**
 * @brief Handles the reject action (Cancel button)
 * 
 * Discards any changes and closes the dialog.
 */
void ProviderConfigDialog::OnReject()
{
    reject();
}

/**
 * @brief Handles waveform type selection changes
 * 
 * Updates the available waveform configuration controls when the
 * user selects a different waveform type in the Demo provider settings.
 * 
 * @param index Index of the selected waveform type
 */
void ProviderConfigDialog::OnDemoWaveformTypeChanged(int index)
{
    UpdateDemoWaveformControls(index);
}

/**
 * @brief Handles update interval changes
 * 
 * Updates any dependent controls when the user changes
 * the update interval for the Demo provider.
 * 
 * @param value New update interval value in milliseconds
 */
void ProviderConfigDialog::OnDemoUpdateIntervalChanged(int value)
{
    // Warn if update interval is too low
    if (value < 20) {
        QMessageBox::warning(this, tr("Performance Warning"),
                           tr("Setting update interval below 20ms may cause high CPU usage."));
    }
}

/**
 * @brief Updates UI elements based on waveform type
 * 
 * Adjusts the available settings and control ranges when the
 * waveform type changes in the Demo provider settings.
 * 
 * @param index Index of the selected waveform type
 */
void ProviderConfigDialog::UpdateDemoWaveformControls(int index)
{
    QString waveformType = waveform_type_combo_box_->itemData(index).toString();
    
    // Set appropriate ranges and defaults based on waveform type
    if (waveformType == "ecg") {
        amplitude_spin_box_->setRange(0.1, 3.0);
        amplitude_spin_box_->setValue(config_.value("ecgAmplitude", 1.0).toDouble());
        frequency_spin_box_->setRange(0.5, 3.0);
        frequency_spin_box_->setValue(config_.value("ecgFrequency", 1.2).toDouble());
        noise_spin_box_->setValue(config_.value("ecgNoise", 0.03).toDouble());
    } else if (waveformType == "resp") {
        amplitude_spin_box_->setRange(0.1, 2.0);
        amplitude_spin_box_->setValue(config_.value("respAmplitude", 0.5).toDouble());
        frequency_spin_box_->setRange(0.1, 0.5);
        frequency_spin_box_->setValue(config_.value("respFrequency", 0.25).toDouble());
        noise_spin_box_->setValue(config_.value("respNoise", 0.02).toDouble());
    } else if (waveformType == "pleth") {
        amplitude_spin_box_->setRange(0.1, 2.0);
        amplitude_spin_box_->setValue(config_.value("plethAmplitude", 1.0).toDouble());
        frequency_spin_box_->setRange(0.5, 3.0);
        frequency_spin_box_->setValue(config_.value("plethFrequency", 1.2).toDouble());
        noise_spin_box_->setValue(config_.value("plethNoise", 0.05).toDouble());
    } else if (waveformType == "abp") {
        amplitude_spin_box_->setRange(20.0, 60.0);
        amplitude_spin_box_->setValue(config_.value("abpAmplitude", 40.0).toDouble());
        frequency_spin_box_->setRange(0.5, 3.0);
        frequency_spin_box_->setValue(config_.value("abpFrequency", 1.2).toDouble());
        noise_spin_box_->setValue(config_.value("abpNoise", 0.02).toDouble());
    } else if (waveformType == "capno") {
        amplitude_spin_box_->setRange(20.0, 50.0);
        amplitude_spin_box_->setValue(config_.value("capnoAmplitude", 35.0).toDouble());
        frequency_spin_box_->setRange(0.1, 0.5);
        frequency_spin_box_->setValue(config_.value("capnoFrequency", 0.25).toDouble());
        noise_spin_box_->setValue(config_.value("capnoNoise", 0.02).toDouble());
    }
}

/**
 * @brief Updates the configuration map from UI controls
 * 
 * Reads the current values from all UI controls and updates
 * the configuration map accordingly.
 */
void ProviderConfigDialog::UpdateConfigFromControls()
{
    if (provider_name_ == "Demo") {
        // Get parameter values
        if (heart_rate_spin_box_)
            config_["heartRate"] = heart_rate_spin_box_->value();
        
        if (respiration_rate_spin_box_)
            config_["respirationRate"] = respiration_rate_spin_box_->value();
        
        if (spo2_spin_box_)
            config_["spo2"] = spo2_spin_box_->value();
        
        if (systolic_bp_spin_box_)
            config_["systolicBP"] = systolic_bp_spin_box_->value();
        
        if (diastolic_bp_spin_box_)
            config_["diastolicBP"] = diastolic_bp_spin_box_->value();
        
        if (temperature_spin_box_)
            config_["temperature"] = temperature_spin_box_->value();
        
        if (etco2_spin_box_)
            config_["etco2"] = etco2_spin_box_->value();
        
        if (update_interval_spin_box_)
            config_["UpdateInterval"] = update_interval_spin_box_->value();
        
        // Get waveform values
        if (waveform_type_combo_box_)
            config_["waveformType"] = waveform_type_combo_box_->currentData().toString();
        
        if (amplitude_spin_box_)
            config_["amplitude"] = amplitude_spin_box_->value();
        
        if (frequency_spin_box_)
            config_["frequency"] = frequency_spin_box_->value();
        
        if (noise_spin_box_)
            config_["noise"] = noise_spin_box_->value();
        
        if (artifacts_check_box_)
            config_["artifacts"] = artifacts_check_box_->isChecked();
        
    } else if (provider_name_ == "Network") {
        // Get network provider values
        if (host_line_edit_)
            config_["host"] = host_line_edit_->text();
        
        if (port_spin_box_)
            config_["port"] = port_spin_box_->value();
        
        if (protocol_combo_box_)
            config_["protocol"] = protocol_combo_box_->currentData().toString();
        
        if (username_line_edit_)
            config_["username"] = username_line_edit_->text();
        
        if (password_line_edit_)
            config_["password"] = password_line_edit_->text();
        
    } else if (provider_name_ == "File") {
        // Get file provider values
        if (file_path_line_edit_)
            config_["filePath"] = file_path_line_edit_->text();
        
        if (playback_speed_spin_box_)
            config_["playbackSpeed"] = playback_speed_spin_box_->value();
        
        if (loop_check_box_)
            config_["loop"] = loop_check_box_->isChecked();
    }
}

/**
 * @brief Applies configuration values to UI controls
 * 
 * Sets the values of all UI controls based on the current
 * configuration map.
 */
void ProviderConfigDialog::ApplyConfigToControls()
{
    if (provider_name_ == "Demo") {
        // Set parameter values
        if (heart_rate_spin_box_)
            heart_rate_spin_box_->setValue(config_.value("heartRate", 72).toInt());
        
        if (respiration_rate_spin_box_)
            respiration_rate_spin_box_->setValue(config_.value("respirationRate", 15).toInt());
        
        if (spo2_spin_box_)
            spo2_spin_box_->setValue(config_.value("spo2", 98).toInt());
        
        if (systolic_bp_spin_box_)
            systolic_bp_spin_box_->setValue(config_.value("systolicBP", 120).toInt());
        
        if (diastolic_bp_spin_box_)
            diastolic_bp_spin_box_->setValue(config_.value("diastolicBP", 80).toInt());
        
        if (temperature_spin_box_)
            temperature_spin_box_->setValue(config_.value("temperature", 37.0).toDouble());
        
        if (etco2_spin_box_)
            etco2_spin_box_->setValue(config_.value("etco2", 35).toInt());
        
        if (update_interval_spin_box_)
            update_interval_spin_box_->setValue(config_.value("UpdateInterval", 40).toInt());
        
        // Set waveform values
        if (waveform_type_combo_box_) {
            QString waveformType = config_.value("waveformType", "ecg").toString();
            int index = waveform_type_combo_box_->findData(waveformType);
            if (index >= 0) {
                waveform_type_combo_box_->setCurrentIndex(index);
            }
        }
        
        if (amplitude_spin_box_)
            amplitude_spin_box_->setValue(config_.value("amplitude", 1.0).toDouble());
        
        if (frequency_spin_box_)
            frequency_spin_box_->setValue(config_.value("frequency", 1.0).toDouble());
        
        if (noise_spin_box_)
            noise_spin_box_->setValue(config_.value("noise", 0.05).toDouble());
        
        if (artifacts_check_box_)
            artifacts_check_box_->setChecked(config_.value("artifacts", false).toBool());
        
    } else if (provider_name_ == "Network") {
        // Set network provider values
        if (host_line_edit_)
            host_line_edit_->setText(config_.value("host", "localhost").toString());
        
        if (port_spin_box_)
            port_spin_box_->setValue(config_.value("port", 5000).toInt());
        
        if (protocol_combo_box_) {
            QString protocol = config_.value("protocol", "tcp").toString();
            int index = protocol_combo_box_->findData(protocol);
            if (index >= 0) {
                protocol_combo_box_->setCurrentIndex(index);
            }
        }
        
        if (username_line_edit_)
            username_line_edit_->setText(config_.value("username", "").toString());
        
        if (password_line_edit_)
            password_line_edit_->setText(config_.value("password", "").toString());
        
    } else if (provider_name_ == "File") {
        // Set file provider values
        if (file_path_line_edit_)
            file_path_line_edit_->setText(config_.value("filePath", "").toString());
        
        if (playback_speed_spin_box_)
            playback_speed_spin_box_->setValue(config_.value("playbackSpeed", 1.0).toDouble());
        
        if (loop_check_box_)
            loop_check_box_->setChecked(config_.value("loop", true).toBool());
    }
} 