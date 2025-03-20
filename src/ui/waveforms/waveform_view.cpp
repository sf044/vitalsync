/**
 * @file waveform_view.cpp
 * @brief Implementation of the WaveformView class
 *
 * This file contains the implementation of the WaveformView class, which provides
 * a graphical display of physiological waveform data such as ECG, respiration,
 * and other vital signs. It handles real-time drawing of waveforms, grid rendering,
 * and response to user interactions.
 */
#include "waveform_view.h"
#include "../../../include/config_manager.h"
#include "../../../include/vital_sync_types.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QFontMetrics>
#include <QPainterPath>
#include <algorithm> // For std::min
#include <cmath>

/**
 * @namespace Anonymous namespace for constants
 * @brief Contains constants used by the WaveformView implementation
 */
// Constants
namespace {
    const int UPDATE_INTERVAL_MS = 50;      /**< Update interval in milliseconds */
    const int DEFAULT_GRID_MAJOR_X = 50;    /**< Default major grid spacing in X direction */
    const int DEFAULT_GRID_MAJOR_Y = 50;    /**< Default major grid spacing in Y direction */
    const int DEFAULT_GRID_MINOR_X = 10;    /**< Default minor grid spacing in X direction */
    const int DEFAULT_GRID_MINOR_Y = 10;    /**< Default minor grid spacing in Y direction */
    const int LABEL_MARGIN = 5;             /**< Margin for labels in pixels */
    const int WAVEFORM_MARGIN = 20;         /**< Margin around waveform in pixels */
}

/**
 * @brief Constructs a WaveformView widget
 * @param parent The parent widget
 *
 * Initializes a new WaveformView with default settings and starts the 
 * display timer to continuously update the waveform.
 */
WaveformView::WaveformView(QWidget* parent)
    : QWidget(parent)
    , model_(nullptr)
    , display_timer_(new QTimer(this))
    , axis_x_(0.0)
    , sweep_speed_(25.0)
    , grid_visible_(true)
    , grid_color_(Qt::darkGray)
    , time_scale_visible_(true)
    , amplitude_scale_visible_(true)
    , background_color_(Qt::black)
    , is_paused_(false)
    , wave_form_data_counter_(0)
{
    // Set up widget properties
    setMinimumSize(300, 100);
    
    // Ensure the background is painted properly
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, background_color_);
    setPalette(pal);
    
    // Initialize the path
    waveform_path_ = QPainterPath();
    draw_starting_point_ = QPointF(-5, 0); // Start off-screen exactly like original
    
    // Create update timer with EXACT same interval as original (25ms)
    // In original: draw_timer.start(25);
    display_timer_->setInterval(25);
    connect(display_timer_, &QTimer::timeout, this, &WaveformView::UpdateDisplay);
    
    // Start the timer
    display_timer_->start();
}

/**
 * @brief Destroys the WaveformView widget
 *
 * Stops the display timer and disconnects any model signals.
 */
WaveformView::~WaveformView()
{
    // Stop the timer
    if (display_timer_->isActive()) {
        display_timer_->stop();
    }
    
    // Disconnect model signals
    if (model_) {
        disconnectModelSignals();
    }
}

/**
 * @brief Sets the waveform model for this view
 * @param model The shared pointer to the waveform model
 *
 * Disconnects signals from any previous model, sets the new model,
 * and connects signals from the new model.
 */
void WaveformView::SetModel(std::shared_ptr<IWaveformModel> model)
{
    QMutexLocker locker(&mutex_);
    
    // Disconnect signals from old model
    if (model_) {
        disconnectModelSignals();
    }
    
    // Set new model
    model_ = model;
    
    // Connect signals to new model
    if (model_) {
        connectModelSignals();
    }
    
    // Force update
    update();
}

/**
 * @brief Gets the current waveform model
 * @return Shared pointer to the current waveform model or nullptr if none set
 */
std::shared_ptr<IWaveformModel> WaveformView::GetModel() const
{
    QMutexLocker locker(&mutex_);
    return model_;
}

/**
 * @brief Sets the sweep speed in pixels per second
 * @param pixelsPerSecond The new sweep speed
 *
 * Updates the sweep speed and adjusts the update interval accordingly.
 * Also resets the waveform path for a clean start.
 */
void WaveformView::SetSweepSpeed(double pixelsPerSecond)
{
    QMutexLocker locker(&mutex_);
    
    // Only update if the value has changed
    if (!qFuzzyCompare(sweep_speed_, pixelsPerSecond)) {
        sweep_speed_ = pixelsPerSecond;
        
        // Reset waveform path on sweep speed change for clean start
        waveform_path_ = QPainterPath();
        axis_x_ = 0;
        
        // Adjust update interval immediately based on new sweep speed
        int newInterval = UPDATE_INTERVAL_MS;
        
        if (sweep_speed_ > 50.0) {
            newInterval = 20; // 50 Hz for very fast speeds
        } else if (sweep_speed_ > 25.0) {
            newInterval = 30; // ~33 Hz for fast speeds
        } else if (sweep_speed_ < 12.5) {
            newInterval = 80; // 12.5 Hz for slow speeds
        }
        
        // Only change the interval if needed
        if (display_timer_->interval() != newInterval) {
            display_timer_->setInterval(newInterval);
        }
        
        // Force an immediate update
        update();
    }
}

/**
 * @brief Gets the current sweep speed
 * @return The current sweep speed in pixels per second
 */
double WaveformView::GetSweepSpeed() const
{
    QMutexLocker locker(&mutex_);
    return sweep_speed_;
}

/**
 * @brief Sets whether the grid is visible
 * @param visible True if the grid should be visible, false otherwise
 */
void WaveformView::SetGridVisible(bool visible)
{
    QMutexLocker locker(&mutex_);
    
    if (grid_visible_ != visible) {
        grid_visible_ = visible;
        update();
    }
}

/**
 * @brief Gets whether the grid is visible
 * @return True if the grid is visible, false otherwise
 */
bool WaveformView::isGridVisible() const
{
    QMutexLocker locker(&mutex_);
    return grid_visible_;
}

/**
 * @brief Sets whether the time scale is visible
 * @param visible True if the time scale should be visible, false otherwise
 */
void WaveformView::SetTimeScaleVisible(bool visible)
{
    QMutexLocker locker(&mutex_);
    
    if (time_scale_visible_ != visible) {
        time_scale_visible_ = visible;
        update();
    }
}

/**
 * @brief Gets whether the time scale is visible
 * @return True if the time scale is visible, false otherwise
 */
bool WaveformView::isTimeScaleVisible() const
{
    QMutexLocker locker(&mutex_);
    return time_scale_visible_;
}

/**
 * @brief Sets whether the amplitude scale is visible
 * @param visible True if the amplitude scale should be visible, false otherwise
 */
void WaveformView::SetAmplitudeScaleVisible(bool visible)
{
    QMutexLocker locker(&mutex_);
    
    if (amplitude_scale_visible_ != visible) {
        amplitude_scale_visible_ = visible;
        update();
    }
}

/**
 * @brief Gets whether the amplitude scale is visible
 * @return True if the amplitude scale is visible, false otherwise
 */
bool WaveformView::isAmplitudeScaleVisible() const
{
    QMutexLocker locker(&mutex_);
    return amplitude_scale_visible_;
}

/**
 * @brief Sets the grid color
 * @param color The new grid color
 */
void WaveformView::SetGridColor(const QColor& color)
{
    QMutexLocker locker(&mutex_);
    
    if (grid_color_ != color) {
        grid_color_ = color;
        update();
    }
}

/**
 * @brief Gets the grid color
 * @return The current grid color
 */
QColor WaveformView::GetGridColor() const
{
    QMutexLocker locker(&mutex_);
    return grid_color_;
}

/**
 * @brief Sets the background color
 * @param color The new background color
 *
 * Updates the widget's palette to use the new background color.
 */
void WaveformView::SetBackgroundColor(const QColor& color)
{
    QMutexLocker locker(&mutex_);
    
    if (background_color_ != color) {
        background_color_ = color;
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        setPalette(pal);
        update();
    }
}

/**
 * @brief Gets the background color
 * @return The current background color
 */
QColor WaveformView::GetBackgroundColor() const
{
    QMutexLocker locker(&mutex_);
    return background_color_;
}

/**
 * @brief Updates the widget
 *
 * Calls the base QWidget::update() method.
 */
void WaveformView::update()
{
    this->QWidget::update();
}

/**
 * @brief Sets whether the waveform display is paused
 * @param paused True to pause the display, false to resume
 *
 * When paused, the display timer is stopped and a "PAUSED" indicator is shown.
 */
void WaveformView::SetPaused(bool paused)
{
    QMutexLocker locker(&mutex_);
    
    if (is_paused_ != paused) {
        is_paused_ = paused;
        
        // Start or stop display timer based on pause state
        if (is_paused_) {
            if (display_timer_->isActive()) {
                display_timer_->stop();
            }
        } else {
            if (!display_timer_->isActive()) {
                display_timer_->start();
            }
        }
        
        update();
    }
}

/**
 * @brief Gets whether the waveform display is paused
 * @return True if the display is paused, false otherwise
 */
bool WaveformView::isPaused() const
{
    QMutexLocker locker(&mutex_);
    return is_paused_;
}

/**
 * @brief Handles paint events for the widget
 * @param event The paint event
 *
 * Draws the background, grid, waveform, labels, and pause indicator if paused.
 */
void WaveformView::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    
    // Ensure the background is filled with the correct color
    painter.fillRect(rect(), background_color_);
    
    // Draw grid if visible
    if (grid_visible_) {
        drawGrid(painter);
    }
    
    // Draw waveform if we have a model
    if (model_) {
        drawWaveform(painter);
    }
    
    // Draw labels
    drawLabels(painter);
    
    // Draw pause indicator if paused
    if (is_paused_) {
        painter.setPen(QPen(Qt::white, 2));
        QFont pauseFont = painter.font();
        pauseFont.setBold(true);
        pauseFont.setPointSize(14);
        painter.setFont(pauseFont);
        painter.drawText(rect(), Qt::AlignCenter, tr("PAUSED"));
    }
}

/**
 * @brief Handles resize events for the widget
 * @param event The resize event
 */
void WaveformView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

/**
 * @brief Handles data updated signals from the model
 *
 * Updates a small region of the widget around the current drawing position.
 */
void WaveformView::HandleDataUpdated()
{
    // Only update if not paused
    if (!is_paused_) {
        // Create a small rect around the current drawing position to update
        // This is more efficient than updating the entire widget
        QRect UpdateRect = rect();
        int margin = 20; // Provide enough margin for the curve
        
        int x = qBound(0, static_cast<int>(axis_x_ - margin), width());
        int width = qMin(2 * margin, this->width() - x);
        
        UpdateRect.setLeft(x);
        UpdateRect.setWidth(width);
        
        // Update just that section of the widget
        QWidget::update(UpdateRect);
    }
}

/**
 * @brief Handles property changes in the model
 *
 * Updates the widget if not paused and a model is set.
 */
void WaveformView::HandlePropertiesChanged()
{
    // Only update if not paused and we have a model
    if (!is_paused_ && model_) {
        update();
    }
}

/**
 * @brief Updates the display timer and redraws the waveform
 *
 * Called by the display timer to update the waveform display. Adjusts
 * the display timer interval based on the sweep speed.
 */
void WaveformView::UpdateDisplay()
{
    // Skip updates if paused
    if (is_paused_) {
        return;
    }
    
    // Adjust the display update interval based on sweep speed to match old implementation
    // Faster sweep speeds need more frequent updates
    int newInterval = 25; // Use the EXACT original timer interval (25ms)
    
    if (sweep_speed_ > 50.0) {
        // Very fast sweep speeds need more frequent updates
        newInterval = 20; // 50 Hz
    } else if (sweep_speed_ > 25.0) {
        // Fast sweep speeds
        newInterval = 30; // ~33 Hz
    } else if (sweep_speed_ < 12.5) {
        // Slow sweep speeds can update less frequently
        newInterval = 80; // 12.5 Hz
    }
    
    // Update timer interval if needed
    if (display_timer_->interval() != newInterval) {
        display_timer_->setInterval(newInterval);
    }
    
    // Trigger a redraw to update the scrolling waveform
    // In original: update(waveformBoundingRect);
    if (waveform_path_.elementCount() > 0) {
        // Create a bounding rect for optimized updates - identical to original
        // In original: waveformBoundingRect.SetTopLeft(QPoint(axisX,0));
        // In original: waveformBoundingRect.SetSize(QSize(15,this->height()));
        QRect waveformBoundingRect;
        waveformBoundingRect.setTopLeft(QPoint(static_cast<int>(axis_x_), 0));
        waveformBoundingRect.setSize(QSize(15, this->height()));
        
        // Update ONLY the waveform bounding rect - exactly like original
        QWidget::update(waveformBoundingRect);
    } else {
        QWidget::update();
    }
}

/**
 * @brief Connects signals from the model
 *
 * Connects the dataUpdated and propertiesChanged signals from the model
 * to the corresponding handler methods in this class.
 */
void WaveformView::connectModelSignals()
{
    if (model_) {
        connect(model_.get(), &IWaveformModel::dataUpdated, this, &WaveformView::HandleDataUpdated);
        connect(model_.get(), &IWaveformModel::propertiesChanged, this, &WaveformView::HandlePropertiesChanged);
    }
}

/**
 * @brief Disconnects signals from the model
 *
 * Disconnects the dataUpdated and propertiesChanged signals from the model.
 */
void WaveformView::disconnectModelSignals()
{
    if (model_) {
        disconnect(model_.get(), &IWaveformModel::dataUpdated, this, &WaveformView::HandleDataUpdated);
        disconnect(model_.get(), &IWaveformModel::propertiesChanged, this, &WaveformView::HandlePropertiesChanged);
    }
}

/**
 * @brief Draws the grid
 * @param painter The painter to use
 *
 * Draws minor and major grid lines with different styles.
 */
void WaveformView::drawGrid(QPainter& painter)
{
    QRect gridRect = rect().adjusted(WAVEFORM_MARGIN, WAVEFORM_MARGIN, 
                                     -WAVEFORM_MARGIN, -WAVEFORM_MARGIN);
    
    // Draw grid with improved visibility
    // Use semi-transparent lines for minor grid
    QPen gridPen(QColor(grid_color_.red(), grid_color_.green(), grid_color_.blue(), 100), 0.7, Qt::DotLine);
    // Use more visible lines for major grid
    QPen majorGridPen(grid_color_, 0.9, Qt::SolidLine);
    
    // Draw minor grid lines
    painter.setPen(gridPen);
    
    // Vertical minor grid lines (time)
    for (int x = gridRect.left(); x <= gridRect.right(); x += DEFAULT_GRID_MINOR_X) {
        painter.drawLine(x, gridRect.top(), x, gridRect.bottom());
    }
    
    // Horizontal minor grid lines (amplitude)
    for (int y = gridRect.top(); y <= gridRect.bottom(); y += DEFAULT_GRID_MINOR_Y) {
        painter.drawLine(gridRect.left(), y, gridRect.right(), y);
    }
    
    // Draw major grid lines
    painter.setPen(majorGridPen);
    
    // Vertical major grid lines
    for (int x = gridRect.left(); x <= gridRect.right(); x += DEFAULT_GRID_MAJOR_X) {
        painter.drawLine(x, gridRect.top(), x, gridRect.bottom());
    }
    
    // Horizontal major grid lines
    for (int y = gridRect.top(); y <= gridRect.bottom(); y += DEFAULT_GRID_MAJOR_Y) {
        painter.drawLine(gridRect.left(), y, gridRect.right(), y);
    }
}

/**
 * @brief Draws the waveform
 * @param painter The painter to use
 *
 * Draws the waveform using the data from the model. Sets up the coordinate
 * transformation and draws the waveform path.
 */
void WaveformView::drawWaveform(QPainter& painter)
{
    if (!model_) {
        qDebug() << "WaveformView::drawWaveform - No model attached";
        return;
    }
    
    QMutexLocker locker(&mutex_);
    
    // Get waveform data
    const QVector<float>& data = model_->GetData();
    if (data.isEmpty() && !model_->GetIsDemo()) {
        qDebug() << "WaveformView::drawWaveform - Empty data for waveform ID:" << model_->GetWaveformId();
        return;
    }
    
    // Get model properties
    QColor waveformColor = model_->GetColor();
    float minValue = model_->GetMinValue();
    float maxValue = model_->GetMaxValue();
    double valueRange = maxValue - minValue;
    int waveformId = model_->GetWaveformId();
    
    // Safety check for value range
    if (qFuzzyCompare(valueRange, 0.0)) {
        valueRange = 1.0; // Prevent division by zero
    }
    
    // IMPORTANT: Use the entire widget area with no margins
    QRect drawRect = rect();
    
    // Set up coordinate transformation exactly like the original:
    // In original PaintWaveform: m.translate(0,size().height()); m.scale(1,-1);
    QTransform transform;
    transform.translate(0, size().height());
    transform.scale(1, -1);
    painter.setTransform(transform);
    
    // Set up pen for drawing waveform - using the same settings as the old implementation
    QPen waveformPen;
    
    // Apply different pen settings based on waveform type (same as old implementation)
    switch (static_cast<VitalSync::WaveformType>(waveformId)) {
        case VitalSync::WaveformType::ECG_I:
        case VitalSync::WaveformType::ECG_II:
        case VitalSync::WaveformType::ECG_III:
            waveformPen = QPen(Qt::green, 1.5, Qt::SolidLine);
            break;
        case VitalSync::WaveformType::RESP:
            waveformPen = QPen(Qt::yellow, 1.5, Qt::SolidLine);
            break;
        case VitalSync::WaveformType::PLETH:
            waveformPen = QPen(Qt::cyan, 1.5, Qt::SolidLine);
            break;
        case VitalSync::WaveformType::ABP:
            waveformPen = QPen(Qt::red, 1.5, Qt::SolidLine);
            break;
        case VitalSync::WaveformType::CAPNO:
            waveformPen = QPen(Qt::white, 1.5, Qt::SolidLine);
            break;
        default:
            waveformPen = QPen(waveformColor, 1.5, Qt::SolidLine);
            break;
    }
    
    painter.setPen(waveformPen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Set the waveform bounding rect for partial updates - identical to original
    QRect waveformBoundingRect;
    waveformBoundingRect.setTopLeft(QPoint(static_cast<int>(axis_x_), 0));
    waveformBoundingRect.setSize(QSize(15, this->height()));
    
    // Path initialization - EXACTLY like original
    // In original: waveformDrawingPath.moveTo(drawStartingPoint);
    if (waveform_path_.elementCount() == 0) {
        waveform_path_ = QPainterPath();
        waveform_path_.moveTo(draw_starting_point_);
        axis_x_ = 0;
    }
    
    // Get current Y value - EXACTLY like original calculation
    float liveTrace = 0.0f;
    
    // Process data just like the original implementation
    if (model_->GetIsDemo()) {
        // In original: chooseWaveFormType(waveformOption, this->height());
        liveTrace = ProcessDemoData(waveformId);
    } else if (!data.isEmpty()) {
        // Use real data
        int index = wave_form_data_counter_ % data.size();
        float value = data[index];
        liveTrace = (value - minValue) / valueRange;
    } else {
        liveTrace = 0.5f;
    }
    
    // Scale value based on waveform type and height - like in chooseWaveFormType
    float scaledValue = 0;
    
    switch (static_cast<VitalSync::WaveformType>(waveformId)) {
        case VitalSync::WaveformType::ECG_I:
        case VitalSync::WaveformType::ECG_II:
        case VitalSync::WaveformType::ECG_III:
            scaledValue = liveTrace * (drawRect.height() * 0.7); // Increased from height/2.0
            break;
        case VitalSync::WaveformType::PLETH:
            scaledValue = liveTrace * (drawRect.height() * 0.5); // Increased from height/3.0
            break;
        case VitalSync::WaveformType::ABP:
            scaledValue = liveTrace * (drawRect.height() * 0.5); // Increased from height/3.0
            break;
        case VitalSync::WaveformType::RESP:
            scaledValue = liveTrace * (drawRect.height() * 0.8); // Significantly increased from height/2.0
            break;
        case VitalSync::WaveformType::CAPNO:
            scaledValue = liveTrace * (drawRect.height() * 0.8); // Significantly increased from height/2.0
            break;
        default:
            scaledValue = liveTrace * (drawRect.height() * 0.7); // Increased from height/2.0
            break;
    }
    
    // Create drawing points - EXACTLY like original
    // In original: QPointF drawEdingPoint(axisX, singletonData->liveTrace);
    QPointF drawEndingPoint(axis_x_, scaledValue);
    
    // Increment x position by exactly 1 pixel - identical to original: axisX+=1;
    axis_x_ += 1.0;
    
    // Handle edge of screen - identical to original reset logic
    if (axis_x_ >= drawRect.width()) {
        axis_x_ = 0;
        waveform_path_ = QPainterPath();
        waveform_path_.moveTo(axis_x_, scaledValue);
        draw_starting_point_ = QPointF(axis_x_, scaledValue);
    } else {
        // Set drawing points EXACTLY as in original
        // Special case for ABP - identical to original
        // In original: if(waveformOption == WaveFormSettingEnum::ibp1) drawEdingPoint.SetY(0);
        if (static_cast<VitalSync::WaveformType>(waveformId) == VitalSync::WaveformType::ABP) {
            drawEndingPoint.setY(0);
        }
        
        // Set starting point - EXACTLY like original
        // In original: drawStartingPoint.SetX(axisX); drawStartingPoint.SetY(singletonData->liveTrace);
        draw_starting_point_.setX(axis_x_);
        draw_starting_point_.setY(scaledValue);
        
        // Set ending point X - EXACTLY like original
        // In original: drawEdingPoint.SetX(axisX);
        drawEndingPoint.setX(axis_x_);
        
        // Use quadratic curve - EXACTLY as in the original:
        // In original: waveformDrawingPath.quadTo(drawStartingPoint, drawEdingPoint);
        waveform_path_.quadTo(draw_starting_point_, drawEndingPoint);
        
        // Update for next iteration - EXACTLY like original:
        // In original: drawStartingPoint = drawEdingPoint;
        draw_starting_point_ = drawEndingPoint;
    }
    
    // Draw the waveform path - identical to original
    painter.drawPath(waveform_path_);
    
    // Reset transform
    painter.resetTransform();
}

/**
 * @brief Processes demo waveform data based on waveform type
 * @param waveformId The waveform ID
 * @return Processed demo value normalized to range 0-1
 *
 * Processes the demo data for the specified waveform type, applying
 * scaling and normalization appropriate for the waveform type.
 */
float WaveformView::ProcessDemoData(int waveformId)
{
    // Exactly match original counter logic
    // In original: waveFormDataCounter++;
    wave_form_data_counter_++;
    
    // Reset counter at same threshold as original
    // In original: if(waveFormDataCounter>=50) waveFormDataCounter=0;
    if (wave_form_data_counter_ >= 50) {
        wave_form_data_counter_ = 0;
    }
    
    float liveTrace = 0.0f;
    
    // Choose waveform type and apply improved scaling
    switch (static_cast<VitalSync::WaveformType>(waveformId)) {
        case VitalSync::WaveformType::ECG_I:
        case VitalSync::WaveformType::ECG_II:
        case VitalSync::WaveformType::ECG_III: {
            // Improved ECG scaling - slightly higher amplitude
            float waveformData = ecgDemoData[wave_form_data_counter_] / 25.0f + 100.0f; // Reduced divisor for more height
            liveTrace = (waveformData * (this->height() / 200.0)) - 25;
            // Normalize to range 0-1
            liveTrace = liveTrace / this->height() + 0.5f;
            break;
        }
        case VitalSync::WaveformType::PLETH: {
            // Improved PLETH scaling - greater amplitude
            float waveformData = spo2DemoData[wave_form_data_counter_] / 22.0f + 100.0f; // Reduced divisor for more height
            liveTrace = (waveformData * (this->height() / 200.0)) - 35;
            // Normalize to range 0-1
            liveTrace = liveTrace / this->height() + 0.5f;
            break;
        }
        case VitalSync::WaveformType::ABP: {
            // Improved ABP scaling - show more detail
            float waveformData = ibp1PhasicDemoData[wave_form_data_counter_] / 30.0f; // Reduced divisor from 40.0f
            liveTrace = (waveformData * (this->height() / 200.0)) - 25;
            // Normalize to range 0-1
            liveTrace = liveTrace / this->height() + 0.5f;
            break;
        }
        case VitalSync::WaveformType::RESP: {
            // Significantly improved RESP scaling - much taller
            float waveformData = (respSimulate[wave_form_data_counter_] / 2.0f) / 100.0f + 20.0f; // Reduced divisor from 8.0f to 2.0f
            liveTrace = (waveformData * (this->height() / 200.0)) - 25;
            // Normalize to range 0-1
            liveTrace = liveTrace / this->height() + 0.5f;
            break;
        }
        case VitalSync::WaveformType::CAPNO: {
            // Significantly improved CAPNO scaling - much taller
            float waveformData = (respSimulate[wave_form_data_counter_] / 1.5f) / 100.0f + 20.0f; // Reduced divisor from 8.0f to 1.5f
            liveTrace = (waveformData * (this->height() / 200.0)) - 25;
            // Normalize to range 0-1
            liveTrace = liveTrace / this->height() + 0.5f;
            break;
        }
        default: {
            // Default to ECG scaling
            float waveformData = ecgDemoData[wave_form_data_counter_] / 25.0f + 100.0f;
            liveTrace = (waveformData * (this->height() / 200.0)) - 25;
            // Normalize to range 0-1
            liveTrace = liveTrace / this->height() + 0.5f;
            break;
        }
    }
    
    return liveTrace;
}

/**
 * @brief Draws the labels
 * @param painter The painter to use
 *
 * Draws the waveform name, min/max values, and time scale.
 */
void WaveformView::drawLabels(QPainter& painter)
{
    if (!model_) return;
    
    QRect drawRect = rect();
    QPen labelPen(Qt::white);
    painter.setPen(labelPen);
    
    // Draw waveform name in top-left corner
    QString name = model_->GetDisplayName();
    QFont nameFont = painter.font();
    nameFont.setBold(true);
    painter.setFont(nameFont);
    painter.drawText(drawRect.adjusted(LABEL_MARGIN, LABEL_MARGIN, 0, 0), 
                    Qt::AlignLeft | Qt::AlignTop, name);
    
    // Draw min/max values in top-right corner if amplitude scale is visible
    if (amplitude_scale_visible_) {
        float minValue = model_->GetMinValue();
        float maxValue = model_->GetMaxValue();
        
        QString minMaxText = QString::number(maxValue, 'f', 1) + "\n" + 
                             QString::number(minValue, 'f', 1);
        
        painter.drawText(drawRect.adjusted(0, LABEL_MARGIN, -LABEL_MARGIN, 0), 
                        Qt::AlignRight | Qt::AlignTop, minMaxText);
    }
    
    // Draw time scale in bottom-right if time scale is visible
    if (time_scale_visible_) {
        QString timeText = QString::number(sweep_speed_, 'f', 1) + " mm/s";
        painter.drawText(drawRect.adjusted(0, 0, -LABEL_MARGIN, -LABEL_MARGIN), 
                        Qt::AlignRight | Qt::AlignBottom, timeText);
    }
} 