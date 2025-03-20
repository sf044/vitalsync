/**
 * @file i_waveform_view.h
 * @brief Interface for waveform view components
 * 
 * This interface defines the contract that all waveform view components must implement.
 * Waveform views are responsible for rendering waveform data on screen and providing
 * user interaction with the waveform display. They visualize physiological waveforms
 * such as ECG, respiration, and other continuous signals.
 * 
 * Waveform views handle various display aspects including:
 * - Scrolling waveform display with configurable sweep speed
 * - Time and amplitude grid lines
 * - Scaling and positioning of waveform data
 * - Visual customization (colors, visibility options)
 * - Pause/resume functionality for analysis
 */

#ifndef I_WAVEFORM_VIEW_H
#define I_WAVEFORM_VIEW_H

#include <QWidget>
#include <memory>
#include "i_waveform_model.h"
#include <QColor>

/**
 * @brief Interface for waveform view components
 * 
 * This interface defines the contract that all waveform view components must implement.
 * It provides methods for connecting to a waveform model, configuring display options,
 * and managing the view's lifecycle. Waveform views are responsible for rendering
 * continuous physiological signals such as ECG, respiration, and other waveforms.
 * 
 * A waveform view implementation is responsible for:
 * - Rendering the waveform with appropriate time and amplitude scaling
 * - Providing measurement grids and reference lines
 * - Supporting user interactions such as pausing and scrolling
 * - Maintaining visual consistency with the rest of the application
 */
class IWaveformView {
public:
    /**
     * @brief Virtual destructor
     * 
     * Ensures proper cleanup when derived classes are destroyed through
     * a pointer to this interface.
     */
    virtual ~IWaveformView() = default;

    /**
     * @brief Get the widget for this view
     * @return Pointer to the widget
     * 
     * Returns a pointer to the Qt widget that represents this waveform view.
     * This widget can be added to layouts or set as a central widget in windows.
     */
    virtual QWidget* GetWidget() = 0;

    /**
     * @brief Set the waveform model to display
     * @param model Shared pointer to the waveform model
     * 
     * Associates this view with a waveform model. The view will connect to the model's
     * signals to receive updates when new waveform data is available or when properties
     * change. If a previous model was set, the view should disconnect from it appropriately.
     */
    virtual void SetModel(std::shared_ptr<IWaveformModel> model) = 0;

    /**
     * @brief Get the associated waveform model
     * @return Shared pointer to the waveform model or nullptr if none
     * 
     * Returns the waveform model currently associated with this view.
     * May return nullptr if no model has been set.
     */
    virtual std::shared_ptr<IWaveformModel> GetModel() const = 0;

    /**
     * @brief Set the sweep speed (time scale) for the waveform
     * @param pixelsPerSecond Number of pixels per second of data
     * 
     * Configures how fast the waveform scrolls across the display.
     * Higher values result in a more "stretched" display of the waveform in time,
     * showing more detail but less history. Common values are 25-50 pixels per second.
     * This affects the spacing of time grid lines as well.
     */
    virtual void SetSweepSpeed(double pixelsPerSecond) = 0;

    /**
     * @brief Get the current sweep speed
     * @return Current sweep speed in pixels per second
     * 
     * Returns the current sweep speed setting, which determines how
     * many horizontal pixels represent one second of waveform data.
     */
    virtual double GetSweepSpeed() const = 0;

    /**
     * @brief Set the grid visibility
     * @param visible True to show grid, false to hide
     * 
     * Controls whether the measurement grid is displayed behind the waveform.
     * The grid typically shows time and amplitude reference lines to help
     * in analyzing the waveform visually.
     */
    virtual void SetGridVisible(bool visible) = 0;

    /**
     * @brief Set the time scale (horizontal grid lines) visibility
     * @param visible True to show time scale, false to hide
     * 
     * Controls whether the horizontal grid lines are displayed.
     * These lines mark time intervals (typically seconds) and help
     * in measuring the duration of waveform features.
     */
    virtual void SetTimeScaleVisible(bool visible) = 0;

    /**
     * @brief Set the amplitude scale (vertical grid lines) visibility
     * @param visible True to show amplitude scale, false to hide
     * 
     * Controls whether the vertical grid lines are displayed.
     * These lines mark amplitude intervals and help in measuring
     * the magnitude of waveform features.
     */
    virtual void SetAmplitudeScaleVisible(bool visible) = 0;

    /**
     * @brief Set the grid color
     * @param color Color for the grid lines
     * 
     * Sets the color used for drawing the grid lines.
     * Typically a light color that provides contrast against both
     * the background and the waveform itself.
     */
    virtual void SetGridColor(const QColor& color) = 0;

    /**
     * @brief Set the background color
     * @param color Background color
     * 
     * Sets the background color of the waveform display area.
     * Typically a dark color to provide contrast with the waveform.
     */
    virtual void SetBackgroundColor(const QColor& color) = 0;

    /**
     * @brief Update the view to reflect the current model state
     * 
     * Forces an immediate update of the view to reflect the current state of the
     * associated waveform model. This is typically called automatically when the
     * model changes, but can be called manually if needed.
     */
    virtual void update() = 0;

    /**
     * @brief Pause or resume the waveform display
     * @param paused True to pause, false to resume
     * 
     * Controls whether the waveform display continues to scroll and update with
     * new data. When paused, the view stops scrolling but maintains the current
     * display, allowing detailed examination of waveform features. When resumed,
     * the view begins scrolling again and showing new data.
     */
    virtual void SetPaused(bool paused) = 0;

    /**
     * @brief Check if the waveform display is currently paused
     * @return True if paused, false if running
     * 
     * Returns whether the waveform display is currently in a paused state.
     * When paused, the view stops scrolling but maintains the current display.
     */
    virtual bool isPaused() const = 0;

    /**
     * @brief Check if the grid is visible
     * @return True if the grid is visible
     * 
     * Returns whether the measurement grid is currently visible.
     * This corresponds to the state set by SetGridVisible().
     */
    virtual bool isGridVisible() const = 0;

    /**
     * @brief Check if the time scale is visible
     * @return True if the time scale is visible
     * 
     * Returns whether the horizontal time grid lines are currently visible.
     * This corresponds to the state set by SetTimeScaleVisible().
     */
    virtual bool isTimeScaleVisible() const = 0;

    /**
     * @brief Check if the amplitude scale is visible
     * @return True if the amplitude scale is visible
     * 
     * Returns whether the vertical amplitude grid lines are currently visible.
     * This corresponds to the state set by SetAmplitudeScaleVisible().
     */
    virtual bool isAmplitudeScaleVisible() const = 0;

    /**
     * @brief Get the current grid color
     * @return Current grid color
     * 
     * Returns the color currently used for the grid lines.
     * This corresponds to the color set by SetGridColor().
     */
    virtual QColor GetGridColor() const = 0;

    /**
     * @brief Get the current background color
     * @return Current background color
     * 
     * Returns the background color currently used for the waveform display area.
     * This corresponds to the color set by SetBackgroundColor().
     */
    virtual QColor GetBackgroundColor() const = 0;
};

#endif // I_WAVEFORM_VIEW_H 