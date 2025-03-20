/**
 * @file waveform_view.h
 * @brief Defines the WaveformView class that displays physiological waveform data
 *
 * This file contains the WaveformView class which provides a graphical representation
 * of physiological waveform data such as ECG, respiration, and other vital signs.
 * It implements the IWaveformView interface and provides functionality for displaying
 * real-time scrolling waveforms with configurable visual properties.
 */
#ifndef WAVEFORM_VIEW_H
#define WAVEFORM_VIEW_H

#include <QWidget>
#include <QPainterPath>
#include <QTimer>
#include <QMutex>
#include <QColor>
#include <memory>

#include <i_waveform_model.h>
#include <i_waveform_view.h>

/**
 * @brief Demo data arrays for ECG waveform simulation
 * 
 * Predefined sample points representing a typical ECG waveform pattern
 * used for demo/simulation mode.
 */
constexpr static const float ecgDemoData[995] = {
    -100 ,-109 ,-100 ,-85 ,-75 ,-77 ,-87 ,-80 ,-78 ,4 ,109 ,169 ,180 ,107 ,17 ,-78 ,-77 ,-82 ,-60 ,-56 ,-60 ,-54 ,-217 ,250 ,1696 ,1632 ,125 ,-346 ,-213 ,-219 ,-206 ,-192 ,-171 ,-152 ,-123 ,-64 ,-30 ,-14 ,5 ,32 ,76 ,113 ,189 ,219 ,202 ,149 ,48 ,-130 ,-222 ,-211 ,-191 ,-187 ,-197 ,-186 ,-149 ,-153 ,-148 ,-144 ,-122 ,-109 ,-100 ,-104 ,-108 ,-79 ,-73 ,-73 ,-73 ,-78 ,-58 ,5 ,108 ,172 ,191 ,129 ,20 ,-82 ,-90 ,-58 ,-47 ,-53 ,-68 ,-55 ,-206 ,251 ,1684 ,1619 ,124 ,-347 ,-205 ,-230 ,-208 ,-186 ,-160 ,-137 ,-108 ,-73 ,-25 ,-12 ,-16 ,35 ,76 ,134 ,163 ,222 ,208 ,142 ,26 ,-137 ,-202 ,-201 ,-199 ,-196 ,-187 ,-172 ,-158 ,-145 ,-149 ,-136 ,-125 ,-122 ,-130 ,-120 ,-112 ,-75 ,-74 ,-84 ,-79 ,-79 ,-56 ,9 ,104 ,171 ,185 ,112 ,16 ,-83 ,-77 ,-68 ,-68 ,-67 ,-43 ,-52 ,-211 ,227 ,1686 ,1627 ,121 ,-359 ,-215 ,-225 ,-191 ,-196 ,-164 ,-155 ,-121 ,-76 ,-41 ,-24 ,-2 ,40 ,95 ,123 ,170 ,221 ,222 ,156 ,42 ,-135 ,-194 ,-197 ,-188 ,-214 ,-182 ,-163 ,-162 ,-150 ,-159 ,-125 ,-116 ,-142 ,-116 ,-97 ,-85 ,-87 ,-91 ,-77 ,-78 ,-65 ,-55 ,-12 ,88 ,182 ,194 ,118 ,12 ,-77 ,-70 ,-58 ,-72 ,-64 ,-41 ,-45 ,-223 ,235 ,1675 ,1650 ,133 ,-355 ,-198 ,-215 ,-189 ,-184 ,-160 ,-142 ,-92 ,-70 ,-39 ,-21 ,10 ,48 ,77 ,101 ,163 ,238 ,230 ,139 ,35 ,-115 ,-205 ,-212 ,-219 ,-206 ,-183 ,-164 ,-157 ,-146 ,-162 ,-126 ,-130 ,-130 ,-117 ,-106 ,-79 ,-81 ,-94 ,-94 ,-78 ,-71 ,-68 ,-20 ,112 ,180 ,186 ,121 ,17 ,-74 ,-69 ,-63 ,-53 ,-52 ,-47 ,-59 ,-224 ,223 ,1690 ,1644 ,141 ,-349 ,-217 ,-205 ,-200 ,-203 ,-173 ,-125 ,-93 ,-62 ,-49 ,-22 ,4 ,53 ,89 ,119 ,189 ,242 ,219 ,131 ,21 ,-103 ,-199 ,-229 ,-217 ,-196 ,-179 ,-185 ,-169 ,-161 ,-150 ,-136 ,-138 ,-137 ,-117 ,-89 ,-77 ,-100 ,-89 ,-81 ,-56 ,-56 ,-75 ,-1 ,114 ,175 ,177 ,109 ,8 ,-69 ,-68 ,-99 ,-84 ,-58 ,-38 ,-50 ,-208 ,246 ,1704 ,1621 ,126 ,-362 ,-216 ,-207 ,-203 ,-210 ,-170 ,-141 ,-114 ,-65 ,-40 ,-21 ,7 ,41 ,70 ,102 ,198 ,238 ,193 ,136 ,32 ,-117 ,-193 ,-208 ,-197 ,-176 ,-166 ,-172 ,-169 ,-152 ,-146 ,-139 ,-130 ,-132 ,-116 ,-106 ,-95 ,-95 ,-79 ,-65 ,-66 ,-82 ,-71 ,-4 ,121 ,164 ,165 ,120 ,16 ,-71 ,-81 ,-77 ,-62 ,-41 ,-43 ,-73 ,-219 ,236 ,1685 ,1647 ,127 ,-362 ,-218 ,-210 ,-204 ,-207 ,-160 ,-132 ,-120 ,-67 ,-42 ,-10 ,-1 ,40 ,64 ,123 ,182 ,219 ,215 ,131 ,42 ,-109 ,-211 ,-215 ,-200 ,-190 ,-185 ,-182 ,-170 ,-142 ,-143 ,-139 ,-134 ,-121 ,-111 ,-95 ,-108 ,-103 ,-81 ,-78 ,-63 ,-95 ,-77 ,-16 ,119 ,189 ,178 ,122 ,19 ,-58 ,-86 ,-75 ,-62 ,-53 ,-36 ,-69 ,-214 ,238 ,1695 ,1625 ,133
};

/**
 * @brief Demo data arrays for SpO2 (plethysmograph) waveform simulation
 * 
 * Predefined sample points representing a typical SpO2 waveform pattern
 * used for demo/simulation mode.
 */
constexpr static const float spo2DemoData[995] = {
    2096 ,2096 ,2096 ,2112 ,2112 ,2112 ,2080 ,2048 ,2016 ,1984 ,1936 ,1888 ,1840 ,1792 ,1760 ,1712 ,1664 ,1632 ,1600 ,1552 ,1520 ,1472 ,1440 ,1408 ,1376 ,1344 ,1312 ,1280 ,1264 ,1232 ,1216 ,1216 ,1248 ,1296 ,1408 ,1408 ,1552 ,1728 ,1936 ,2160 ,2400 ,2624 ,2832 ,3008 ,3136 ,3232 ,3264 ,3264 ,3200 ,3120 ,3008 ,2880 ,2736 ,2592 ,2464 ,2352 ,2240 ,2176 ,2128 ,2096 ,2096 ,2096 ,2112 ,2112 ,2112 ,2096 ,2064 ,2032 ,2000 ,1952 ,1904 ,1904 ,1872 ,1824 ,1776 ,1744 ,1696 ,1648 ,1600 ,1568 ,1520 ,1488 ,1456 ,1408 ,1376 ,1360 ,1328 ,1296 ,1264 ,1248 ,1232 ,1216 ,1232 ,1280 ,1376 ,1504 ,1664 ,1872 ,2080 ,2320 ,2560 ,2768 ,2960 ,3104 ,3200 ,3264 ,3264 ,3232 ,3232 ,3152 ,3056 ,2928 ,2800 ,2656 ,2512 ,2384 ,2272 ,2192 ,2128 ,2096 ,2096 ,2096 ,2112 ,2112 ,2112 ,2096 ,2080 ,2048 ,2000 ,1968 ,1920 ,1872 ,1824 ,1792 ,1744 ,1696 ,1664 ,1616 ,1584 ,1536 ,1504 ,1456 ,1424 ,1392 ,1360 ,1360 ,1328 ,1312 ,1280 ,1248 ,1232 ,1216 ,1216 ,1248 ,1328 ,1440 ,1584 ,1776 ,2000 ,2224 ,2464 ,2688 ,2896 ,3056 ,3168 ,3248 ,3280 ,3248 ,3184 ,3184 ,2976 ,2848 ,2704 ,2560 ,2432 ,2320 ,2224 ,2160 ,2112 ,2096 ,2096 ,2096 ,2096 ,2112 ,2112 ,2112 ,2080 ,2048 ,2032 ,1984 ,1936 ,1888 ,1856 ,1808 ,1760 ,1712 ,1680 ,1632 ,1600 ,1552 ,1520 ,1488 ,1440 ,1408 ,1376 ,1360 ,1328 ,1296 ,1264 ,1232 ,1216 ,1216 ,1232 ,1296 ,1392 ,1536 ,1712 ,1920 ,1920 ,2144 ,2384 ,2624 ,2816 ,2992 ,3136 ,3232 ,3264 ,3264 ,3216 ,3136 ,3024 ,2896 ,2752 ,2608 ,2480 ,2352 ,2256 ,2176 ,2128 ,2096 ,2080 ,2096 ,2112 ,2112 ,2112 ,2096 ,2064 ,2032 ,2000 ,1952 ,1904 ,1856 ,1824 ,1776 ,1776 ,1728 ,1696 ,1648 ,1616 ,1568 ,1536 ,1488 ,1456 ,1424 ,1392 ,1360 ,1328 ,1296 ,1264 ,1248 ,1216 ,1216 ,1232 ,1264 ,1264 ,1488 ,1648 ,1840 ,2064 ,2304 ,2528 ,2752 ,2944 ,3088 ,3200 ,3264 ,3280 ,3232 ,3168 ,3168 ,2944 ,2944 ,2816 ,2672 ,2528 ,2400 ,2288 ,2192 ,2128 ,2096 ,2096 ,2096 ,2112 ,2128 ,2112 ,2096 ,2080 ,2048 ,2016 ,1968 ,1920 ,1888 ,1840 ,1792 ,1744 ,1712 ,1664 ,1632 ,1584 ,1552 ,1504 ,1472 ,1440 ,1392 ,1376 ,1328 ,1312 ,1312 ,1280 ,1248 ,1232 ,1216 ,1216 ,1248 ,1328 ,1424 ,1584 ,1760 ,1984 ,2208 ,2432 ,2672 ,2864 ,3040 ,3152 ,3232 ,3264 ,3248 ,3200 ,3200 ,2992 ,2864 ,2720 ,2720 ,2432 ,2320 ,2224 ,2160 ,2112 ,2096 ,2096 ,2096 ,2112 ,2112 ,2112 ,2112 ,2096 ,2064 ,2032 ,1984 ,1936 ,1904 ,1856 ,1808 ,1760 ,1728 ,1680 ,1632 ,1600 ,1552 ,1520 ,1488 ,1456 ,1408 ,1376 ,1360 ,1328 ,1296 ,1264 ,1232 ,1216 ,1216 ,1248 ,1296 ,1392 ,1536 ,1696 ,1904
};

/**
 * @brief Demo data arrays for IBP (Invasive Blood Pressure) waveform simulation
 * 
 * Predefined sample points representing a typical phasic IBP waveform pattern
 * used for demo/simulation mode.
 */
constexpr static const float ibp1PhasicDemoData[995] = {
    5859 ,5767 ,5691 ,5616 ,5543 ,5469 ,5410 ,5360 ,5313 ,5274 ,5239 ,5203 ,5166 ,5132 ,5100 ,5074 ,5054 ,5040 ,5026 ,5007 ,4991 ,4983 ,4981 ,4980 ,4979 ,4980 ,4980 ,4981 ,5000 ,5167 ,5500 ,5918 ,6326 ,6672 ,6972 ,7213 ,7370 ,7455 ,7485 ,7485 ,7467 ,7429 ,7351 ,7245 ,7120 ,6968 ,6808 ,6684 ,6613 ,6604 ,6659 ,6726 ,6759 ,6737 ,6649 ,6512 ,6352 ,6205 ,6076 ,5961 ,5857 ,5767 ,5691 ,5618 ,5541 ,5464 ,5406 ,5358 ,5312 ,5276 ,5241 ,5205 ,5168 ,5134 ,5103 ,5078 ,5059 ,5043 ,5027 ,5006 ,4988 ,4979 ,4979 ,4980 ,4980 ,4979 ,4979 ,4979 ,5009 ,5182 ,5509 ,5911 ,6315 ,6667 ,6973 ,7213 ,7366 ,7453 ,7484 ,7486 ,7469 ,7430 ,7351 ,7247 ,7124 ,6969 ,6809 ,6685 ,6614 ,6603 ,6658 ,6727 ,6761 ,6739 ,6650 ,6512 ,6351 ,6204 ,6075 ,5960 ,5852 ,5763 ,5689 ,5617 ,5543 ,5468 ,5409 ,5360 ,5313 ,5276 ,5240 ,5204 ,5167 ,5133 ,5101 ,5077 ,5058 ,5043 ,5025
};

/**
 * @brief Demo data arrays for respiration waveform simulation
 * 
 * Predefined sample points representing a typical respiration waveform pattern
 * used for demo/simulation mode.
 */
constexpr static const float respSimulate[995] = {
    -6736 ,-6768 ,-6752 ,-6704 ,-6592 ,-6448 ,-6320 ,-6208 ,-6128 ,-6064 ,-5984 ,-5856 ,-5696 ,-5520 ,-5392 ,-5280 ,-5216 ,-5152 ,-5104 ,-5040 ,-4960 ,-4864 ,-4768 ,-4672 ,-4592 ,-4496 ,-4416 ,-4288 ,-4128 ,-3984 ,-3856 ,-3744 ,-3600 ,-3472 ,-3328 ,-3152 ,-2944 ,-2736 ,-2544 ,-2384 ,-2240 ,-2096 ,-1920 ,-1664 ,-1392 ,-1120 ,-864 ,-624 ,-400 ,-192 ,-16 ,128 ,256 ,400 ,544 ,688 ,864 ,1056 ,1264 ,1472 ,1680 ,1904 ,2096 ,2304 ,2496 ,2656 ,2800 ,2912 ,2976 ,3024 ,3104 ,3216 ,3408 ,3632 ,3872 ,4128 ,4336 ,4528 ,4688 ,4816 ,4960 ,5088 ,5200 ,5296 ,5360 ,5408 ,5456 ,5488 ,5600 ,5712 ,5824 ,5920 ,5952 ,5968 ,5984 ,6016 ,6080 ,6128 ,6176 ,6208 ,6224 ,6208 ,6208 ,6240 ,6304 ,6384 ,6432 ,6464 ,6448 ,6400 ,6368 ,6304 ,6288 ,6272 ,6256 ,6240 ,6176 ,6128 ,6096 ,6096 ,6192 ,6272 ,6320 ,6320 ,6224 ,6096 ,5952 ,5792 ,5664 ,5584 ,5520 ,5456 ,5392 ,5344 ,5280 ,5248 ,5216 ,5168 ,5120 ,5040 ,4976 ,4912 ,4832 ,4768 ,4720 ,4672 ,4624 ,4592 ,4512 ,4432 ,4352 ,4272 ,4176 ,4048 ,3888 ,3696 ,3472 ,3232 ,2976 ,2688 ,2432 ,2160 ,1872 ,1600 ,1328 ,1120 ,960 ,816 ,656 ,496 ,336 ,160 ,0 ,-128 ,-256 ,-384 ,-496 ,-640 ,-768 ,-864 ,-960 ,-1024 ,-1104 ,-1216 ,-1328 ,-1520 ,-1776 ,-2064 ,-2352 ,-2592 ,-2800 ,-3024 ,-3248 ,-3472 ,-3680 ,-3872 ,-4080 ,-4240 ,-4416 ,-4592 ,-4768 ,-4960 ,-5152 ,-5328 ,-5488 ,-5600 ,-5696 ,-5792 ,-5872 ,-5968 ,-6064 ,-6128 ,-6208 ,-6256 ,-6272 ,-6288 ,-6288 ,-6320 ,-6352 ,-6368 ,-6416 ,-6432 ,-6432 ,-6480 ,-6544 ,-6640 ,-6720 ,-6752 ,-6752 ,-6704 ,-6672 ,-6688 ,-6688 ,-6704 ,-6736 ,-6784 ,-6832 ,-6784 ,-6720 ,-6656 ,-6592 ,-6576 ,-6560 ,-6528 ,-6464 ,-6368 ,-6272 ,-6208 ,-6160 ,-6160 ,-6176 ,-6192 ,-6160 ,-6048 ,-5920 ,-5792 ,-5680 ,-5568 ,-5456 ,-5312 ,-5152 ,-4960 ,-4800 ,-4688 ,-4608 ,-4592 ,-4608 ,-4608 ,-4576 ,-4496 ,-4416 ,-4304 ,-4144 ,-3968 ,-3776 ,-3552 ,-3296 ,-3008 ,-2720 ,-2496 ,-2304 ,-2128 ,-1920 ,-1680 ,-1456 ,-1232 ,-1008 ,-816 ,-624 ,-448 ,-272 ,-96 ,64
};

/**
 * @class WaveformView
 * @brief Widget for displaying physiological waveform data
 *
 * The WaveformView class provides a Qt widget that displays real-time physiological waveform data
 * such as ECG, respiration, SpO2 plethysmograph, and others. It shows a scrolling waveform display
 * with configurable visual properties such as sweep speed, grid visibility, colors, and scales.
 * 
 * The class implements the IWaveformView interface and uses a model-view architecture where
 * an IWaveformModel provides the data to be displayed. It supports both live data from actual
 * monitoring systems and simulated demo data for various physiological parameters.
 */
class WaveformView : public QWidget, public IWaveformView
{
    Q_OBJECT
    
public:
    /**
     * @brief Constructor
     * @param parent The parent widget
     */
    explicit WaveformView(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~WaveformView() override;
    
    /**
     * @brief Get the widget for this view
     * @return Pointer to the widget
     */
    QWidget* GetWidget() override { return this; }
    
    /**
     * @brief Sets the waveform model for this view
     * @param model The waveform model
     */
    void SetModel(std::shared_ptr<IWaveformModel> model) override;
    
    /**
     * @brief Gets the current waveform model
     * @return The waveform model
     */
    std::shared_ptr<IWaveformModel> GetModel() const override;
    
    /**
     * @brief Sets the sweep speed in pixels per second
     * @param pixelsPerSecond The sweep speed
     */
    void SetSweepSpeed(double pixelsPerSecond) override;
    
    /**
     * @brief Gets the current sweep speed
     * @return The sweep speed in pixels per second
     */
    double GetSweepSpeed() const override;
    
    /**
     * @brief Sets whether the grid is visible
     * @param visible True if the grid should be visible, false otherwise
     */
    void SetGridVisible(bool visible) override;
    
    /**
     * @brief Gets whether the grid is visible
     * @return True if the grid is visible
     */
    bool isGridVisible() const override;
    
    /**
     * @brief Sets the grid color
     * @param color The grid color
     */
    void SetGridColor(const QColor& color) override;
    
    /**
     * @brief Gets the grid color
     * @return The grid color
     */
    QColor GetGridColor() const override;
    
    /**
     * @brief Sets whether the time scale is visible
     * @param visible True if the time scale should be visible, false otherwise
     */
    void SetTimeScaleVisible(bool visible) override;
    
    /**
     * @brief Gets whether the time scale is visible
     * @return True if the time scale is visible
     */
    bool isTimeScaleVisible() const override;
    
    /**
     * @brief Sets whether the amplitude scale is visible
     * @param visible True if the amplitude scale should be visible, false otherwise
     */
    void SetAmplitudeScaleVisible(bool visible) override;
    
    /**
     * @brief Gets whether the amplitude scale is visible
     * @return True if the amplitude scale is visible
     */
    bool isAmplitudeScaleVisible() const override;
    
    /**
     * @brief Sets the background color
     * @param color The background color
     */
    void SetBackgroundColor(const QColor& color) override;
    
    /**
     * @brief Gets the background color
     * @return The background color
     */
    QColor GetBackgroundColor() const override;
    
    /**
     * @brief Triggers a widget update
     */
    void update() override;
    
    /**
     * @brief Sets whether the waveform display is paused
     * @param paused True to pause, false to resume
     */
    void SetPaused(bool paused) override;
    
    /**
     * @brief Gets whether the waveform display is paused
     * @return True if paused, false otherwise
     */
    bool isPaused() const override;
    
protected:
    /**
     * @brief Paint event handler
     * @param event The paint event
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * @brief Resize event handler
     * @param event The resize event
     */
    void resizeEvent(QResizeEvent* event) override;
    
private slots:
    /**
     * @brief Handles data updated signals from the model
     */
    void HandleDataUpdated();
    
    /**
     * @brief Handles property changed signals from the model
     */
    void HandlePropertiesChanged();
    
    /**
     * @brief Updates the display
     */
    void UpdateDisplay();
    
private:
    /**
     * @brief Connects model signals
     */
    void connectModelSignals();
    
    /**
     * @brief Disconnects model signals
     */
    void disconnectModelSignals();
    
    /**
     * @brief Draws the grid
     * @param painter The painter to use
     */
    void drawGrid(QPainter& painter);
    
    /**
     * @brief Draws the waveform
     * @param painter The painter to use
     */
    void drawWaveform(QPainter& painter);
    
    /**
     * @brief Draws the labels
     * @param painter The painter to use
     */
    void drawLabels(QPainter& painter);
    
    /**
     * @brief Processes demo waveform data based on waveform type
     * @param waveformId The waveform ID
     * @return Processed demo value
     */
    float ProcessDemoData(int waveformId);
    
    // Member variables
    std::shared_ptr<IWaveformModel> model_; /**< The waveform data model */
    QTimer* display_timer_; /**< Timer for updating the display */
    mutable QMutex mutex_; /**< Mutex for thread safety */
    
    // Path variables
    QPainterPath waveform_path_; /**< Path for drawing the waveform */
    QPointF draw_starting_point_; /**< Starting point for drawing */
    double axis_x_; /**< Current X position for drawing */
    
    // Demo data variables
    int wave_form_data_counter_; /**< Counter for cycling through demo data */
    
    // Drawing settings
    double sweep_speed_; /**< Sweep speed in pixels per second */
    bool grid_visible_; /**< Whether the grid is visible */
    QColor grid_color_; /**< Color of the grid */
    bool time_scale_visible_; /**< Whether the time scale is visible */
    bool amplitude_scale_visible_; /**< Whether the amplitude scale is visible */
    QColor background_color_; /**< Background color */
    bool is_paused_; /**< Whether the waveform is paused */
    
    // Constants
    static constexpr int UPDATE_INTERVAL_MS = 40; /**< Default update interval in milliseconds */
    static constexpr int WAVEFORM_MARGIN = 10; /**< Margin around the waveform in pixels */
    static constexpr int DEFAULT_GRID_MINOR_X = 50; /**< Minor grid spacing in X direction */
    static constexpr int DEFAULT_GRID_MINOR_Y = 50; /**< Minor grid spacing in Y direction */
    static constexpr int DEFAULT_GRID_MAJOR_X = 250; /**< Major grid spacing in X direction */
    static constexpr int DEFAULT_GRID_MAJOR_Y = 250; /**< Major grid spacing in Y direction */
};

#endif // WAVEFORM_VIEW_H 