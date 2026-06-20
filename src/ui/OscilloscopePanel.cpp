#include "OscilloscopePanel.h"
#include "../domain/components/Oscilloscope.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QPaintEvent>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

namespace {
constexpr int kDivX = 10;
constexpr int kDivY = 8;

void drawChannel(QPainter& p,
                 const std::deque<double>& time,
                 const std::deque<double>& values,
                 const QRectF& plot,
                 double now,
                 double timePerDiv,
                 double voltsPerDiv,
                 double offset,
                 const QColor& color)
{
    if (time.size() < 2 || values.size() != time.size()) return;
    const double window = std::max(1e-9, timePerDiv * kDivX);
    const double leftTime = now - window;
    const double halfVolts = std::max(1e-9, voltsPerDiv * (kDivY / 2.0));

    bool havePrev = false;
    QPointF prevPoint;
    p.setPen(QPen(color, 2));

    for (size_t i = 0; i < time.size(); ++i) {
        if (time[i] < leftTime) continue;

        const double tx = (time[i] - leftTime) / window;
        const double x = plot.left() + tx * plot.width();
        const double norm = (values[i] + offset) / halfVolts;
        const double y = std::clamp(plot.center().y() - norm * plot.height() / 2.0,
                                    plot.top(), plot.bottom());
        const QPointF currentPoint(x, y);

        if (havePrev) {
            p.drawLine(prevPoint, currentPoint);
        }

        prevPoint = currentPoint;
        havePrev = true;
    }
}
}

ScopePlotWidget::ScopePlotWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumHeight(260);
    setAutoFillBackground(false);
}

void ScopePlotWidget::setScope(std::shared_ptr<Oscilloscope> scope)
{
    m_scope = scope;
    update();
}

void ScopePlotWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(10, 18, 10));

    // IMPORTANT: the oscilloscope grid is a UI overlay, not a simulated waveform.
    // Draw it in integer screen pixels with antialiasing disabled; otherwise Qt draws
    // many grid lines on half/sub-pixels and the grid appears to shimmer or jump while
    // the scope is repainting.
    QRect plotRect = rect().adjusted(46, 20, -18, -34);
    plotRect.setWidth((plotRect.width() / kDivX) * kDivX);
    plotRect.setHeight((plotRect.height() / kDivY) * kDivY);
    const QRectF plot(plotRect);

    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(QPen(QColor(55, 105, 55), 1));
    p.setBrush(QColor(8, 16, 8));
    p.drawRect(plotRect.adjusted(0, 0, -1, -1));

    auto crispX = [](double x) { return std::floor(x) + 0.5; };
    auto crispY = [](double y) { return std::floor(y) + 0.5; };

    const int minorX = kDivX * 5;
    const int minorY = kDivY * 5;

    QPen minorPen(QColor(28, 68, 28), 1);
    QPen majorPen(QColor(65, 130, 65), 1);
    QPen axisPen(QColor(115, 220, 115), 1);
    minorPen.setCosmetic(true);
    majorPen.setCosmetic(true);
    axisPen.setCosmetic(true);

    for (int i = 0; i <= minorX; ++i) {
        const double x = crispX(plot.left() + plot.width() * i / minorX);
        p.setPen((i % 5 == 0) ? majorPen : minorPen);
        p.drawLine(QPointF(x, crispY(plot.top())), QPointF(x, crispY(plot.bottom())));
    }
    for (int i = 0; i <= minorY; ++i) {
        const double y = crispY(plot.top() + plot.height() * i / minorY);
        p.setPen((i % 5 == 0) ? majorPen : minorPen);
        p.drawLine(QPointF(crispX(plot.left()), y), QPointF(crispX(plot.right()), y));
    }

    p.setPen(axisPen);
    p.drawLine(QPointF(crispX(plot.left()), crispY(plot.center().y())),
               QPointF(crispX(plot.right()), crispY(plot.center().y())));
    p.drawLine(QPointF(crispX(plot.center().x()), crispY(plot.top())),
               QPointF(crispX(plot.center().x()), crispY(plot.bottom())));

    p.setRenderHint(QPainter::Antialiasing, true);

    auto scope = m_scope.lock();
    if (!scope) {
        p.setPen(Qt::white);
        p.drawText(plot, Qt::AlignCenter, "Select an Oscilloscope component");
        return;
    }

    const auto& t = scope->timeSamples();
    const double now = t.empty() ? 0.0 : t.back();
    if (scope->ch1Enabled())
        drawChannel(p, t, scope->ch1Samples(), plot, now, scope->timePerDiv(), scope->ch1VoltsPerDiv(), scope->ch1VerticalOffset(), QColor(0,255,80));
    if (scope->ch2Enabled())
        drawChannel(p, t, scope->ch2Samples(), plot, now, scope->timePerDiv(), scope->ch2VoltsPerDiv(), scope->ch2VerticalOffset(), QColor(255,210,0));

    p.setFont(QFont("Monospace", 8));
    p.setPen(QColor(0,255,80));
    p.drawText(QPointF(plot.left(), height() - 18),
               QString("CH1 %1 V/div off %2 V")
                    .arg(scope->ch1VoltsPerDiv(), 0, 'g', 4)
                    .arg(scope->ch1VerticalOffset(), 0, 'g', 4));
    p.setPen(QColor(255,210,0));
    p.drawText(QPointF(plot.left(), height() - 4),
               QString("CH2 %1 V/div off %2 V")
                    .arg(scope->ch2VoltsPerDiv(), 0, 'g', 4)
                    .arg(scope->ch2VerticalOffset(), 0, 'g', 4));

    p.setPen(Qt::white);
    p.drawText(QRectF(plot.left() + 210, height() - 25, plot.width() - 210, 20), Qt::AlignRight,
               QString("Time/Div: %1 s")
                    .arg(scope->timePerDiv(), 0, 'g', 4));
}

OscilloscopePanel::OscilloscopePanel(QWidget* parent) : QWidget(parent)
{
    auto* main = new QVBoxLayout(this);
    main->setContentsMargins(6,6,6,6);

    m_status = new QLabel("No oscilloscope selected", this);
    main->addWidget(m_status);

    m_plot = new ScopePlotWidget(this);
    main->addWidget(m_plot, 1);

    auto* controls = new QGroupBox("Oscilloscope Controls", this);
    auto* form = new QFormLayout(controls);

    m_ch1Box = new QCheckBox("CH1", this);
    m_ch1Box->setChecked(true);
    m_ch2Box = new QCheckBox("CH2", this);
    m_ch2Box->setChecked(true);
    auto* chBox = new QWidget(this);
    auto* chLayout = new QHBoxLayout(chBox);
    chLayout->setContentsMargins(0,0,0,0);
    chLayout->addWidget(m_ch1Box);
    chLayout->addWidget(m_ch2Box);
    chLayout->addStretch();
    form->addRow("Channels:", chBox);

    m_ch1VoltsDiv = new QDoubleSpinBox(this);
    m_ch1VoltsDiv->setRange(0.001, 1000.0);
    m_ch1VoltsDiv->setDecimals(4);
    m_ch1VoltsDiv->setValue(1.0);
    m_ch1VoltsDiv->setSuffix(" V/div");
    form->addRow("CH1 Volt/Div:", m_ch1VoltsDiv);

    m_ch2VoltsDiv = new QDoubleSpinBox(this);
    m_ch2VoltsDiv->setRange(0.001, 1000.0);
    m_ch2VoltsDiv->setDecimals(4);
    m_ch2VoltsDiv->setValue(1.0);
    m_ch2VoltsDiv->setSuffix(" V/div");
    form->addRow("CH2 Volt/Div:", m_ch2VoltsDiv);

    m_timeDiv = new QDoubleSpinBox(this);
    m_timeDiv->setRange(0.000001, 10.0);
    m_timeDiv->setDecimals(6);
    m_timeDiv->setValue(0.1);
    m_timeDiv->setSuffix(" s/div");
    form->addRow("Time/Div:", m_timeDiv);

    m_ch1Offset = new QDoubleSpinBox(this);
    m_ch1Offset->setRange(-1000.0, 1000.0);
    m_ch1Offset->setDecimals(4);
    m_ch1Offset->setValue(0.0);
    m_ch1Offset->setSuffix(" V");
    form->addRow("CH1 offset:", m_ch1Offset);

    m_ch2Offset = new QDoubleSpinBox(this);
    m_ch2Offset->setRange(-1000.0, 1000.0);
    m_ch2Offset->setDecimals(4);
    m_ch2Offset->setValue(0.0);
    m_ch2Offset->setSuffix(" V");
    form->addRow("CH2 offset:", m_ch2Offset);

    m_memoryLabel = new QLabel("Auto", this);
    m_memoryLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow("Memory (auto):", m_memoryLabel);

    m_clearButton = new QPushButton("Clear trace", this);
    form->addRow("", m_clearButton);

    main->addWidget(controls);

    auto apply = [this]() { applyControlsToScope(); };
    connect(m_ch1Box, &QCheckBox::toggled, this, apply);
    connect(m_ch2Box, &QCheckBox::toggled, this, apply);
    connect(m_ch1VoltsDiv, qOverload<double>(&QDoubleSpinBox::valueChanged), this, apply);
    connect(m_ch2VoltsDiv, qOverload<double>(&QDoubleSpinBox::valueChanged), this, apply);
    connect(m_timeDiv, qOverload<double>(&QDoubleSpinBox::valueChanged), this, apply);
    connect(m_ch1Offset, qOverload<double>(&QDoubleSpinBox::valueChanged), this, apply);
    connect(m_ch2Offset, qOverload<double>(&QDoubleSpinBox::valueChanged), this, apply);
    connect(m_clearButton, &QPushButton::clicked, this, [this](){ clearScope(); });
}

void OscilloscopePanel::setScope(std::shared_ptr<Oscilloscope> scope)
{
    m_scope = scope;
    m_plot->setScope(scope);
    updateControlsFromScope();
    refresh();
}

void OscilloscopePanel::updateControlsFromScope()
{
    auto scope = m_scope.lock();
    m_updating = true;
    const bool has = static_cast<bool>(scope);
    m_ch1Box->setEnabled(has);
    m_ch2Box->setEnabled(has);
    m_ch1VoltsDiv->setEnabled(has);
    m_ch2VoltsDiv->setEnabled(has);
    m_timeDiv->setEnabled(has);
    m_ch1Offset->setEnabled(has);
    m_ch2Offset->setEnabled(has);
    m_memoryLabel->setEnabled(has);
    m_clearButton->setEnabled(has);

    if (scope) {
        m_ch1Box->setChecked(scope->ch1Enabled());
        m_ch2Box->setChecked(scope->ch2Enabled());
        m_ch1VoltsDiv->setValue(scope->ch1VoltsPerDiv());
        m_ch2VoltsDiv->setValue(scope->ch2VoltsPerDiv());
        m_timeDiv->setValue(scope->timePerDiv());
        m_ch1Offset->setValue(scope->ch1VerticalOffset());
        m_ch2Offset->setValue(scope->ch2VerticalOffset());
        updateMemoryLabel();
    }
    m_updating = false;
}

void OscilloscopePanel::applyControlsToScope()
{
    if (m_updating) return;
    auto scope = m_scope.lock();
    if (!scope) return;
    scope->setCh1Enabled(m_ch1Box->isChecked());
    scope->setCh2Enabled(m_ch2Box->isChecked());
    scope->setCh1VoltsPerDiv(m_ch1VoltsDiv->value());
    scope->setCh2VoltsPerDiv(m_ch2VoltsDiv->value());
    scope->setTimePerDiv(m_timeDiv->value());
    scope->setCh1VerticalOffset(m_ch1Offset->value());
    scope->setCh2VerticalOffset(m_ch2Offset->value());
    updateMemoryLabel();
    refresh();
}

void OscilloscopePanel::updateMemoryLabel()
{
    auto scope = m_scope.lock();
    if (!scope) {
        m_memoryLabel->setText("Auto");
        return;
    }
    m_memoryLabel->setText(scope->memorySummary());
}

void OscilloscopePanel::refresh()
{
    auto scope = m_scope.lock();
    if (scope) {
        updateMemoryLabel();
        m_status->setText(QString("Selected: %1 — %2 — Memory: %3")
                          .arg(scope->label(), scope->channelSummary(), scope->memorySummary()));
    } else {
        m_status->setText("No oscilloscope selected");
    }
    m_plot->update();
}

void OscilloscopePanel::clearScope()
{
    auto scope = m_scope.lock();
    if (scope) scope->clearSamples();
    refresh();
}
