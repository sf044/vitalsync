/**
 * @file parameter_view.cpp
 * @brief Implementation of the ParameterView class
 *
 * This file contains the implementation of the ParameterView class, which provides
 * a graphical display of physiological parameters such as heart rate, blood pressure,
 * and SpO2. It handles formatting and displaying parameter values, and provides
 * visual feedback for alarm states such as high or low values.
 */
#include "parameter_view.h"
#include "../../../include/config_manager.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

/**
 * @namespace Anonymous namespace for constants
 * @brief Contains constants used by the ParameterView implementation
 */
// Constants
namespace {
    const int DEFAULT_BLINK_INTERVAL_MS = 500;      /**< Blink interval for alarms in ms */
    const int DEFAULT_VALUE_FONT_SIZE = 24;         /**< Default font size for value */
    const int DEFAULT_LABEL_FONT_SIZE = 12;         /**< Default font size for label */
}

/**
 * @brief Constructs a ParameterView widget
 * @param parent The parent widget
 *
 * Initializes a new ParameterView with default settings and UI components.
 * Sets up the layout, fonts, colors, and alarm settings.
 */
ParameterView::ParameterView(QWidget* parent)
    : QWidget(parent)
    , label_widget_(nullptr)
    , value_widget_(nullptr)
    , unit_widget_(nullptr)
    , label_visible_(true)
    , unit_visible_(true)
    , value_font_size_(DEFAULT_VALUE_FONT_SIZE)
    , label_font_size_(DEFAULT_LABEL_FONT_SIZE)
    , background_color_(Qt::black)
    , text_color_(Qt::white)
    , current_alarm_state_(IParameterModel::AlarmState::Normal)
    , alarm_blink_state_(false)
    , update_timer_(nullptr)
{
    // Set up default widget
    setMinimumSize(100, 60);
    
    // Create UI components
    label_widget_ = new QLabel(this);
    value_widget_ = new QLabel(this);
    unit_widget_ = new QLabel(this);
    
    // Set up fonts
    QFont valueFont = value_widget_->font();
    valueFont.setPointSize(value_font_size_);
    valueFont.setBold(true);
    value_widget_->setFont(valueFont);
    
    QFont labelFont = label_widget_->font();
    labelFont.setPointSize(label_font_size_);
    label_widget_->setFont(labelFont);
    unit_widget_->setFont(labelFont);
    
    // Set alignments
    label_widget_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    value_widget_->setAlignment(Qt::AlignCenter);
    unit_widget_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    // Set background and colors
    SetBackgroundColor(background_color_);
    SetTextColor(text_color_);
    
    // Set up layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(2);
    
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(label_widget_, 3);
    headerLayout->addStretch(1);
    headerLayout->addWidget(unit_widget_, 1);
    
    mainLayout->addLayout(headerLayout, 1);
    mainLayout->addWidget(value_widget_, 3);
    
    setLayout(mainLayout);
    
    // Set up blink timer
    blink_timer_.setInterval(DEFAULT_BLINK_INTERVAL_MS);
    connect(&blink_timer_, &QTimer::timeout, this, &ParameterView::UpdateDisplay);
    
    // Set up default alarm colors
    alarm_background_colors_[IParameterModel::AlarmState::Normal] = Qt::black;
    alarm_background_colors_[IParameterModel::AlarmState::HighWarning] = QColor(60, 30, 0);  // Dark orange
    alarm_background_colors_[IParameterModel::AlarmState::HighCritical] = QColor(60, 0, 0);  // Dark red
    alarm_background_colors_[IParameterModel::AlarmState::LowWarning] = QColor(0, 0, 60);    // Dark blue
    alarm_background_colors_[IParameterModel::AlarmState::LowCritical] = QColor(50, 0, 50);  // Dark purple
    alarm_background_colors_[IParameterModel::AlarmState::Technical] = QColor(50, 50, 0);    // Dark yellow
    
    alarm_text_colors_[IParameterModel::AlarmState::Normal] = Qt::white;
    alarm_text_colors_[IParameterModel::AlarmState::HighWarning] = QColor(255, 160, 0);     // Orange
    alarm_text_colors_[IParameterModel::AlarmState::HighCritical] = QColor(255, 0, 0);      // Red
    alarm_text_colors_[IParameterModel::AlarmState::LowWarning] = QColor(100, 100, 255);    // Blue
    alarm_text_colors_[IParameterModel::AlarmState::LowCritical] = QColor(255, 0, 255);     // Purple
    alarm_text_colors_[IParameterModel::AlarmState::Technical] = QColor(255, 255, 0);       // Yellow
}

/**
 * @brief Destroys the ParameterView widget
 *
 * Stops the blink timer and disconnects any model signals.
 */
ParameterView::~ParameterView()
{
    // Stop the timer
    if (blink_timer_.isActive()) {
        blink_timer_.stop();
    }
    
    // Disconnect model signals
    if (model_) {
        disconnectModelSignals();
    }
}

/**
 * @brief Sets the parameter model for this view
 * @param model The shared pointer to the parameter model
 *
 * Disconnects signals from any previous model, sets the new model,
 * updates the UI with data from the model, and connects signals.
 */
void ParameterView::SetModel(std::shared_ptr<IParameterModel> model)
{
    // Disconnect old model signals
    if (model_) {
        disconnectModelSignals();
    }
    
    // Set new model
    model_ = model;
    
    if (model_) {
        // Update UI with model data
        label_widget_->setText(model_->GetDisplayName());
        
        // Format the initial value properly using the same logic as HandleValueChanged
        float value = model_->GetValue();
        if (model_->GetUnit() == "%") {
            value_widget_->setText(QString::number(static_cast<int>(value)));
        } else if (model_->GetUnit().contains("°C")) {
            value_widget_->setText(QString::number(value, 'f', 1));
        } else {
            int precision = (value >= 100.0) ? 0 : 1;
            value_widget_->setText(QString::number(value, 'f', precision));
        }
        
        unit_widget_->setText(model_->GetUnit());
        
        // Get current alarm state
        current_alarm_state_ = model_->GetAlarmState();
        
        // Connect model signals
        connectModelSignals();
        
        // Update appearance
        UpdateAlarmAppearance();
        
        // Force initial update to ensure everything is displayed correctly
        QTimer::singleShot(100, this, [this]() {
            if (model_) {
                HandleValueChanged(model_->GetValue());
            }
        });
    } else {
        // Clear UI
        label_widget_->setText("--");
        value_widget_->setText("--");
        unit_widget_->setText("");
        
        // Reset alarm state
        current_alarm_state_ = IParameterModel::AlarmState::Normal;
        
        // Update appearance
        UpdateAlarmAppearance();
    }
    
    this->QWidget::update();
}

/**
 * @brief Gets the current parameter model
 * @return Shared pointer to the current parameter model or nullptr if none set
 */
std::shared_ptr<IParameterModel> ParameterView::GetModel() const
{
    QMutexLocker locker(&mutex_);
    return model_;
}

/**
 * @brief Gets the widget associated with this view
 * @return Pointer to the widget
 */
QWidget* ParameterView::GetWidget()
{
    return this;
}

/**
 * @brief Sets whether to show the parameter label
 * @param visible True to show the label, false to hide it
 */
void ParameterView::SetLabelVisible(bool visible)
{
    QMutexLocker locker(&mutex_);
    
    if (label_visible_ != visible) {
        label_visible_ = visible;
        label_widget_->setVisible(visible);
        this->QWidget::update();
    }
}

/**
 * @brief Sets whether to show the unit label
 * @param visible True to show the unit, false to hide it
 */
void ParameterView::SetUnitVisible(bool visible)
{
    QMutexLocker locker(&mutex_);
    
    if (unit_visible_ != visible) {
        unit_visible_ = visible;
        unit_widget_->setVisible(visible);
        this->QWidget::update();
    }
}

/**
 * @brief Sets the font size for the value display
 * @param size Font size in points
 */
void ParameterView::SetValueFontSize(int size)
{
    QMutexLocker locker(&mutex_);
    
    if (value_font_size_ != size) {
        value_font_size_ = size;
        QFont font = value_widget_->font();
        font.setPointSize(size);
        value_widget_->setFont(font);
        this->QWidget::update();
    }
}

/**
 * @brief Sets the font size for the label display
 * @param size Font size in points
 */
void ParameterView::SetLabelFontSize(int size)
{
    QMutexLocker locker(&mutex_);
    
    if (label_font_size_ != size) {
        label_font_size_ = size;
        QFont font = label_widget_->font();
        font.setPointSize(size);
        label_widget_->setFont(font);
        
        font = unit_widget_->font();
        font.setPointSize(size);
        unit_widget_->setFont(font);
        
        this->QWidget::update();
    }
}

/**
 * @brief Sets the background color
 * @param color Background color
 */
void ParameterView::SetBackgroundColor(const QColor& color)
{
    QMutexLocker locker(&mutex_);
    
    if (background_color_ != color) {
        background_color_ = color;
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        setPalette(pal);
        this->QWidget::update();
    }
}

/**
 * @brief Sets the text color for normal state
 * @param color Text color
 */
void ParameterView::SetTextColor(const QColor& color)
{
    QMutexLocker locker(&mutex_);
    
    if (text_color_ != color) {
        text_color_ = color;
        
        // Update alarm colors for normal state
        alarm_text_colors_[IParameterModel::AlarmState::Normal] = color;
        
        // If currently in normal state, update the text colors
        if (current_alarm_state_ == IParameterModel::AlarmState::Normal) {
            QPalette pal = label_widget_->palette();
            pal.setColor(QPalette::WindowText, color);
            label_widget_->setPalette(pal);
            value_widget_->setPalette(pal);
            unit_widget_->setPalette(pal);
        }
        
        this->QWidget::update();
    }
}

/**
 * @brief Sets alarm background color for a specific state
 * @param state Alarm state to set color for
 * @param color Color to use for the alarm state
 */
void ParameterView::SetAlarmBackgroundColor(IParameterModel::AlarmState state, const QColor& color)
{
    QMutexLocker locker(&mutex_);
    
    if (alarm_background_colors_[state] != color) {
        alarm_background_colors_[state] = color;
        
        // If currently in this alarm state, update the background
        if (current_alarm_state_ == state) {
            UpdateAlarmAppearance();
        }
    }
}

/**
 * @brief Sets alarm text color for a specific state
 * @param state Alarm state to set color for
 * @param color Color to use for the alarm state
 */
void ParameterView::SetAlarmTextColor(IParameterModel::AlarmState state, const QColor& color)
{
    QMutexLocker locker(&mutex_);
    
    if (alarm_text_colors_[state] != color) {
        alarm_text_colors_[state] = color;
        
        // If currently in this alarm state, update the text colors
        if (current_alarm_state_ == state) {
            UpdateAlarmAppearance();
        }
    }
}

/**
 * @brief Updates the widget
 */
void ParameterView::update()
{
    this->QWidget::update();
}

/**
 * @brief Handles paint events for the widget
 * @param event The paint event
 */
void ParameterView::paintEvent(QPaintEvent* event)
{
    // Just use the default implementation
    QWidget::paintEvent(event);
}

/**
 * @brief Handles resize events for the widget
 * @param event The resize event
 */
void ParameterView::resizeEvent(QResizeEvent* event)
{
    // Just use the default implementation
    QWidget::resizeEvent(event);
}

/**
 * @brief Handles value changes from the model
 * @param value New value
 *
 * Formats the value based on the parameter type and updates the display.
 */
void ParameterView::HandleValueChanged(float value)
{
    // Format the value depending on the parameter type
    // Example: 98% for SpO2, 120/80 for BP, etc.
    if (!model_) {
        qWarning() << "ParameterView: Cannot handle value change - model is null";
        return;
    }
    
    QString unit = model_->GetUnit();
    
    qDebug() << "ParameterView: Handling value change for" << model_->GetDisplayName() 
             << "to" << value << unit;
    
    QString formatted_value;
    
    // Google style: Use clear if-else chains for readability
    if (unit == "%") {
        // Display whole numbers for percentages
        formatted_value = QString::number(static_cast<int>(value));
    } else if (unit.contains("°C")) {
        // Display one decimal place for temperature
        formatted_value = QString::number(value, 'f', 1);
    } else {
        // For other values, display one decimal place if < 100, otherwise whole number
        int precision = (value >= 100.0) ? 0 : 1;
        formatted_value = QString::number(value, 'f', precision);
    }
    
    // Update the display
    value_widget_->setText(formatted_value);
    
    // Force a repaint to ensure the new value is shown
    this->QWidget::update();
}

/**
 * @brief Handles alarm state changes from the model
 * @param state New alarm state
 */
void ParameterView::HandleAlarmStateChanged(IParameterModel::AlarmState state)
{
    QMutexLocker locker(&mutex_);
    
    if (current_alarm_state_ != state) {
        current_alarm_state_ = state;
        
        // Update the appearance
        UpdateAlarmAppearance();
    }
}

/**
 * @brief Handles property changes from the model
 *
 * Updates the display name, value, unit, and alarm state.
 */
void ParameterView::HandlePropertiesChanged()
{
    if (!model_) {
        return;
    }
    
    qDebug() << "ParameterView: Properties changed for" << model_->GetDisplayName();
    
    // Update display name (label)
    label_widget_->setText(model_->GetDisplayName());
    
    // Update value directly
    float value = model_->GetValue();
    qDebug() << "ParameterView: Updating value to" << value << model_->GetUnit() << "due to properties change";
    HandleValueChanged(value);
    
    // Update alarm state
    HandleAlarmStateChanged(model_->GetAlarmState());
    
    // Update the unit
    unit_widget_->setText(model_->GetUnit());
    
    // Force update to ensure UI is refreshed
    this->update();
    
    qDebug() << "ParameterView: Completed properties update for" << model_->GetDisplayName();
}

/**
 * @brief Updates the display timer - used for blinking in alarm states
 *
 * Toggles the blink state for alarm states that need blinking.
 */
void ParameterView::UpdateDisplay()
{
    // Toggle blink state for alarm states that need blinking
    if (current_alarm_state_ == IParameterModel::AlarmState::HighCritical ||
        current_alarm_state_ == IParameterModel::AlarmState::LowCritical ||
        current_alarm_state_ == IParameterModel::AlarmState::Technical) {
        alarm_blink_state_ = !alarm_blink_state_;
        UpdateAlarmAppearance();
    }
}

/**
 * @brief Connects signals from the model
 *
 * Sets up connections for property changes and creates a timer
 * for periodic updates of value and alarm state.
 */
void ParameterView::connectModelSignals()
{
    if (!model_) {
        qWarning() << "ParameterView: Cannot connect signals - model is null";
        return;
    }

    qDebug() << "ParameterView: Connecting signals for parameter" << model_->GetDisplayName();
    
    // Connect property changes signal directly
    connect(model_.get(), &IParameterModel::propertiesChanged,
            this, &ParameterView::HandlePropertiesChanged);
            
    // Create and start update timer for value and alarm updates
    if (update_timer_ == nullptr) {
        constexpr int kUpdateIntervalMs = 100;  // 100ms = 10 updates per second
        
        update_timer_ = new QTimer(this);
        update_timer_->setInterval(kUpdateIntervalMs);
        
        connect(update_timer_, &QTimer::timeout, this, [this]() {
            if (!model_) {
                return;  // Early return if model is invalid
            }
            
            // Update value
            float current_value = model_->GetValue();
            qDebug() << "ParameterView Timer: Updating" << model_->GetDisplayName() 
                     << "value to" << current_value << model_->GetUnit();
            HandleValueChanged(current_value);
            
            // Update alarm state
            IParameterModel::AlarmState current_state = model_->GetAlarmState();
            HandleAlarmStateChanged(current_state);
        });
        
        qDebug() << "ParameterView: Started update timer for" << model_->GetDisplayName();
        update_timer_->start();
    }
    
    // Initial update to ensure values are displayed immediately
    QTimer::singleShot(0, this, [this]() {
        if (!model_) {
            return;  // Early return if model is invalid
        }
        
        HandleValueChanged(model_->GetValue());
        HandleAlarmStateChanged(model_->GetAlarmState());
        qDebug() << "ParameterView: Performed initial update for" << model_->GetDisplayName();
    });
}

/**
 * @brief Disconnects signals from the model
 *
 * Stops and clears the update timer and disconnects all connections.
 */
void ParameterView::disconnectModelSignals()
{
    if (!model_) {
        return;
    }

    // Stop and clear update timer if exists
    if (update_timer_) {
        update_timer_->stop();
        delete update_timer_;
        update_timer_ = nullptr;
    }
    
    // Disconnect all connections
    disconnect(model_.get(), &IParameterModel::propertiesChanged, this, &ParameterView::HandlePropertiesChanged);
    // Old connections commented out as we're now using timer-based updates
    // disconnect(model_.get(), &IParameterModel::valueChanged, this, &ParameterView::HandleValueChanged);
    // disconnect(model_.get(), &IParameterModel::alarmStateChanged, this, &ParameterView::HandleAlarmStateChanged);
}

/**
 * @brief Updates the visual appearance based on alarm state
 *
 * Sets the background and text colors based on the current alarm state,
 * and handles blinking for critical alarm states.
 */
void ParameterView::UpdateAlarmAppearance()
{
    // Get the appropriate colors for the current alarm state
    QColor bgColor = alarm_background_colors_[current_alarm_state_];
    QColor textColor = alarm_text_colors_[current_alarm_state_];
    
    // For blinking alarms, toggle colors if in blink state
    if (alarm_blink_state_ &&
        (current_alarm_state_ == IParameterModel::AlarmState::HighCritical ||
         current_alarm_state_ == IParameterModel::AlarmState::LowCritical ||
         current_alarm_state_ == IParameterModel::AlarmState::Technical)) {
        // Swap colors for blinking effect
        QColor temp = bgColor;
        bgColor = textColor;
        textColor = temp;
    }
    
    // Update background
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, bgColor);
    setPalette(pal);
    
    // Update text colors
    pal = label_widget_->palette();
    pal.setColor(QPalette::WindowText, textColor);
    label_widget_->setPalette(pal);
    value_widget_->setPalette(pal);
    unit_widget_->setPalette(pal);
    
    // Start or stop blink timer as needed
    if (current_alarm_state_ == IParameterModel::AlarmState::HighCritical ||
        current_alarm_state_ == IParameterModel::AlarmState::LowCritical ||
        current_alarm_state_ == IParameterModel::AlarmState::Technical) {
        if (!blink_timer_.isActive()) {
            blink_timer_.start();
        }
    } else {
        if (blink_timer_.isActive()) {
            blink_timer_.stop();
            alarm_blink_state_ = false;
        }
    }
} 