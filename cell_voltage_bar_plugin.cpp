#include "cell_voltage_bar_plugin.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>
#include <limits>
#include <QDebug>

// ============================================================
// CellVoltageBarWidget
// ============================================================

CellVoltageBarWidget::CellVoltageBarWidget(QWidget* parent)
    : QWidget(parent)
    , m_trackerTime(0)
    , m_yMin(2.5)
    , m_yMax(4.2)
    , m_autoRange(false)
    , m_values(CELL_COUNT, 0.0)
{
    setMinimumHeight(220);
    setMinimumWidth(400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(250, 250, 252));
    setPalette(pal);
}

void CellVoltageBarWidget::setVoltageValues(const QVector<double>& values, double trackerTime)
{
    m_values = values;
    m_trackerTime = trackerTime;

    // Auto range if enabled
    if (m_autoRange) {
        double dmin = std::numeric_limits<double>::max();
        double dmax = std::numeric_limits<double>::lowest();
        bool hasData = false;
        for (double v : m_values) {
            if (!std::isnan(v) && !std::isinf(v) && v != 0.0) {
                dmin = std::min(dmin, v);
                dmax = std::max(dmax, v);
                hasData = true;
            }
        }
        if (hasData) {
            double margin = (dmax - dmin) * 0.15;
            if (margin < 0.1) margin = 0.5;
            m_yMin = std::floor((dmin - margin) * 10.0) / 10.0;
            m_yMax = std::ceil((dmax + margin) * 10.0) / 10.0;
            emit yRangeChanged(m_yMin, m_yMax);
        }
    }

    update();
}

void CellVoltageBarWidget::setYRange(double ymin, double ymax)
{
    if (ymax > ymin) {
        m_yMin = ymin;
        m_yMax = ymax;
        update();
    }
}

void CellVoltageBarWidget::setAutoRange(bool enabled)
{
    m_autoRange = enabled;
}

QColor CellVoltageBarWidget::valueToColor(double value) const
{
    if (std::isnan(value) || std::isinf(value) || value == 0.0) {
        return QColor(180, 180, 180);  // Gray for no data
    }

    double range = m_yMax - m_yMin;
    if (range < 1e-6) range = 1.0;
    double t = (value - m_yMin) / range;
    t = qBound(0.0, t, 1.0);

    // Color gradient: Red(low) -> Orange -> Green(normal) -> Blue -> Purple(high)
    int r, g, b;
    if (t < 0.25) {
        double s = t / 0.25;
        r = 220 + (int)(35 * s);   // 220 -> 255
        g = (int)(80 * s);         // 0 -> 80
        b = 50;
    } else if (t < 0.5) {
        double s = (t - 0.25) / 0.25;
        r = 255 - (int)(205 * s);  // 255 -> 50
        g = 80 + (int)(120 * s);   // 80 -> 200
        b = 50;
    } else if (t < 0.75) {
        double s = (t - 0.5) / 0.25;
        r = 50;
        g = 200 - (int)(20 * s);   // 200 -> 180
        b = 50 + (int)(205 * s);   // 50 -> 255
    } else {
        double s = (t - 0.75) / 0.25;
        r = 50 + (int)(130 * s);   // 50 -> 180
        g = 180 - (int)(130 * s);  // 180 -> 50
        b = 255 - (int)(35 * s);   // 255 -> 220
    }

    return QColor(r, g, b);
}

void CellVoltageBarWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect().adjusted(8, 8, -8, -8);

    // Title
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(r, Qt::AlignTop | Qt::AlignHCenter, "Cell Voltage (Cell 0~20)");
    r.adjust(0, 22, 0, 0);

    // Time label
    painter.setFont(QFont("Arial", 8));
    painter.setPen(QColor(100, 100, 100));
    painter.drawText(r, Qt::AlignTop | Qt::AlignRight,
                     QString("t = %1 s").arg(m_trackerTime, 0, 'f', 3));
    r.adjust(0, 16, 0, 0);

    drawBars(painter, r);
}

void CellVoltageBarWidget::drawBars(QPainter& painter, const QRect& rect)
{
    // Reserve space for Y axis labels
    const int yLabelWidth = 50;
    const int bottomMargin = 25;
    const int rightMargin = 10;

    QRect chartRect = rect.adjusted(yLabelWidth, 0, -rightMargin, -bottomMargin);
    if (chartRect.width() < 50 || chartRect.height() < 50) return;

    // Draw Y axis grid and labels
    painter.setFont(QFont("Arial", 8));
    painter.setPen(QColor(180, 180, 180));

    int yTicks = 6;
    for (int i = 0; i <= yTicks; i++) {
        double val = m_yMin + (m_yMax - m_yMin) * i / yTicks;
        int y = chartRect.bottom() - (int)(chartRect.height() * i / (double)yTicks);

        // Grid line
        painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
        painter.drawLine(chartRect.left(), y, chartRect.right(), y);

        // Label
        painter.setPen(QColor(80, 80, 80));
        painter.drawText(QRect(0, y - 8, yLabelWidth - 4, 16),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(val, 'f', 2));
    }

    // Draw Y axis label
    painter.save();
    painter.translate(rect.left() + 8, chartRect.center().y());
    painter.rotate(-90);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(QRect(-40, -20, 80, 40), Qt::AlignCenter, "Voltage (V)");
    painter.restore();

    // Draw bars
    double barWidth = chartRect.width() / (double)CELL_COUNT;
    double yRange = m_yMax - m_yMin;
    if (yRange < 1e-9) yRange = 1.0;

    for (int i = 0; i < CELL_COUNT; i++) {
        double val = m_values[i];
        int x = chartRect.left() + (int)(i * barWidth);
        int barW = (int)(barWidth * 0.8);
        int barX = x + (int)(barWidth * 0.1);

        QColor barColor = valueToColor(val);

        if (std::isnan(val) || std::isinf(val) || val == 0.0) {
            // No data - draw empty bar
            painter.setPen(QPen(QColor(200, 200, 200), 1));
            painter.setBrush(QBrush(QColor(240, 240, 240)));
            int barH = chartRect.height() / 3;
            int barY = chartRect.bottom() - barH;
            painter.drawRect(barX, barY, barW, barH);
        } else {
            // Calculate bar height
            double normalized = (val - m_yMin) / yRange;
            normalized = qBound(0.0, normalized, 1.0);
            int barH = (int)(normalized * chartRect.height());
            if (barH < 2) barH = 2;
            int barY = chartRect.bottom() - barH;

            // Draw bar with gradient
            QLinearGradient grad(barX, barY, barX, barY + barH);
            grad.setColorAt(0.0, barColor.lighter(130));
            grad.setColorAt(1.0, barColor);
            painter.setPen(QPen(barColor.darker(120), 1));
            painter.setBrush(QBrush(grad));
            painter.drawRect(barX, barY, barW, barH);

            // Draw value text on top of bar
            if (barH > 20 && barW > 25) {
                painter.setPen(Qt::black);
                painter.setFont(QFont("Arial", 7));
                painter.drawText(QRect(barX, barY - 2, barW, 14),
                                 Qt::AlignCenter | Qt::AlignBottom,
                                 QString::number(val, 'f', 3));
            }
        }

        // X axis label
        painter.setPen(QColor(80, 80, 80));
        painter.setFont(QFont("Arial", 7));
        painter.drawText(QRect(x, chartRect.bottom() + 2, (int)barWidth, bottomMargin - 2),
                         Qt::AlignCenter | Qt::AlignTop,
                         QString::number(i));
    }

    // X axis label
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(QRect(chartRect.left(), chartRect.bottom() + bottomMargin - 16,
                           chartRect.width(), 14),
                     Qt::AlignCenter, "Cell Index");

    // Draw Y axis line
    painter.setPen(QPen(Qt::black, 1));
    painter.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

    // Draw baseline
    painter.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
}

// ============================================================
// CellVoltageBarPlugin (StatePublisher)
// ============================================================

CellVoltageBarPlugin::CellVoltageBarPlugin()
    : StatePublisher()
    , m_widget(nullptr)
    , m_barWidget(nullptr)
    , m_minSpin(nullptr)
    , m_maxSpin(nullptr)
    , m_autoRangeCheck(nullptr)
    , m_infoLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_enabled(false)
    , m_channelsDetected(false)
{
    for (int i = 0; i < CellVoltageBarWidget::CELL_COUNT; i++) {
        m_channels[i].index = i;
    }
}

CellVoltageBarPlugin::~CellVoltageBarPlugin()
{
}

bool CellVoltageBarPlugin::enabled() const
{
    return m_enabled;
}

void CellVoltageBarPlugin::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

QWidget* CellVoltageBarPlugin::embeddedWidget()
{
    if (!m_widget) {
        // Build UI
        m_widget = new QWidget();
        m_widget->setWindowTitle("Cell Voltage Bar Chart (Cell 0~20)");
        m_widget->setMinimumSize(500, 320);

        QVBoxLayout* mainLayout = new QVBoxLayout(m_widget);
        mainLayout->setContentsMargins(4, 4, 4, 4);
        mainLayout->setSpacing(4);

        // Info label
        m_infoLabel = new QLabel("Cell Voltage Bar Chart — Load data to detect channels", m_widget);
        m_infoLabel->setStyleSheet("QLabel { color: #666; font-size: 11px; padding: 2px; }");
        mainLayout->addWidget(m_infoLabel);

        // Bar chart widget
        m_barWidget = new CellVoltageBarWidget(m_widget);
        mainLayout->addWidget(m_barWidget, 1);

        // Connect auto-range signal
        connect(m_barWidget, &CellVoltageBarWidget::yRangeChanged,
                [this](double ymin, double ymax) {
                    if (m_autoRangeCheck && m_autoRangeCheck->isChecked()) {
                        m_minSpin->blockSignals(true);
                        m_maxSpin->blockSignals(true);
                        m_minSpin->setValue(ymin);
                        m_maxSpin->setValue(ymax);
                        m_minSpin->blockSignals(false);
                        m_maxSpin->blockSignals(false);
                    }
                });

        // Bottom control panel
        QWidget* controlPanel = new QWidget(m_widget);
        QHBoxLayout* ctrlLayout = new QHBoxLayout(controlPanel);
        ctrlLayout->setContentsMargins(4, 2, 4, 2);
        ctrlLayout->setSpacing(8);

        ctrlLayout->addWidget(new QLabel("Y Min:", controlPanel));
        m_minSpin = new QDoubleSpinBox(controlPanel);
        m_minSpin->setRange(-100.0, 100.0);
        m_minSpin->setValue(CellVoltageBarWidget::DEFAULT_Y_MIN);
        m_minSpin->setSingleStep(CellVoltageBarWidget::Y_STEP);
        m_minSpin->setDecimals(2);
        m_minSpin->setSuffix(" V");
        m_minSpin->setToolTip("Y-axis minimum voltage");
        ctrlLayout->addWidget(m_minSpin);

        ctrlLayout->addWidget(new QLabel("Y Max:", controlPanel));
        m_maxSpin = new QDoubleSpinBox(controlPanel);
        m_maxSpin->setRange(-100.0, 100.0);
        m_maxSpin->setValue(CellVoltageBarWidget::DEFAULT_Y_MAX);
        m_maxSpin->setSingleStep(CellVoltageBarWidget::Y_STEP);
        m_maxSpin->setDecimals(2);
        m_maxSpin->setSuffix(" V");
        m_maxSpin->setToolTip("Y-axis maximum voltage");
        ctrlLayout->addWidget(m_maxSpin);

        m_autoRangeCheck = new QCheckBox("Auto Range", controlPanel);
        m_autoRangeCheck->setChecked(false);
        m_autoRangeCheck->setToolTip("Automatically adjust Y-axis based on data range");
        ctrlLayout->addWidget(m_autoRangeCheck);

        m_timeLabel = new QLabel("Time: ---", controlPanel);
        m_timeLabel->setStyleSheet("QLabel { color: #888; font-size: 11px; }");
        ctrlLayout->addStretch();
        ctrlLayout->addWidget(m_timeLabel);

        mainLayout->addWidget(controlPanel);

        // Connect signals
        connect(m_minSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &CellVoltageBarPlugin::onMinValueChanged);
        connect(m_maxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &CellVoltageBarPlugin::onMaxValueChanged);
        connect(m_autoRangeCheck, &QCheckBox::toggled,
                this, &CellVoltageBarPlugin::onAutoRangeToggled);
    }

    return m_widget;
}

void CellVoltageBarPlugin::updateState(double current_time)
{
    if (!m_enabled || !_datamap) return;

    // Lazy detection of cell channels
    if (!m_channelsDetected) {
        detectCellChannels();
    }

    // Read voltage values at current_time
    QVector<double> values(CellVoltageBarWidget::CELL_COUNT, 0.0);
    for (int i = 0; i < CellVoltageBarWidget::CELL_COUNT; i++) {
        if (m_channels[i].found && m_channels[i].data) {
            double val = interpolateValue(m_channels[i].data, current_time);
            values[i] = val;
        }
    }

    // Update bar widget
    if (m_barWidget) {
        m_barWidget->setVoltageValues(values, current_time);
    }

    // Update time label
    if (m_timeLabel) {
        m_timeLabel->setText(QString("t = %1 s").arg(current_time, 0, 'f', 3));
    }
}

void CellVoltageBarPlugin::play(double interval)
{
    Q_UNUSED(interval);
    // updateState is called separately by PlotJuggler
}

void CellVoltageBarPlugin::onMinValueChanged(double val)
{
    if (val >= m_maxSpin->value()) {
        m_minSpin->blockSignals(true);
        m_minSpin->setValue(m_maxSpin->value() - CellVoltageBarWidget::Y_STEP);
        m_minSpin->blockSignals(false);
        val = m_minSpin->value();
    }
    if (m_barWidget) {
        m_barWidget->setYRange(val, m_maxSpin->value());
    }
}

void CellVoltageBarPlugin::onMaxValueChanged(double val)
{
    if (val <= m_minSpin->value()) {
        m_maxSpin->blockSignals(true);
        m_maxSpin->setValue(m_minSpin->value() + CellVoltageBarWidget::Y_STEP);
        m_maxSpin->blockSignals(false);
        val = m_maxSpin->value();
    }
    if (m_barWidget) {
        m_barWidget->setYRange(m_minSpin->value(), val);
    }
}

void CellVoltageBarPlugin::onAutoRangeToggled(bool checked)
{
    m_minSpin->setEnabled(!checked);
    m_maxSpin->setEnabled(!checked);
    if (m_barWidget) {
        m_barWidget->setAutoRange(checked);
    }
}

void CellVoltageBarPlugin::detectCellChannels()
{
    if (!_datamap) return;

    // Reset
    for (int i = 0; i < CellVoltageBarWidget::CELL_COUNT; i++) {
        m_channels[i].found = false;
        m_channels[i].data  = nullptr;
        m_channels[i].name  = "";
    }

    // Search in numeric data
    const auto& numericData = _datamap->numeric;

    for (const auto& pair : numericData) {
        const std::string& stdName = pair.first;
        const PJ::PlotData& plotData = pair.second;

        QString fullName = QString::fromStdString(stdName);
        QString lowerName = fullName.toLower();

        for (int cellIdx = 0; cellIdx < CellVoltageBarWidget::CELL_COUNT; cellIdx++) {
            if (m_channels[cellIdx].found) continue;

            // Patterns: cell_0, cell0, cell/0, cells_0, cell[0], etc.
            QStringList patterns;
            patterns << QString("cell_%1").arg(cellIdx)
                     << QString("cell%1").arg(cellIdx)
                     << QString("cell/%1").arg(cellIdx)
                     << QString("cells_%1").arg(cellIdx)
                     << QString("cells%1").arg(cellIdx)
                     << QString("cell[%1]").arg(cellIdx);

            for (const QString& pat : patterns) {
                int pos = lowerName.indexOf(pat);
                if (pos >= 0) {
                    // Verify no trailing digit (cell_1 should not match cell_10)
                    int afterPos = pos + pat.length();
                    if (afterPos < lowerName.length() && lowerName[afterPos].isDigit()) {
                        continue;
                    }
                    m_channels[cellIdx].found = true;
                    m_channels[cellIdx].name  = fullName;
                    m_channels[cellIdx].data  = &plotData;
                    break;
                }
            }
        }
    }

    // Update info label
    int foundCount = 0;
    QStringList missing;
    for (int i = 0; i < CellVoltageBarWidget::CELL_COUNT; i++) {
        if (m_channels[i].found) foundCount++;
        else missing << QString::number(i);
    }

    QString info = QString("Found %1/%2 channels.").arg(foundCount).arg(CellVoltageBarWidget::CELL_COUNT);
    if (foundCount < CellVoltageBarWidget::CELL_COUNT) {
        info += QString(" Missing: [%1]").arg(missing.join(", "));
    }
    if (m_infoLabel) {
        m_infoLabel->setText(info);
    }

    m_channelsDetected = true;

    qDebug() << "[CellVoltageBar]" << info;
}

double CellVoltageBarPlugin::interpolateValue(const PJ::PlotData* data, double x) const
{
    if (!data || data->size() == 0) return std::nan("");

    // Use PlotJuggler's built-in getYfromX
    auto val = data->getYfromX(x);
    if (val.has_value()) {
        return val.value();
    }
    return std::nan("");
}
