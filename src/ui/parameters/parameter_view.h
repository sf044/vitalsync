/**
 * @file parameter_view.h
 * @brief Defines the ParameterView class for displaying physiological parameters
 *
 * This file contains the declaration of the ParameterView class which provides a
 * UI component for displaying vital sign parameters such as heart rate, blood pressure,
 * SpO2, etc. The class implements the IParameterView interface and supports customizable
 * visual appearance with alarm state visualization.
 */
#ifndef PARAMETER_VIEW_H
#define PARAMETER_VIEW_H

#include "../../../include/i_parameter_view.h"
#include "../../../include/i_parameter_model.h"
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <QMap>

/**
 * @brief Basic implementation of the IParameterView interface
 * 
 * This class provides a standard implementation of the IParameterView interface
 * that displays a vital sign parameter with customizable appearance and alarm
 * state visualization.
 */
class ParameterView : public QWidget, public IParameterView {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit ParameterView(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ParameterView() override;

    /**
     * @brief Set the parameter model to display
     * @param model Shared pointer to the parameter model
     */
    void SetModel(std::shared_ptr<IParameterModel> model) override;

    /**
     * @brief Get the associated parameter model
     * @return Shared pointer to the parameter model or nullptr if none
     */
    std::shared_ptr<IParameterModel> GetModel() const override;

    /**
     * @brief Get the widget associated with this view
     * @return Pointer to the widget
     */
    QWidget* GetWidget() override;

    /**
     * @brief Set whether to show the parameter label
     * @param visible True to show the label, false to hide it
     */
    void SetLabelVisible(bool visible) override;

    /**
     * @brief Set whether to show the unit label
     * @param visible True to show the unit, false to hide it
     */
    void SetUnitVisible(bool visible) override;

    /**
     * @brief Set the font size for the value display
     * @param size Font size in points
     */
    void SetValueFontSize(int size) override;

    /**
     * @brief Set the font size for the label display
     * @param size Font size in points
     */
    void SetLabelFontSize(int size) override;

    /**
     * @brief Set the background color
     * @param color Background color
     */
    void SetBackgroundColor(const QColor& color) override;

    /**
     * @brief Set the text color for normal state
     * @param color Text color
     */
    void SetTextColor(const QColor& color) override;

    /**
     * @brief Set alarm background color for a specific state
     * @param state Alarm state to set color for
     * @param color Color to use for the alarm state
     */
    void SetAlarmBackgroundColor(IParameterModel::AlarmState state, const QColor& color) override;

    /**
     * @brief Set alarm text color for a specific state
     * @param state Alarm state to set color for
     * @param color Color to use for the alarm state
     */
    void SetAlarmTextColor(IParameterModel::AlarmState state, const QColor& color) override;

    /**
     * @brief Update the view to reflect the current model state
     */
    void update() override;

protected:
    /**
     * @brief Paint event handler for rendering the parameter
     * @param event Paint event
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent* event) override;

private slots:
    /**
     * @brief Handle value changes from the model
     * @param value New value
     */
    void HandleValueChanged(float value);

    /**
     * @brief Handle alarm state changes from the model
     * @param state New alarm state
     */
    void HandleAlarmStateChanged(IParameterModel::AlarmState state);

    /**
     * @brief Handle property changes from the model
     */
    void HandlePropertiesChanged();

    /**
     * @brief Update the display timer - used for blinking in alarm states
     */
    void UpdateDisplay();

private:
    /**
     * @brief Set up the user interface
     */
    void SetupUI();

    /**
     * @brief Update the visual appearance based on alarm state
     */
    void UpdateAlarmAppearance();

    /**
     * @brief Connect signals from the model
     */
    void connectModelSignals();

    /**
     * @brief Disconnect signals from the model
     */
    void disconnectModelSignals();

private:
    std::shared_ptr<IParameterModel> model_;  /**< Parameter model */
    QLabel* label_widget_;                    /**< Label showing parameter name */
    QLabel* value_widget_;                    /**< Label showing parameter value */
    QLabel* unit_widget_;                     /**< Label showing parameter unit */
    QWidget* main_widget_;                    /**< Main widget container */
    
    bool label_visible_;                      /**< Whether label is visible */
    bool unit_visible_;                       /**< Whether unit is visible */
    int value_font_size_;                     /**< Font size for value display */
    int label_font_size_;                     /**< Font size for label display */
    QColor background_color_;                 /**< Background color */
    QColor text_color_;                       /**< Text color */
    
    IParameterModel::AlarmState current_alarm_state_; /**< Current alarm state */
    bool alarm_blink_state_;                  /**< Current blink state for alarms */
    
    QMap<IParameterModel::AlarmState, QColor> alarm_background_colors_; /**< Background colors for alarm states */
    QMap<IParameterModel::AlarmState, QColor> alarm_text_colors_;       /**< Text colors for alarm states */
    
    QColor default_background_color_;         /**< Default background color */
    QColor default_text_color_;               /**< Default text color */
    
    QTimer blink_timer_;                      /**< Timer for blinking in alarm states */
    QTimer* update_timer_;                    /**< Timer for periodic updates */
    
    mutable QMutex mutex_;                    /**< Thread safety */
};

#endif // PARAMETER_VIEW_H 