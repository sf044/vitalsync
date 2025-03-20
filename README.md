# VitalSync

This is a gradution project, a physiological monitoring application built with C++ and Qt. Purpose of this project was primarly using software engineering tools and secondly achieving working simple application in order to test showing vital data of patiens while working on a main product. This project may consist unused or overly developed programming tools for education purposes and lots of documentation/comments. This project and readme file are mostly notes to myself in order to look later and clearly remember/understand.

![VitalSync](screnshot.png)

## Overview

VitalSync is a physiological monitoring application developed to acquire, process, and display real-time patient data. The application supports various waveform visualizations (ECG, respiration, plethysmograph, etc.) and numerical parameters (heart rate, SpO2, blood pressure, etc.).

The application is developed for achieving a modular architecture that allows for multiple data sources, customizable displays, and extensible features. This document provides a technical overview of the project's architecture, components, and implementation details.

## Project Architecture

VitalSync follows model-view-controller architecture. Here are the core components:

```
┌───────────────────────────┐     ┌────────────────────────────┐     ┌────────────────────────────┐
│       Data Providers      │     │        Data Manager        │     │           Models           │
│                           │     │                            │     │                            │
│ ┌─────────────────────┐   │     │ ┌──────────────────────┐   │     │ ┌──────────────────────┐   │
│ │  IDataProvider      │   │     │ │  IDataManager        │   │     │ │  IWaveformModel      │   │
│ │  - start()          │   │     │ │  - initialize()      │   │     │ │  - addWaveformData() │   │
│ │  - stop()           ├───┼────►│ │  - startAcquisition()├───┼────►│ │  - GetData()         │   │
│ │  - GetStatus()      │   │     │ │  - stopAcquisition() │   │     │ └──────────────────────┘   │
│ └─────────────────────┘   │     │ └──────────────────────┘   │     │                            │
│          ▲                │     │                            │     │ ┌──────────────────────┐   │
│          │                │     │                            │     │ │  IParameterModel     │   │
│ ┌────────┴──────────┐     │     │                            │     │ │  - setValue()        │   │
│ │ DemoDataProvider  │     │     │                            │     │ │  - GetValue()        │   │
│ │ - generateData()  │     │     │                            │     │ └──────────────────────┘   │
│ └───────────────────┘     │     │                            │     │                            │
└───────────────────────────┘     └────────────────────────────┘     └─────────────┬──────────────┘
          │                                                                        │
          │               ┌────────────────────────────────────────────────────────┘
          │               │
          │               ▼
          │      ┌────────────────────────────┐                 ┌────────────────────────────┐
          │      │           Views            │                 │     Configuration          │
          │      │                            │                 │                            │
          │      │ ┌──────────────────────┐   │                 │ ┌──────────────────────┐   │
          │      │ │  IWaveformView       │   │                 │ │  ConfigManager       │   │
          │      │ │  - SetModel()        │   │◄────Configure───│ │  - Initialize()      │   │
          └─────►│ └──────────────────────┘   │    Settings     │ │  - save()            │   │
                 │                            │                 │ └──────────────────────┘   │
                 │ ┌──────────────────────┐   │                 │                            │
                 │ │  IParameterView      │   │                 │                            │
                 │ │  - SetModel()        │   │                 │                            │
                 │ │  - updateValue()     │   │                 │                            │
                 │ └──────────────────────┘   │                 │                            │
                 └────────────────────────────┘                 └────────────────────────────┘
```

### Component Responsibilities

1. **Data Providers**: Responsible for acquiring physiological data from various sources
   - Currently implemented: `DemoDataProvider` for simulated data
   - Interface design supports future implementations for network and file-based data sources

2. **Data Manager**: Central coordinator that routes data between providers and models
   - Manages provider selection and configuration
   - Routes waveform and parameter data to appropriate models

3. **Models**: Store and process physiological data
   - Waveform models: Store continuous time-series data (ECG, respiration, etc.)
   - Parameter models: Store numerical vital values (heart rate, SpO2, etc.)

4. **Views**: Display data and handle user interactions
   - Waveform views: Display scrolling waveforms with customizable appearance
   - Parameter views: Display numerical values with alarm indications

5. **Configuration Manager**: Manages application-wide settings and user preferences
   - Persistent storage using Qt's QSettings
   - Typed accessors for various configuration options

## Project Structure

The project is organized into the following directory structure:

```
VitalSyncPro/
├── include/                # Public interface headers
│   ├── i_data_provider.h               # Data provider interface
│   ├── i_data_manager.h                # Data manager interface
│   ├── i_waveform_model.h              # Waveform model interface
│   ├── i_parameter_model.h             # Parameter model interface
│   ├── i_waveform_view.h               # Waveform view interface
│   ├── i_parameter_view.h              # Parameter view interface
│   ├── config_manager.h                # Configuration manager
│   └── vital_sync_types.h              # Common types and enumerations
├── src/                    # Implementation files
│   ├── core/               # Core implementation components
│   │   ├── data_manager.h/cpp          # Data manager implementation
│   │   ├── waveform_model.h/cpp        # Waveform model implementation
│   │   └── parameter_model.h/cpp       # Parameter model implementation
│   ├── providers/  # Data provider implementations
│   │   └── demo_data_provider.h/cpp    # Demo data provider implementation
│   ├── ui/                 # User interface components
│   │   ├── main_window.h/cpp           # Main application window
│   │   ├── parameters/                 # Parameter display components
│   │   │   └── parameter_view.h/cpp    # Parameter view implementation
│   │   └── waveforms/                  # Waveform display components
│   │       └── waveform_view.h/cpp     # Waveform view implementation
│   └── main.cpp
└── CMakeLists.txt
```

## Application Lifecycle

The VitalSync application follows this lifecycle from startup to shutdown:

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application Startup                         │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│ 1. Initialize Qt Application                                    │
│    - Parse command line arguments                               │
│    - Set application metadata (name, organization, version)     │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. Initialize ConfigManager                                     │
│    - Load user settings from disk                               │
│    - Apply default settings if no previous configuration exists │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. Create Main Window                                           │
│    - Initialize UI components                                   │
│    - Create and configure DataManager                           │
│    - Register data providers                                    │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│ 4. Initialize DataManager                                       │
│    - Create data models (waveforms and parameters)              │
│    - Restore last active provider from configuration            │
│    - Connect signals and slots for data routing                 │
└───────────────────────────────┬─────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│ 5. Enter Qt Event Loop                                          │
│    ┌───────────────────────────────────────────────────────────┐│
│    │                 Process Events & Signals                  ││
│    │  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐  ││
│    │  │ User Input  │────►│ Data Updates│────►│ UI Updates  │  ││
│    │  └─────────────┘     └─────────────┘     └─────────────┘  ││
│    │                           ▲ ▲                             ││
│    │  ┌─────────────┐          │ │          ┌─────────────┐    ││
│    │  │Configuration│◄─────────┘ └─────────►│ Data Source │    ││
│    │  │   Updates   │                       │   Events    │    ││
│    │  └─────────────┘                       └─────────────┘    ││
│    └───────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
                                │
                                │ (Application Exit)
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│ 6. Cleanup and Shutdown                                         │
│    - Stop active data acquisition                               │
│    - Save current settings to disk                              │
│    - Destroy data models and providers                          │
│    - Clean up Qt resources                                      │
└─────────────────────────────────────────────────────────────────┘
```

### Key Lifecycle Events

1. **Application Initialization**: In `main.cpp`, the application initializes the Qt framework, sets application metadata, and creates core components.

2. **Configuration Loading**: The `ConfigManager` loads saved settings or applies defaults.

3. **Component Creation**: The `MainWindow` creates and configures the UI, the `DataManager`, and registers available data providers.

4. **Data Acquisition**: When started, the selected `IDataProvider` begins generating or acquiring data which flows through the system.

5. **Event Loop**: The Qt event loop handles user input, timer events, and signal/slot communications.

6. **Shutdown**: The application saves settings, stops data acquisition, and releases resources.

## Class Diagram

The following simplified class diagram shows the key components and their relationships:

```
┌───────────────────────┐
│       QObject         │
│ + connect()           │
│ + disconnect()        │
└─────────┬─────────────┘
          │
          ├───────────────────────┬───────────────────────┬───────────────────────┐
          │                       │                       │                       │
┌─────────▼─────────────┐ ┌───────▼───────────┐  ┌───────▼───────────┐  ┌────────▼──────────┐
│    IDataProvider      │ │   IDataManager    │  │   IWaveformModel  │  │  IParameterModel  │
│ + start(): bool       │ │ + initialize()    │  │ + addWaveformData │  │ + setValue()      │
│ + stop(): void        │ │ + startAcquisition│  │ + GetData()       │  │ + GetValue()      │
│ + GetStatus(): enum   │ │ + stopAcquisition │  │ + isActive()      │  │ + GetLimits()     │
└─────────┬─────────────┘ └───────┬───────────┘  └───────┬───────────┘  └────────┬──────────┘
          │                       │                      │                       │
          │                       │                      │                       │
┌─────────▼─────────────┐ ┌───────▼───────────┐  ┌───────▼───────────┐  ┌────────▼──────────┐
│   DemoDataProvider    │ │    DataManager    │  │   WaveformModel   │  │   ParameterModel  │
│ - waveform_timer_     │ │ - providers_      │  │ - data_           │  │ - value_          │
│ - parameter_timer_    │ │ - waveform_models_│  │ - mutex_          │  │ - timestamp_      │
│ + generateWaveformData│ │ - parameter_models│  │ - max_buffer_size_│  │ - alarm_limits_   │
│ + generateParameterDat│ │ + HandleWaveformDa│  │ + addWaveformData │  │ + setValue()      │
└───────────────────────┘ └───────────────────┘  └───────────────────┘  └───────────────────┘

┌───────────────────────┐  ┌───────────────────────┐
│       QWidget         │  │       QObject         │
│ + paintEvent()        │  │ + connect()           │
│ + resizeEvent()       │  │ + disconnect()        │
└─────────┬─────────────┘  └─────────┬─────────────┘
          │                          │
          │                          │
┌─────────▼─────────────┐  ┌─────────▼─────────────┐
│     IWaveformView     │  │    IParameterView     │
│ + SetModel()          │  │ + SetModel()          │
│ + GetSweepSpeed()     │  │ + GetModel()          │
│ + SetGridVisible()    │  │ + SetAlarmLimits()    │
└─────────┬─────────────┘  └─────────┬─────────────┘
          │                          │
          │                          │
┌─────────▼─────────────┐  ┌─────────▼─────────────┐
│     WaveformView      │  │     ParameterView     │
│ - m_model             │  │ - m_model             │
│ - m_gridVisible       │  │ - m_value_label       │
│ - m_backgroundColor   │  │ - m_units_label       │
│ + paintEvent()        │  │ + updateValue()       │
│ + drawWaveform()      │  │ + updateAlarmState()  │
└─────────┬─────────────┘  └─────────┬─────────────┘
          │                          │
          └──────────────┬───────────┘
                         │
                ┌────────▼───────────┐
                │     MainWindow     │        Owns/Creates
                │ - waveform_views_  │◄─ - - - - - - - - - - - - - - ┐
                │ - parameter_views_ │                               │
                │ - data_manager_    │─ - - - - - - - - - - - - ┐    │
                │ + SetupUi()        │                          │    │
                │ + connectSignals() │                          │    │
                └────────────────────┘                          │    │
                           │                                    │    │
                  Uses     │                                    │    │
                           ▼                                    │    │
                ┌────────────────────┐           Manages        │    │
                │   ConfigManager    │◄ - - - - - - - - - - - - ┘    │
                │   (Singleton)      │                               │
                │ + GetInstance()    │           Configures          │
                │ + Initialize()     │─ - - - - - - - - - - - - - - >│
                │ + GetString()      │
                │ + SetInt()         │
                └────────────────────┘
```

## C++ Features Used

### 1. Interface-Based Design

Abstract base classes with virtual methods to define interfaces:

```cpp
class IDataProvider : public QObject
{
    Q_OBJECT
public:
    explicit IDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDataProvider() {}
    
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual VitalSync::ConnectionStatus GetConnectionStatus() const = 0;
    virtual std::string GetName() const = 0;
    virtual bool isActive() const = 0;
    virtual bool configure(const QVariantMap& params) = 0;

signals:
    void connectionStatusChanged(VitalSync::ConnectionStatus status);
    void waveformDataReceived(int waveformType, qint64 timestamp, const QVector<float>& data);
    void parameterDataReceived(int parameterType, qint64 timestamp, float value);
    void errorOccurred(int errorCode, const QString& errorMessage);
};
```
This enables polymorphism and loose coupling between components:


```cpp
// Concrete implementation of IDataProvider
class DemoDataProvider : public IDataProvider {
public:
    // Implementation of interface methods
    bool start() override {
        // Start generating data...
        return true;
    }
    
    void stop() override {
        // Stop generating data...
    }
    
    // Additional implementations...
};

// Code that depends only on the interface
bool DataManager::startAcquisition() {
    if (current_provider_) {
        return current_provider_->start();  // Works with any IDataProvider
    }
    return false;
}
```
This pattern is used throughout the codebase, with interfaces for data management, waveform models, parameter models, and views.


### 2. Smart Pointers

`std::shared_ptr` for automatic memory management(which is raw pointer with hiding extra steps of deallocate):
```cpp
// In DataManager.h - Member variables using shared_ptr
private:
    std::shared_ptr<IDataProvider> current_provider_;
    QMap<int, std::shared_ptr<IWaveformModel>> waveform_models_;
    QMap<int, std::shared_ptr<IParameterModel>> parameter_models_;

// Method return types
std::shared_ptr<IWaveformModel> GetWaveformModel(int waveformId) const override {
    QMutexLocker locker(&mutex_);
    return waveform_models_.value(waveformId, nullptr);
}

// Method parameters
void setWaveformModel(std::shared_ptr<IWaveformModel> model);

// In collections
std::vector<std::shared_ptr<IWaveformModel>> GetAllWaveformModels() const override {
    QMutexLocker locker(&mutex_);
    std::vector<std::shared_ptr<IWaveformModel>> models;
    for (auto it = waveform_models_.begin(); it != waveform_models_.end(); ++it) {
        models.push_back(it.value());
    }
    return models;
}
```
Smart pointers eliminate manual memory management so helps prevent memory leaks, reduces human errors actually.

### 3. Namespaces

Enumerations and utility functions are encapsulated with `VitalSync` namespace:

```cpp
namespace VitalSync {
    // Waveform type definitions
    enum class WaveformType {
        ECG_I = 0,
        ECG_II = 1,
        ECG_III = 2,
        RESP = 3,
        PLETH = 4,
        ABP = 5,
        CVP = 6,
        CAPNO = 7,
        EEG = 8
    };
    
    // Parameter type definitions
    enum class ParameterType {
        HR = 0,
        RR = 1,
        SPO2 = 2,
        NIBP_SYS = 3,
        NIBP_DIA = 4,
        NIBP_MAP = 5,
        TEMP1 = 6,
        TEMP2 = 7,
        ETCO2 = 8,
        IBP1_SYS = 9,
        IBP1_DIA = 10,
        IBP1_MAP = 11,
        IBP2_SYS = 12,
        IBP2_DIA = 13,
        IBP2_MAP = 14
    };
    
    // Connection status enum
    enum class ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    
    // Utility functions
    inline QString GetWaveformDisplayName(WaveformType type) {
        switch (type) {
            case WaveformType::ECG_I:
                return "ECG I";
            // Other cases...
            default:
                return "Unknown";
        }
    }
    
    inline QString GetParameterUnit(ParameterType type) {
        switch (type) {
            case ParameterType::HR:
                return "bpm";
            // Other cases...
            default:
                return "";
        }
    }
}
```

Using a namespace:
- Groups related items together logically
- Prevents naming conflicts with other libraries
- Creates a clearer API by providing context for types and functions
- Supports modular code organization


### 4. Type-Safe Enumerations

`enum class` for type safety:
```cpp
enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class WaveformType {
    ECG_I = 0,
    ECG_II = 1,
    // Additional types...
};

enum class ErrorCode {
    NoError = 0,
    ConnectionError = 100,
    ConfigurationError = 200,
    DataError = 300,
    HardwareError = 400,
    CriticalError = 500,
    UnknownError = 999
};
```

Example usage in the codebase:

```cpp
void UpdateConnectionStatus(VitalSync::ConnectionStatus status) {
    switch (status) {
        case VitalSync::ConnectionStatus::Connected:
            status_label_->setStyleSheet("color: green;");
            status_label_->setText("Connected");
            break;
        case VitalSync::ConnectionStatus::Disconnected:
            status_label_->setStyleSheet("color: red;");
            status_label_->setText("Disconnected");
            break;
        // Other cases...
    }
}
```


### 5. Containers and Algorithms

STL and Qt containers:

```cpp
// QVector for waveform data storage (contiguous memory, optimized for numerical data)
QVector<float> m_data_;  // In WaveformModel

// QMap for associative data (key-based lookup)
QMap<int, std::shared_ptr<IWaveformModel>> waveform_models_;  // In DataManager
QMap<VitalSync::ParameterType, std::shared_ptr<IParameterView>> parameter_views_;  // In MainWindow

// std::vector for collections
std::vector<std::shared_ptr<IWaveformModel>> GetAllWaveformModels() const;

// std::unordered_map for function mapping
std::unordered_map<int, std::function<QVector<float>(double, int)>> waveform_generators_;  // In DemoDataProvider

// Using container methods
void addWaveformData(qint64 timestamp, const QVector<float>& data) override {
    QMutexLocker locker(&mutex_);
    
    // Vector operations
    for (float value : data) {
        m_data_.append(value);
    }
    
    // Maintain maximum buffer size
    while (m_data_.size() > max_buffer_size_) {
        m_data_.removeFirst();
    }
    
    last_update_timestamp_ = timestamp;
    emit dataUpdated();
}
```
Benefits of using these containers:
- Automatic memory management
- Interfaces for common operations
- Type safety with templates


### 6. Function Objects and Lambdas

The codebase uses function objects and lambda expressions for callbacks and mapping operations:

```cpp
// In DemoDataProvider, using function objects to map waveform types to generator functions for callback-based behavior
std::unordered_map<int, std::function<QVector<float>(double, int)>> waveform_generators_;

// Initializing with function mappings
void DemoDataProvider::initialize() {
    // Map waveform types to their generator functions
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::ECG_I)] = 
        [this](double time, int points) { return generateECG(time, points); };
    
    waveform_generators_[static_cast<int>(VitalSync::WaveformType::RESP)] = 
        [this](double time, int points) { return generateRespiration(time, points); };
    
    // More mappings...
}

// Using function objects for modular behavior
QVector<float> DemoDataProvider::generateWaveformData(int waveformType) {
    auto it = waveform_generators_.find(waveformType);
    if (it != waveform_generators_.end()) {
        double currentTime = elapsed_timer_.elapsed() / 1000.0;
        return it->second(currentTime, samples_per_update_);
    }
    return QVector<float>();
}

// Using lambdas for signal-slot connections
connect(m_updateTimer, &QTimer::timeout, 
        this, &DemoDataProvider::generateWaveformData);

// Using lambdas for simple callbacks
QTimer::singleShot(100, this, [this]() {
    // Delayed initialization code
    initializeModels();
});
```
Benefits of using function objects and lambdas:
- Simplifies event handling and signal connections
- Organizes code by keeping related functionality together


### 7. Mutex-Based Synchronization

Mutexes to provides thread safety when accessing shared data:

```cpp
// Mutex declaration in class definition
private:
    mutable QMutex mutex_;  // mutable allows locking in const methods

// Thread-safe access pattern using lock guards
QVector<float> WaveformModel::getData() const {
    QMutexLocker locker(&mutex_);  // RAII-based locking
    return m_data_;
    // Mutex automatically unlocked when locker goes out of scope
}

void WaveformModel::addWaveformData(qint64 timestamp, const QVector<float>& data) {
    QMutexLocker locker(&mutex_);
    
    // Protected critical section - safely modify shared data
    for (float sample : data) {
        m_data_.append(sample);
    }
    
    // Keep buffer at maximum size
    while (m_data_.size() > max_buffer_size_) {
        m_data_.removeFirst();
    }
    
    last_update_timestamp_ = timestamp;
    
    // Signal can be safely emitted after data is updated
    emit dataUpdated();
}
```
Thread synchronization:
- Mutex protection for shared data access
- RAII-style locking with QMutexLocker
- Locking for all read/write operations

This approach provides:
- Data integrity is maintained in multi-threaded scenarios
- Race conditions are prevented when accessing shared state
- The UI remains responsive while processing data


## Qt Features Used

### 1. Signal-Slot Mechanism

Qt's signal-slot mechanism enables loose coupling between components while keeping type safety:

```cpp
// Signal declarations in IDataProvider interface
signals:
    void connectionStatusChanged(VitalSync::ConnectionStatus status);
    void waveformDataReceived(int waveformType, qint64 timestamp, const QVector<float>& data);
    void parameterDataReceived(int parameterType, qint64 timestamp, float value);
    void errorOccurred(int errorCode, const QString& errorMessage);

// Signal emissions in DemoDataProvider implementation
void DemoDataProvider::generateWaveformData() {
    // Generate waveform data for each active type
    for (int waveformType : activeWaveforms) {
        qint timestamp = QDateTime::currentMSecsSinceEpoch();
        QVector<float> data = generateWaveformSamples(waveformType);
        
        // Emit signal with data
        emit waveformDataReceived(waveformType, timestamp, data);
    }
}

// Connecting signals to slots in DataManager
void DataManager::connectProviderSignals() {
    if (!current_provider_) return;
    
    // Connect provider data signals to manager handlers
    connect(current_provider_.get(), &IDataProvider::waveformDataReceived,
            this, &DataManager::HandleWaveformData, Qt::QueuedConnection);
    
    connect(current_provider_.get(), &IDataProvider::parameterDataReceived,
            this, &DataManager::HandleParameterData, Qt::QueuedConnection);
    
    connect(current_provider_.get(), &IDataProvider::connectionStatusChanged,
            this, &DataManager::HandleConnectionStatusChanged);
    
    connect(current_provider_.get(), &IDataProvider::errorOccurred,
            this, &DataManager::HandleProviderError);
}

// Signal propagation through the system
void DataManager::HandleWaveformData(int waveformType, qint64 timestamp, const QVector<float>& data) {
    // Find the appropriate model for this waveform type
    auto model = waveform_models_.value(waveformType);
    if (model) {
        // Forward the data to the model
        model->addWaveformData(timestamp, data);
    }
}
```

```cpp
// Queued connection for cross-thread communication
connect(networkProvider, &NetworkProvider::waveformDataReceived,
        this, &DataManager::HandleWaveformData, Qt::QueuedConnection);
```
Benefits of the signal-slot system:
- Components remain decoupled - senders don't need to know about receivers
- Type-safe connections that are checked at compile-time
- Supports one-to-many communication patterns(observer)
- Self-documenting code that clearly shows component interactions


### 2. QObject Hierarchy

Most classes inherit from QObject for enabling these features:
- Signal-slot support
- Memory management through parent-child relationships
- QObject capabilities

```cpp
// Base class inheritance
class IDataProvider : public QObject {
    Q_OBJECT  // Required macro for QObject features
    // ...
};

// Multiple inheritance with QObject
class WaveformModel : public QObject, public IWaveformModel {
    Q_OBJECT
    // ...
};

// QObject inheritance hierarchies
class MainWindow : public QMainWindow {  // QMainWindow inherits QWidget, which inherits QObject
    // ...
};
```
Parent-child relationships are established for automatic memory management:

```cpp
// In MainWindow constructor
provider_selector_ = new QComboBox(this);  // MainWindow takes ownership
status_label_ = new QLabel(this);          // MainWindow takes ownership

// In DataManager
void DataManager::initializeProviders() {
    // DataManager becomes the parent/owner of the providers
    registerProvider(std::make_shared<DemoDataProvider>(this));
    // Other providers...
}
```
Benefits of QObject hierarchy:
- Signal-slot communication
- Parent-child memory management
- Property system

### 3. Custom Painting

Custom painting is used to render waveforms:

```cpp
// WaveformView implements custom painting
void WaveformView::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    // Draw background
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Fill background
    painter.fillRect(rect(), m_backgroundColor);
    
    // Draw grid if visible
    if (m_gridVisible) {
        drawGrid(painter);
    }
    
    // Draw waveform
    drawWaveform(painter);
    
    // Draw labels and scales if enabled
    if (m_timeScaleVisible) {
        drawTimeScale(painter);
    }
    
    if (m_amplitudeScaleVisible) {
        drawAmplitudeScale(painter);
    }
}

// Specialized waveform drawing
void WaveformView::drawWaveform(QPainter& painter) {
    if (!m_model || m_model->GetData().isEmpty()) {
        return;
    }
    
    // Setup pen for drawing
    QPen waveformPen(m_model->GetColor(), 2);
    painter.setPen(waveformPen);
    
    // Get data from model
    const QVector<float>& data = m_model->GetData();
    
    // Create a path for the waveform
    QPainterPath path;
    
    // Calculate scaling factors
    float xScale = static_cast<float>(width()) / (data.size() > 0 ? data.size() - 1 : 1);
    float yRange = m_model->GetMaxValue() - m_model->GetMinValue();
    float yScale = yRange > 0 ? height() / yRange : 1.0f;
    
    // Start the path
    if (!data.isEmpty()) {
        float x = 0;
        float y = height() - (data[0] - m_model->GetMinValue()) * yScale;
        path.moveTo(x, y);
        
        // Add points to the path
        for (int i = 1; i < data.size(); ++i) {
            x = i * xScale;
            y = height() - (data[i] - m_model->GetMinValue()) * yScale;
            path.lineTo(x, y);
        }
    }
    
    // Draw the path
    painter.drawPath(path);
}
```


### 4. Timer System

QTimer is used for regular updates and data generation:

```cpp
// In DemoDataProvider constructor
DemoDataProvider::DemoDataProvider(QObject* parent) : IDataProvider(parent) {
    // Create timer for waveform updates
    waveform_timer_ = new QTimer(this);
    waveform_timer_->setInterval(waveform_update_interval_ms_);
    connect(waveform_timer_, &QTimer::timeout, 
            this, &DemoDataProvider::generateWaveformData);
    
    // Create timer for parameter updates
    parameter_timer_ = new QTimer(this);
    parameter_timer_->setInterval(parameter_update_interval_ms_);
    connect(parameter_timer_, &QTimer::timeout,
            this, &DemoDataProvider::generateParameterData);
    
    // More initialization...
}

// Starting/stopping timers with data acquisition
bool DemoDataProvider::start() {
    if (active_) return true;
    
    // Reset and start elapsed timer
    elapsed_timer_.restart();
    
    // Start the update timers
    waveform_timer_->start();
    parameter_timer_->start();
    
    active_ = true;
    emit connectionStatusChanged(VitalSync::ConnectionStatus::Connected);
    return true;
}

void DemoDataProvider::stop() {
    if (!active_) return;
    
    // Stop the update timers
    waveform_timer_->stop();
    parameter_timer_->stop();
    
    active_ = false;
    emit connectionStatusChanged(VitalSync::ConnectionStatus::Disconnected);
}

// One-shot timer for deferred operations
void MainWindow::initializeComponents() {
    // Setup initial components...
    
    // Deferred loading of large datasets
    QTimer::singleShot(100, this, [this]() {
        loadDemoData();
    });
}
```
Benefits of Qt's timer system:
- Regular updates for real-time data display
- Configurable intervals for different update frequencies
- Automatic integration with the Qt event loop
- Easy start/stop control for data generation
- Single-shot timers for non-continuous operations


### 5. Thread Management

Qt's threading capabilities:

```cpp
// Worker thread setup example (from NetworkProvider implementation)
void NetworkProvider::start() {
    if (m_isRunning) return;
    
    // Create a new thread for the network operations
    m_networkThread = new QThread(this);
    
    // Create the worker that will be moved to the thread
    m_networkWorker = new NetworkWorker(m_hostAddress, m_port);
    m_networkWorker->moveToThread(m_networkThread);
    
    // Connect thread signals
    connect(m_networkThread, &QThread::started,
            m_networkWorker, &NetworkWorker::startAcquisition);
    
    connect(m_networkWorker, &NetworkWorker::finished,
            m_networkThread, &QThread::quit);
    
    connect(m_networkThread, &QThread::finished,
            m_networkWorker, &NetworkWorker::deleteLater);
    
    // Connect worker signals to our forwarding slots
    // Use queued connections for cross-thread safety
    connect(m_networkWorker, &NetworkWorker::dataReceived,
            this, &NetworkProvider::onDataReceived,
            Qt::QueuedConnection);
    
    // Start the thread
    m_networkThread->start();
    m_isRunning = true;
    
    emit connectionStatusChanged(VitalSync::ConnectionStatus::Connected);
}

void NetworkProvider::stop() {
    if (!m_isRunning) return;
    
    // Signal the worker to stop
    if (m_networkWorker) {
        m_networkWorker->stopAcquisition();
    }
    
    // Stop and wait for the thread
    if (m_networkThread) {
        m_networkThread->quit();
        m_networkThread->wait(1000); // Wait up to 1 second for clean shutdown
    }
    
    m_isRunning = false;
    emit connectionStatusChanged(VitalSync::ConnectionStatus::Disconnected);
}
```

Thread safety is provieded by using mutexes and queued connections:

```cpp
// Thread-safe model access
void WaveformModel::addWaveformData(qint64 timestamp, const QVector<float>& data) {
    QMutexLocker locker(&mutex_); // Thread-safe lock
    
    // Update data safely
    // ...
    
    // Emit signal after data is updated
    emit dataUpdated();
}

// Cross-thread signal connections in DataManager
connect(dataProvider, &IDataProvider::waveformDataReceived,
        this, &DataManager::HandleWaveformData, Qt::QueuedConnection);
```
Benefits of Qt's threading system:
- Keeps UI responsive during intensive work
- Simplified thread lifecycle management(techical depth is less)
- Safe communication ensured by Qt between threads using signals/slots
- Thread-safe synchronization primitives (QMutex, QReadWriteLock)
- Event-driven architecture integrates well with background processing
- Qt's quarantees

### 6. Settings Management

QSettings for configuration storage:

```cpp
// In ConfigManager
bool ConfigManager::Initialize(const QString& organization, const QString& application) {
    // Create settings with the specified organization and application names
    m_settings = new QSettings(organization, application, this);
    
    // Load settings or use defaults
    return loadSettings();
}

bool ConfigManager::loadSettings() {
    // Example of reading various types of settings with defaults
    m_activeProviderName = m_settings->value("activeProvider", "Demo").toString();
    
    // Load provider-specific settings from groups
    m_settings->beginGroup("NetworkProvider");
    m_networkAddress = m_settings->value("address", "localhost").toString();
    m_networkPort = m_settings->value("port", 8080).toInt();
    m_settings->endGroup();
    
    // Load waveform display settings
    m_settings->beginGroup("WaveformDisplay");
    m_gridVisible = m_settings->value("gridVisible", true).toBool();
    m_gridColor = m_settings->value("gridColor", QColor(Qt::darkGray)).value<QColor>();
    m_timeScaleVisible = m_settings->value("timeScaleVisible", true).toBool();
    m_settings->endGroup();
    
    return true;
}

bool ConfigManager::save() {
    // Save current settings
    m_settings->setValue("activeProvider", m_activeProviderName);
    
    // Save provider-specific settings in groups
    m_settings->beginGroup("NetworkProvider");
    m_settings->setValue("address", m_networkAddress);
    m_settings->setValue("port", m_networkPort);
    m_settings->endGroup();
    
    // Save waveform display settings
    m_settings->beginGroup("WaveformDisplay");
    m_settings->setValue("gridVisible", m_gridVisible);
    m_settings->setValue("gridColor", m_gridColor);
    m_settings->setValue("timeScaleVisible", m_timeScaleVisible);
    m_settings->endGroup();
    
    // Ensure settings are written to disk
    m_settings->sync();
    
    return true;
}
```

Integration with the application lifecycle, main.cpp:

```cpp
// In main.cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application information for QSettings
    QApplication::setApplicationName("VitalSync");
    QApplication::setOrganizationName("VitalSyncTech");
    QApplication::setOrganizationDomain("vitalsynctech.com");
    QApplication::setApplicationVersion("1.0.0");
    
    // Initialize config manager with application info
    ConfigManager::GetInstance().Initialize(
        QApplication::organizationName(),
        QApplication::applicationName()
    );
    
    // Rest of the application setup...
    
    return app.exec();
}

// In MainWindow's closeEvent
void MainWindow::closeEvent(QCloseEvent* event) {
    // Save settings before closing
    ConfigManager::GetInstance().save();
    
    // Stop active data acquisition
    if (data_manager_) {
        data_manager_->stopAcquisition();
    }
    
    // Accept the close event
    event->accept();
}
```
Benefits of QSettings:
- Simple key-value API for different data types
- Hierarchical organization using groups
- Automatic conversion for Qt types like QColor


### 7. Property System

Qt's property system is used for component configuration:

```cpp
// Property declarations in class definition
class NetworkProvider : public QObject, public IDataProvider {
    Q_OBJECT
    
    // Property declarations with READ and WRITE methods
    Q_PROPERTY(QString hostAddress READ hostAddress WRITE setHostAddress)
    Q_PROPERTY(int port READ port WRITE setPort)
    
public:
    // Property accessor methods
    QString hostAddress() const { return m_hostAddress; }
    void setHostAddress(const QString& address) { m_hostAddress = address; }
    
    int port() const { return m_port; }
    void setPort(int port) { m_port = port; }
    
private:
    QString m_hostAddress = "localhost";
    int m_port = 8080;
};
```

Using properties for flexible configuration:

```cpp
// Using properties for generic configuration
bool configureProvider(QObject* provider, const QVariantMap& config) {
    // Set each property from the configuration map
    for (auto it = config.begin(); it != config.end(); ++it) {
        provider->setProperty(it.key().toLatin1(), it.value());
    }
    
    return true;
}

// Application example
QVariantMap config;
config["hostAddress"] = "192.168.1.100";
config["port"] = 9000;

// Configure provider using properties
configureProvider(networkProvider, config);
```
Benefits of the property system:
- Dynamic configuration at runtime
- Type conversion handled automatically
- Integration with Qt's meta-object
- Ready generic configuration tools


## Building and Running

- Qt 5.15 or newer
- C++17 compatible compiler
- CMake 3.16 or newer

### Build

```bash
# Clone the repository
git clone https://github.com/sf044/vitalsync.git
cd vitalsync

# Create build directory
mkdir build && cd build

# Run CMake and build
cmake ..
make -j$(nproc)

# Run the application
./bin/VitalSync
```
