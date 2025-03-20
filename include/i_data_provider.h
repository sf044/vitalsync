/**
 * @file i_data_provider.h
 * @brief Interface for all data providers
 * 
 * IDataProvider interface, serves as the contract
 * that all physiological data source providers must implement. Data providers
 * are responsible for acquiring data from various sources (demo generation,
 * network connections, files), processing it into a standardized format,
 * and emitting signals with waveform and parameter data for consumption by
 * the application's data models. The interface defines methods for starting
 * and stopping data acquisition, checking status, and configuring the provider.
 */

#ifndef I_DATA_PROVIDER_H
#define I_DATA_PROVIDER_H

#include <string>
#include <memory>
#include <QVector>
#include <QString>
#include <QVariantMap>
#include <QObject>
#include "vital_sync_types.h"

/**
 * @brief Interface for physiological data providers
 *
 * The IDataProvider interface defines the contract for classes that provide
 * physiological waveform and parameter data from various sources.
 * 
 * - Demo providers: Generate simulated physiological data for testing and demonstration
 * - Network providers: Connect to external devices or servers to receive real data
 * - File providers: Read data from recorded files for playback and analysis
 * 
 * All providers must implement methods for starting and stopping data acquisition,
 * checking status, and configuring provider-specific settings. Providers emit signals
 * when new data is available, when connection status changes, or when errors occur.
 */
class IDataProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject for memory management
     */
    explicit IDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    
    /**
     * @brief Virtual destructor for proper inheritance cleanup
     */
    virtual ~IDataProvider() {}
    
    /**
     * @brief Start data acquisition
     * @return True if acquisition was successfully started
     * 
     * Initiates the data acquisition process for this provider. The specific
     * behavior depends on the provider type:
     * - Demo provider: Starts generating simulated data
     * - Network provider: Establishes connection to the data source
     * - File provider: Opens the file and begins reading data
     * 
     * Returns false if acquisition cannot be started due to configuration
     * issues or connection problems.
     */
    virtual bool start() = 0;
    
    /**
     * @brief Stop data acquisition
     * 
     * Stops the data acquisition process for this provider. The specific
     * behavior depends on the provider type:
     * - Demo provider: Stops generating simulated data
     * - Network provider: Disconnects from the data source
     * - File provider: Stops reading from the file
     * 
     * This method is safe to call even if acquisition has not been started.
     */
    virtual void stop() = 0;
    
    /**
     * @brief Get the current connection status
     * @return Connection status enumeration value
     * 
     * Returns the current connection status of the provider, which can be:
     * - Disconnected: Not connected to any data source
     * - Connecting: Attempting to connect to a data source
     * - Connected: Successfully connected and receiving data
     * - Error: Connection failed or encountered an error
     */
    virtual VitalSync::ConnectionStatus GetConnectionStatus() const = 0;
    
    /**
     * @brief Get provider name
     * @return String identifier for this provider
     * 
     * Returns a unique string identifier for this provider type,
     * such as "Demo", "Network", or "File". This name is used to
     * identify and select providers in the user interface.
     */
    virtual std::string GetName() const = 0;
    
    /**
     * @brief Check if this provider is currently in use
     * @return True if this provider is active and acquiring data
     * 
     * Returns true if this provider is currently active and in the
     * process of acquiring data. Returns false if the provider has
     * been stopped or has not been started yet.
     */
    virtual bool isActive() const = 0;
    
    /**
     * @brief Configure the provider with connection parameters
     * @param params Provider-specific connection parameters
     * @return True if configuration was successful
     * 
     * Sets configuration parameters for this provider. The specific
     * parameters depend on the provider type:
     * - Demo provider: waveform type, update interval, etc.
     * - Network provider: host, port, protocol, etc.
     * - File provider: file path, playback speed, etc.
     * 
     * Returns false if the configuration is invalid or cannot be applied.
     */
    virtual bool configure(const QVariantMap& params) = 0;

signals:
    /**
     * @brief Signal emitted when connection status changes
     * @param status New connection status
     * 
     * This signal is emitted whenever the provider's connection status
     * changes, such as when connecting to a data source, disconnecting,
     * or encountering connection errors. UI components can connect to
     * this signal to display the current connection state.
     */
    void connectionStatusChanged(VitalSync::ConnectionStatus status);

    /**
     * @brief Signal emitted when new waveform data is available
     * @param waveformType Type of waveform (ECG, respiration, etc.)
     * @param timestamp Timestamp in milliseconds when the data was captured
     * @param data Vector of floating-point values representing the waveform samples
     * 
     * This signal is emitted when the provider has new waveform data available.
     * The data manager connects to this signal to route the data to the
     * appropriate waveform model for processing and display.
     */
    void waveformDataReceived(int waveformType, qint64 timestamp, const QVector<float>& data);

    /**
     * @brief Signal emitted when new parameter data is available
     * @param parameterType Type of parameter (heart rate, SpO2, etc.)
     * @param timestamp Timestamp in milliseconds when the data was captured
     * @param value The parameter value
     * 
     * This signal is emitted when the provider has a new parameter value available.
     * The data manager connects to this signal to route the data to the
     * appropriate parameter model for processing and display.
     */
    void parameterDataReceived(int parameterType, qint64 timestamp, float value);

    /**
     * @brief Signal emitted when provider encounters an error
     * @param errorCode Numeric identifier for the error type
     * @param errorMessage Human-readable description of the error
     * 
     * This signal is emitted when the provider encounters an error during
     * operation, such as connection failures, data format errors, or
     * internal processing issues. The data manager connects to this signal
     * to handle errors and display messages to the user.
     */
    void errorOccurred(int errorCode, const QString& errorMessage);
};

#endif // I_DATA_PROVIDER_H 
