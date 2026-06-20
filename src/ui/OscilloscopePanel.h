#pragma once
#include <QWidget>
#include <QPointer>
#include <memory>

class QLabel;
class QCheckBox;
class QDoubleSpinBox;
class QPushButton;
class Oscilloscope;

/** Dedicated oscilloscope UI panel.
 *
 * Provides a full-size plot and live controls required by the project statement:
 * two independent channels, different colours, shared Time/Div, and per-channel
 * Volt/Div and vertical offset controls. The physical connection is still done
 * on the schematic through the Oscilloscope component pins CH1, CH2 and GND.
 */
class ScopePlotWidget : public QWidget {
public:
    explicit ScopePlotWidget(QWidget* parent = nullptr);
    void setScope(std::shared_ptr<Oscilloscope> scope);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::weak_ptr<Oscilloscope> m_scope;
};

class OscilloscopePanel : public QWidget {
public:
    explicit OscilloscopePanel(QWidget* parent = nullptr);
    void setScope(std::shared_ptr<Oscilloscope> scope);
    void refresh();
    void clearScope();

private:
    void updateControlsFromScope();
    void applyControlsToScope();
    void updateMemoryLabel();

    std::weak_ptr<Oscilloscope> m_scope;
    ScopePlotWidget* m_plot{nullptr};
    QLabel* m_status{nullptr};
    QCheckBox* m_ch1Box{nullptr};
    QCheckBox* m_ch2Box{nullptr};
    QDoubleSpinBox* m_ch1VoltsDiv{nullptr};
    QDoubleSpinBox* m_ch2VoltsDiv{nullptr};
    QDoubleSpinBox* m_timeDiv{nullptr};
    QDoubleSpinBox* m_ch1Offset{nullptr};
    QDoubleSpinBox* m_ch2Offset{nullptr};
    QLabel* m_memoryLabel{nullptr};
    QPushButton* m_clearButton{nullptr};
    bool m_updating{false};
};
