/**
 * @file main.cpp
 * @brief Application entry point
 * 
 * This file contains the main entry point.
 * It initializes the Qt application, sets up the application metadata,
 * initializes the configuration manager, creates the main application window,
 * and starts the main event loop.
 * 
 * The application follows the standard Qt application lifecycle:
 * 1. Create a QApplication instance to manage application-wide resources
 * 2. Configure application metadata (name, organization, version)
 * 3. Initialize required subsystems (configuration manager)
 * 4. Create and display the main window
 * 5. Enter the event processing loop
 */

#include <QApplication>
#include <QSettings>

#include "../include/config_manager.h"
#include "ui/main_window.h"


int main(int argc, char *argv[])
{
    // Create the application
    QApplication app(argc, argv);
    
    // Set application information
    QApplication::setApplicationName("VitalSync");
    QApplication::setOrganizationName("VitalSyncTech");
    QApplication::setOrganizationDomain("vitalsynctech.com");
    QApplication::setApplicationVersion("1.0.0");
    
    // Initialize the configuration manager
    if (!ConfigManager::GetInstance().Initialize(QApplication::organizationName(), QApplication::applicationName())) {
        // Show error message?
        return 1;
    }
    
    // Create and show the main window
    MainWindow main_window;
    main_window.show();
    
    // Run the application
    return app.exec();
} 
