/**
 * @file main_window.cpp
 * @brief Implementation of the MainWindow class
 * 
 * This file contains the implementation of the MainWindow class, which provides
 * the main user interface.
 * It manages the display of physiological waveforms and vital sign parameters,
 * handles user interactions, and coordinates with the data management system.
 */

#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSplitter>
#include <QVariantMap>
#include <QDebug>

#include "../../include/config_manager.h"
#include "../core/data_manager.h"
#include "waveforms/waveform_view.h"
#include "parameters/parameter_view.h"
#include "settings_dialog.h"
#include "provider_config_dialog.h"

/**
 * @namespace Anonymous
 * @brief Contains constants for default view configurations
 */
namespace {
    const int DEFAULT_WINDOW_WIDTH = 1200;    /**< Default window width in pixels */
    const int DEFAULT_WINDOW_HEIGHT = 800;    /**< Default window height in pixels */
    const int PARAMETER_VIEW_WIDTH = 150;     /**< Default parameter view width in pixels */
    const int PARAMETER_VIEW_HEIGHT = 100;    /**< Default parameter view height in pixels */
}

/**
 * @brief Constructs the main window
 * 
 * Initializes the data manager, sets up the UI components, establishes signal connections,
 * populates the provider selector, and auto-starts the default provider if available.
 * 
 * @param parent The parent widget (default: nullptr)
 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , is_acquiring_(false)
    , connection_status_(VitalSync::ConnectionStatus::Disconnected)
{
    // Initialize the data manager
    data_manager_ = std::make_shared<DataManager>();

    // Set up the UI
    SetupUi();
    SetupWaveformViews();
    SetupParameterViews();
    connectSignals();
    
    // Initialize with default settings
    ApplyDefaultSettings();
    
    // Initialize the data manager
    if (!data_manager_->initialize()) {
        QMessageBox::critical(this, tr("Initialization Error"),
                            tr("Failed to initialize the data manager."));
    }
    
    // Populate provider selector with available providers
    std::vector<std::string> providers = data_manager_->GetAvailableProviders();
    for (const auto& provider : providers) {
        provider_selector_->addItem(QString::fromStdString(provider));
    }
    
    // Get last used provider from config and select it
    QString lastProvider = ConfigManager::GetInstance().GetLastProvider();
    int lastIndex = provider_selector_->findText(lastProvider);
    if (lastIndex >= 0) {
        provider_selector_->setCurrentIndex(lastIndex);
    }
    
    // Connect models to views
    connectWaveformModels();
    connectParameterModels();
    
    // Auto-start the demo provider
    if (providers.size() > 0) {
        qDebug() << "MainWindow: Auto-starting data acquisition...";
        if (data_manager_->startAcquisition()) {
            is_acquiring_ = true;
            start_stop_button_->setText(tr("Stop"));
            // Unpause all waveform views
            for (auto& [type, view] : waveform_views_.toStdMap()) {
                view->SetPaused(false);
            }
            // Get current connection status after auto-start
            connection_status_ = data_manager_->GetCurrentProvider()->GetConnectionStatus();
            qDebug() << "MainWindow: Auto-start successful, connection status:" << static_cast<int>(connection_status_);
        } else {
            qWarning() << "MainWindow: Auto-start failed";
        }
    }
    
    // Update connection status display
    UpdateConnectionStatus(connection_status_);
}

/**
 * @brief Destroys the main window
 * 
 * Stops data acquisition if it's currently running before destroying the window.
 */
MainWindow::~MainWindow()
{
    // Stop acquisition if it's running
    if (is_acquiring_) {
        data_manager_->stopAcquisition();
    }
}

/**
 * @brief Handles the window close event
 * 
 * Performs cleanup operations when the window is being closed, including
 * stopping data acquisition and saving configuration settings.
 * 
 * @param event The close event
 */
void MainWindow::closeEvent(QCloseEvent* event)
{
    // Stop acquisition before closing
    if (is_acquiring_) {
        data_manager_->stopAcquisition();
    }
    
    // Save current configuration
    ConfigManager::GetInstance().save();
    
    // Accept the close event
    event->accept();
}

/**
 * @brief Sets up the user interface components
 * 
 * Creates and configures the main window layout, toolbar, status bar,
 * and control elements. Establishes the visual hierarchy of the UI.
 */
void MainWindow::SetupUi()
{
    // Set window properties
    setWindowTitle(tr("VitalSync"));
    resize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    // Create main widget and layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Create toolbar for controls
    QToolBar* controlBar = new QToolBar(tr("Controls"), this);
    controlBar->setMovable(false);
    addToolBar(controlBar);
    
    // Provider selector
    provider_selector_ = new QComboBox(this);
    provider_selector_->setMinimumWidth(150);
    controlBar->addWidget(new QLabel(tr("Data Source:")));
    controlBar->addWidget(provider_selector_);
    
    // Configure button
    configure_button_ = new QPushButton(tr("Configure"), this);
    controlBar->addWidget(configure_button_);
    
    // Add separator
    controlBar->addSeparator();
    
    // Start/Stop button
    start_stop_button_ = new QPushButton(tr("Start"), this);
    controlBar->addWidget(start_stop_button_);
    
    // Add spacer
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    controlBar->addWidget(spacer);
    
    // Settings button
    settings_button_ = new QPushButton(tr("Settings"), this);
    controlBar->addWidget(settings_button_);
    
    // Create a splitter for waveforms and parameters
    QSplitter* splitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // Create scrollable area for waveforms
    QScrollArea* waveformScrollArea = new QScrollArea(splitter);
    waveformScrollArea->setWidgetResizable(true);
    waveformScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    waveformScrollArea->setFrameShape(QFrame::NoFrame);
    
    // Create container for waveforms
    QWidget* waveformContainer = new QWidget(waveformScrollArea);
    QVBoxLayout* waveformLayout = new QVBoxLayout(waveformContainer);
    waveformLayout->setContentsMargins(0, 0, 0, 0);
    waveformLayout->setSpacing(1);
    waveformScrollArea->setWidget(waveformContainer);
    
    // Create container for parameter views
    QWidget* parameterContainer = new QWidget(splitter);
    QGridLayout* parameterLayout = new QGridLayout(parameterContainer);
    parameterLayout->setContentsMargins(1, 1, 1, 1);
    parameterLayout->setSpacing(1);
    
    // Add the containers to the splitter
    splitter->addWidget(waveformScrollArea);
    splitter->addWidget(parameterContainer);
    
    // Set initial splitter sizes (70% waveforms, 30% parameters)
    QList<int> sizes;
    sizes << (DEFAULT_WINDOW_WIDTH * 0.7) << (DEFAULT_WINDOW_WIDTH * 0.3);
    splitter->setSizes(sizes);
    
    // Add the splitter to the main layout
    mainLayout->addWidget(splitter);
    
    // Create status bar
    statusBar()->showMessage(tr("Ready"));
    
    // Status label
    status_label_ = new QLabel(tr("Status:"));
    statusBar()->addPermanentWidget(status_label_);
    
    // Connection status
    connection_status_label_ = new QLabel(tr("Disconnected"));
    connection_status_label_->setStyleSheet("QLabel { color: red; }");
    statusBar()->addPermanentWidget(connection_status_label_);
    
    // Set central widget
    setCentralWidget(centralWidget);
    
    // Store layout pointers for later use when adding views
    waveformContainer->setProperty("layout", QVariant::fromValue(waveformLayout));
    parameterContainer->setProperty("layout", QVariant::fromValue(parameterLayout));
}

/**
 * @brief Sets up the waveform view components
 * 
 * Creates visualization components for each supported waveform type,
 * configures their initial appearance, and arranges them in the layout.
 */
void MainWindow::SetupWaveformViews()
{
    // Get the waveform container
    QWidget* centralWidget = this->centralWidget();
    QSplitter* splitter = qobject_cast<QSplitter*>(centralWidget->layout()->itemAt(0)->widget());
    QScrollArea* waveformScrollArea = qobject_cast<QScrollArea*>(splitter->widget(0));
    QWidget* waveformContainer = waveformScrollArea->widget();
    QVBoxLayout* waveformLayout = qobject_cast<QVBoxLayout*>(waveformContainer->layout());
    
    // Create waveform views for each waveform type we want to display
    // We'll use an array of waveform types we want to show
    const VitalSync::WaveformType waveformTypes[] = {
        VitalSync::WaveformType::ECG_II,
        VitalSync::WaveformType::RESP,
        VitalSync::WaveformType::PLETH,
        VitalSync::WaveformType::ABP,
        VitalSync::WaveformType::CAPNO
    };
    
    // Create a view for each waveform type
    for (const auto& type : waveformTypes) {
        // Create waveform view
        auto waveformView = std::make_shared<WaveformView>();
        
        // Set initial properties
        QString displayName = VitalSync::GetWaveformDisplayName(type);
        auto [minValue, maxValue] = VitalSync::GetDefaultWaveformRange(type);
        
        // Add to layout
        waveformLayout->addWidget(waveformView->GetWidget());
        
        // Store in map for later access
        waveform_views_[type] = waveformView;
    }
}

/**
 * @brief Sets up the parameter view components
 * 
 * Creates display components for each supported vital sign parameter,
 * configures their initial appearance, and arranges them in the layout.
 */
void MainWindow::SetupParameterViews()
{
    // Get the parameter container
    QWidget* centralWidget = this->centralWidget();
    QSplitter* splitter = qobject_cast<QSplitter*>(centralWidget->layout()->itemAt(0)->widget());
    QWidget* parameterContainer = splitter->widget(1);
    QGridLayout* parameterLayout = qobject_cast<QGridLayout*>(parameterContainer->layout());
    
    // Create parameter views for each parameter type we want to display
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
    
    // Calculate grid layout
    const int numColumns = 2;
    
    // Create a view for each parameter type
    for (size_t i = 0; i < sizeof(parameterTypes) / sizeof(parameterTypes[0]); ++i) {
        auto type = parameterTypes[i];
        
        // Create parameter view
        auto parameterView = std::make_shared<ParameterView>();
        
        // Add to layout
        int row = i / numColumns;
        int col = i % numColumns;
        parameterLayout->addWidget(parameterView->GetWidget(), row, col);
        
        // Set size
        parameterView->GetWidget()->setMinimumSize(PARAMETER_VIEW_WIDTH, PARAMETER_VIEW_HEIGHT);
        
        // Store in map for later access
        parameter_views_[type] = parameterView;
    }
}

/**
 * @brief Connects signals and slots between components
 * 
 * Establishes signal-slot connections between UI elements, data manager,
 * and various models and views to enable event-driven interactions.
 */
void MainWindow::connectSignals()
{
    // Connect UI signals
    connect(start_stop_button_, &QPushButton::clicked, this, &MainWindow::OnStartStopButtonClicked);
    connect(configure_button_, &QPushButton::clicked, this, &MainWindow::OnConfigureProviderClicked);
    connect(settings_button_, &QPushButton::clicked, this, &MainWindow::OnSettingsButtonClicked);
    connect(provider_selector_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::OnProviderSelectionChanged);
    
    // Connect data manager signals
    connect(data_manager_.get(), &IDataManager::connectionStatusChanged, 
            this, &MainWindow::OnConnectionStatusChanged);
    connect(data_manager_.get(), &IDataManager::errorOccurred, 
            this, &MainWindow::OnErrorOccurred);
}

/**
 * @brief Handles the start/stop button click event
 * 
 * Toggles the data acquisition state. If currently acquiring, stops acquisition.
 * If not acquiring, starts acquisition using the selected provider.
 * Updates UI state to reflect the current acquisition status.
 * 
 * Exception handling is implemented to prevent crashes due to acquisition errors.
 */
void MainWindow::OnStartStopButtonClicked()
{
    // Disable the button temporarily to prevent multiple clicks
    start_stop_button_->setEnabled(false);
    
    try {
        if (is_acquiring_) {
            // Stop acquisition
            qDebug() << "MainWindow: Stopping data acquisition...";
            data_manager_->stopAcquisition();
            start_stop_button_->setText(tr("Start"));
            is_acquiring_ = false;
            
            // Pause all waveform views
            for (auto& [type, view] : waveform_views_.toStdMap()) {
                view->SetPaused(true);
            }
            
            qDebug() << "MainWindow: Data acquisition stopped successfully";
        } else {
            // Check if we have a provider selected
            if (provider_selector_->currentIndex() < 0) {
                QMessageBox::warning(this, tr("No Provider"),
                                    tr("Please select a data provider first."));
                start_stop_button_->setEnabled(true);
                return;
            }
            
            // Start acquisition
            qDebug() << "MainWindow: Starting data acquisition...";
            bool success = data_manager_->startAcquisition();
            
            if (success) {
                start_stop_button_->setText(tr("Stop"));
                is_acquiring_ = true;
                
                // Unpause all waveform views
                for (auto& [type, view] : waveform_views_.toStdMap()) {
                    view->SetPaused(false);
                }
                
                qDebug() << "MainWindow: Data acquisition started successfully";
            } else {
                QMessageBox::critical(this, tr("Start Acquisition Failed"),
                                    tr("Failed to start data acquisition."));
                qDebug() << "MainWindow: Failed to start data acquisition";
            }
        }
    } catch (const std::exception& e) {
        // Handle any exceptions
        QMessageBox::critical(this, tr("Error"),
                            tr("An error occurred: %1").arg(e.what()));
        qDebug() << "MainWindow: Exception in start/stop handler:" << e.what();
    } catch (...) {
        // Handle any other exceptions
        QMessageBox::critical(this, tr("Error"),
                            tr("An unknown error occurred."));
        qDebug() << "MainWindow: Unknown exception in start/stop handler";
    }
    
    // Re-enable the button
    start_stop_button_->setEnabled(true);
}

/**
 * @brief Handles changes in the provider selection
 * 
 * Updates the active data provider when the user selects a different provider
 * from the dropdown. Stops any current acquisition, sets the new provider as active,
 * and updates the configuration and UI state.
 * 
 * @param index The index of the selected provider in the combo box
 */
void MainWindow::OnProviderSelectionChanged(int index)
{
    if (index >= 0) {
        QString providerName = provider_selector_->itemText(index);
        current_provider_name_ = providerName;
        
        // Stop acquisition if it's running
        if (is_acquiring_) {
            data_manager_->stopAcquisition();
            start_stop_button_->setText(tr("Start"));
            is_acquiring_ = false;
        }
        
        // Set the active provider
        if (data_manager_->SetActiveProvider(providerName.toStdString())) {
            // Save the selected provider
            ConfigManager::GetInstance().SetLastProvider(providerName);
            
            // Connect models to views (in case they changed)
            connectWaveformModels();
            connectParameterModels();
            
            // Update the connection status
            auto provider = data_manager_->GetCurrentProvider();
            if (provider) {
                UpdateConnectionStatus(provider->GetConnectionStatus());
            } else {
                UpdateConnectionStatus(VitalSync::ConnectionStatus::Disconnected);
            }
        } else {
            QMessageBox::warning(this, tr("Provider Selection"),
                                tr("Failed to select the provider."));
        }
    }
}

/**
 * @brief Handles connection status changes
 * 
 * Responds to changes in the connection status of the current data provider
 * by updating the UI elements to reflect the new status.
 * 
 * @param status The new connection status
 */
void MainWindow::OnConnectionStatusChanged(VitalSync::ConnectionStatus status)
{
    UpdateConnectionStatus(status);
}

/**
 * @brief Updates UI elements to reflect the current connection status
 * 
 * Updates status indicators, button states, and other UI elements based on
 * the current connection status of the data provider.
 * 
 * @param status The current connection status
 */
void MainWindow::UpdateConnectionStatus(VitalSync::ConnectionStatus status)
{
    connection_status_ = status;
    
    // Update status display
    QString statusText;
    QString statusStyle;
    
    switch (status) {
        case VitalSync::ConnectionStatus::Connected:
            statusText = tr("Connected");
            statusStyle = "QLabel { color: green; }";
            break;
        case VitalSync::ConnectionStatus::Connecting:
            statusText = tr("Connecting...");
            statusStyle = "QLabel { color: orange; }";
            break;
        case VitalSync::ConnectionStatus::Disconnected:
            statusText = tr("Disconnected");
            statusStyle = "QLabel { color: red; }";
            break;
        case VitalSync::ConnectionStatus::Error:
            statusText = tr("Error");
            statusStyle = "QLabel { color: darkred; }";
            break;
        default:
            statusText = tr("Unknown");
            statusStyle = "QLabel { color: gray; }";
            break;
    }
    
    connection_status_label_->setText(statusText);
    connection_status_label_->setStyleSheet(statusStyle);
}

/**
 * @brief Handles provider error events
 * 
 * Displays error messages from the data provider and takes appropriate action
 * based on the error severity, such as stopping acquisition for critical errors.
 * 
 * @param errorCode The error code indicating the type of error
 * @param errorMessage The descriptive error message
 */
void MainWindow::OnErrorOccurred(int errorCode, const QString& errorMessage)
{
    // Display error in status bar
    statusBar()->showMessage(tr("Error: %1").arg(errorMessage), 5000);
    
    // For critical errors, show a message box
    if (errorCode >= static_cast<int>(VitalSync::ErrorCode::CriticalError)) {
        QMessageBox::critical(this, tr("Critical Error"),
                            tr("A critical error occurred: %1").arg(errorMessage));
    }
}

/**
 * @brief Opens the provider configuration dialog
 * 
 * Displays a dialog allowing the user to configure settings for the
 * currently selected data provider.
 */
void MainWindow::OnConfigureProviderClicked()
{
    // Get the current provider
    auto provider = data_manager_->GetCurrentProvider();
    if (!provider) {
        QMessageBox::warning(this, tr("No Provider"),
                            tr("Please select a data provider first."));
        return;
    }
    
    // Get current provider configuration
    QVariantMap currentConfig = ConfigManager::GetInstance().GetProviderConfig(current_provider_name_);
    
    // Show configuration dialog
    ProviderConfigDialog dialog(current_provider_name_, currentConfig, this);
    if (dialog.exec() == QDialog::Accepted) {
        // Get updated configuration
        QVariantMap config = dialog.GetConfig();
        
        // Configure the provider
        if (data_manager_->configureCurrentProvider(config)) {
            // Save the configuration
            ConfigManager::GetInstance().SetProviderConfig(current_provider_name_, config);
            statusBar()->showMessage(tr("Provider configured successfully"), 3000);
        } else {
            QMessageBox::warning(this, tr("Configuration Failed"),
                                tr("Failed to configure the provider."));
        }
    }
}

/**
 * @brief Opens the application settings dialog
 * 
 * Displays a dialog allowing the user to configure general application settings,
 * including display preferences and alarm thresholds.
 */
void MainWindow::OnSettingsButtonClicked()
{
    // Show settings dialog
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Apply settings to views
        ApplyDefaultSettings();
        statusBar()->showMessage(tr("Settings updated"), 3000);
    }
}

/**
 * @brief Connects waveform models to their corresponding views
 * 
 * Establishes signal-slot connections between waveform data models and
 * their visualization components to enable real-time updates.
 */
void MainWindow::connectWaveformModels()
{
    // Get all waveform models from the data manager
    auto allWaveformModels = data_manager_->GetAllWaveformModels();
    
    // Connect each model to the corresponding view
    for (const auto& model : allWaveformModels) {
        if (model) {
            // Get waveform ID
            VitalSync::WaveformType type = static_cast<VitalSync::WaveformType>(model->GetWaveformId());
            
            // Find the corresponding view
            auto viewIt = waveform_views_.find(type);
            if (viewIt != waveform_views_.end()) {
                // Connect model to view
                viewIt.value()->SetModel(model);
            }
        }
    }
}

/**
 * @brief Connects parameter models to their corresponding views
 * 
 * Establishes signal-slot connections between parameter models and
 * their display components to enable real-time updates.
 */
void MainWindow::connectParameterModels()
{
    // Get all parameter models from the data manager
    auto allParameterModels = data_manager_->GetAllParameterModels();
    
    // Connect each model to the corresponding view
    for (const auto& model : allParameterModels) {
        if (model) {
            // Get parameter ID
            VitalSync::ParameterType type = static_cast<VitalSync::ParameterType>(model->GetParameterId());
            
            // Find the corresponding view
            auto viewIt = parameter_views_.find(type);
            if (viewIt != parameter_views_.end()) {
                // Connect model to view
                viewIt.value()->SetModel(model);
            }
        }
    }
}

/**
 * @brief Applies default settings to all view components
 * 
 * Configures all waveform and parameter views with default appearance settings
 * from the configuration manager, including colors, ranges, and sweep speeds.
 */
void MainWindow::ApplyDefaultSettings()
{
    // Get settings from ConfigManager
    ConfigManager& config = ConfigManager::GetInstance();
    
    // Apply settings to waveform views
    double sweepSpeed = config.GetDouble("ui/defaultSweepSpeed", 25.0);
    QColor gridColor = config.GetColor("ui/defaultGridColor", QColor(30, 30, 30));
    QColor backgroundColor = config.GetColor("ui/defaultBackgroundColor", Qt::black);
    
    // Apply settings to all waveform views
    for (auto& view : waveform_views_) {
        view->SetSweepSpeed(sweepSpeed);
        view->SetGridColor(gridColor);
        view->SetBackgroundColor(backgroundColor);
        view->SetGridVisible(true);
        view->SetTimeScaleVisible(true);
        view->SetAmplitudeScaleVisible(true);
    }
    
    // Apply settings to parameter views
    for (auto& view : parameter_views_) {
        view->SetBackgroundColor(backgroundColor);
        view->SetTextColor(Qt::white);
        view->SetLabelVisible(true);
        view->SetUnitVisible(true);
    }
}

/**
 * @brief Retrieves the waveform view for a specific type
 * 
 * Gets the waveform view component associated with the specified waveform type.
 * 
 * @param type The waveform type to find
 * @return A shared pointer to the waveform view or nullptr if not found
 */
std::shared_ptr<IWaveformView> MainWindow::GetWaveformView(VitalSync::WaveformType type) const
{
    auto it = waveform_views_.find(type);
    if (it != waveform_views_.end()) {
        return it.value();
    }
    return nullptr;
}

/**
 * @brief Retrieves the parameter view for a specific type
 * 
 * Gets the parameter view component associated with the specified parameter type.
 * 
 * @param type The parameter type to find
 * @return A shared pointer to the parameter view or nullptr if not found
 */
std::shared_ptr<IParameterView> MainWindow::GetParameterView(VitalSync::ParameterType type) const
{
    auto it = parameter_views_.find(type);
    if (it != parameter_views_.end()) {
        return it.value();
    }
    return nullptr;
} 
