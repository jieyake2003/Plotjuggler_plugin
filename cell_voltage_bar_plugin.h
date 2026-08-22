#ifndef CELL_VOLTAGE_BAR_PLUGIN_H
#define CELL_VOLTAGE_BAR_PLUGIN_H

#include <PlotJuggler/statepublisher_base.h>
#include <PlotJuggler/plotdata.h>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>

// Forward declarations for Qwt
class QwtPlot;
class QwtPlotBarChart;
class QwtPlotGrid;
class QwtColumnSymbol;

// ============================================================
// Bar chart widget: draws 21 bars for Cell 0~20
// ============================================================
class CellVoltageBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CellVoltageBarWidget(QWidget* parent = nullptr);

    void setVoltageValues(const QVector<double>& values, double trackerTime);
    void setYRange(double ymin, double ymax);
    void setAutoRange(bool enabled);
    bool autoRange() const { return m_autoRange; }

    double yMin() const { return m_yMin; }
    double yMax() const { return m_yMax; }

signals:
    void yRangeChanged(double ymin, double ymax);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawBars(QPainter& painter, const QRect& rect);
    QColor valueToColor(double value) const;

    QVector<double> m_values;   // 21 voltage values
    double m_trackerTime;
    double m_yMin;
    double m_yMax;
    bool   m_autoRange;

public:
    static constexpr int CELL_COUNT = 21;
    static constexpr double DEFAULT_Y_MIN = 2.5;
    static constexpr double DEFAULT_Y_MAX = 4.2;
    static constexpr double Y_STEP = 0.1;
};

// ============================================================
// PlotJuggler StatePublisher plugin
// ============================================================
class CellVoltageBarPlugin : public PJ::StatePublisher
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.StatePublisher" FILE "cell_voltage_bar_plugin.json")
    Q_INTERFACES(PJ::StatePublisher)

public:
    CellVoltageBarPlugin();
    virtual ~CellVoltageBarPlugin() override;

    // StatePublisher interface
    const char* name() const override { return "Cell Voltage Bar Chart"; }
    bool enabled() const override;
    QWidget* embeddedWidget() override;
    void updateState(double current_time) override;
    void play(double interval) override;

public slots:
    void setEnabled(bool enabled) override;

private slots:
    void onMinValueChanged(double val);
    void onMaxValueChanged(double val);
    void onAutoRangeToggled(bool checked);
    void detectCellChannels();

private:
    struct CellChannel {
        int     index;
        QString name;
        const PJ::PlotData* data;
        bool    found;
        CellChannel() : index(-1), data(nullptr), found(false) {}
    };

    double interpolateValue(const PJ::PlotData* data, double x) const;

    QWidget*             m_widget;
    CellVoltageBarWidget* m_barWidget;
    QDoubleSpinBox*      m_minSpin;
    QDoubleSpinBox*      m_maxSpin;
    QCheckBox*           m_autoRangeCheck;
    QLabel*              m_infoLabel;
    QLabel*              m_timeLabel;

    CellChannel m_channels[CellVoltageBarWidget::CELL_COUNT];
    bool m_enabled;
    bool m_channelsDetected;
};

#endif // CELL_VOLTAGE_BAR_PLUGIN_H
