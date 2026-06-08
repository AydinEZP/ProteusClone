#include "LibraryPanel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QFont>
#include <QPainter>
#include <QIcon>
#include <QMap>
#include <QMenu>
#include "IconProvider.h"

namespace {
QIcon libraryIcon(const QString& name)
{
    return IconProvider::icon(name);
}

QString iconKeyForComponent(const QString& type)
{
    static const QMap<QString, QString> map = {
        {"DCVoltageSource", "source"}, {"Battery", "battery"}, {"Ground", "ground"}, {"ClockGenerator", "clock"},
        {"Resistor", "resistor"}, {"Capacitor", "capacitor"}, {"Inductor", "inductor"}, {"Potentiometer", "resistor"},
        {"Switch", "switch"}, {"PushButton", "pushbutton"}, {"LED", "led"}, {"SevenSegment", "sevensegment"},
        {"AndGate", "gate"}, {"OrGate", "gate"}, {"NotGate", "gate"}, {"XorGate", "gate"}, {"NandGate", "gate"}, {"DFlipFlop", "dff"},
        {"SimpleADC", "adc"}, {"SimpleDAC", "dac"}, {"Microcontroller", "mcu"}, {"ExternalMemory", "mcu"}, {"LCD16x2", "lcd"}, {"Keypad", "keypad"},
        {"VoltageProbe", "probe"}, {"Voltmeter", "voltmeter"}, {"Ammeter", "ammeter"}, {"Oscilloscope", "oscilloscope"}
    };
    return map.value(type, "app");
}

QString iconKeyForCategory(const QString& category)
{
    static const QMap<QString, QString> map = {
        {"Sources", "source"}, {"Passive", "passive"}, {"Interactive", "interactive"},
        {"Digital", "digital"}, {"Advanced", "advanced"}, {"Measurement", "measurement"}
    };
    return map.value(category, "app");
}
}

LibraryPanel::LibraryPanel(QWidget* parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);

    auto title = new QLabel("Component Library", this);
    title->setStyleSheet("font-weight:bold;");
    layout->addWidget(title);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Search components or categories...");
    layout->addWidget(m_search);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    layout->addWidget(m_tree, 3);

    m_preview = new QLabel("Select a component to preview schematic pins.", this);
    m_preview->setWordWrap(true);
    m_preview->setMinimumHeight(80);
    m_preview->setStyleSheet("QLabel { border: 1px solid palette(mid); padding: 6px; }");
    layout->addWidget(new QLabel("Schematic Preview", this));
    layout->addWidget(m_preview);

    layout->addWidget(new QLabel("Active Devices", this));
    m_activeList = new QListWidget(this);
    m_activeList->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_activeList, 1);

    m_categories["Sources"]     = {"DCVoltageSource","Battery","Ground","ClockGenerator"};
    m_categories["Passive"]     = {"Resistor","Capacitor","Inductor","Potentiometer"};
    m_categories["Interactive"] = {"Switch","PushButton","LED","SevenSegment"};
    m_categories["Digital"]     = {"AndGate","OrGate","NotGate","XorGate","NandGate","DFlipFlop"};
    m_categories["Advanced"]    = {"SimpleADC","SimpleDAC","Microcontroller","ExternalMemory","LCD16x2","Keypad"};
    m_categories["Measurement"] = {"VoltageProbe","Voltmeter","Ammeter","Oscilloscope"};

    populateTree();
    connect(m_tree, &QTreeWidget::itemClicked, this, &LibraryPanel::onItemClicked);
    connect(m_search, &QLineEdit::textChanged, this, &LibraryPanel::onSearchChanged);
    connect(m_activeList, &QListWidget::itemClicked, this, &LibraryPanel::onActiveItemClicked);
    connect(m_activeList, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos){
        auto* item = m_activeList->itemAt(pos);
        if (!item) return;
        QMenu menu(this);
        QAction* remove = menu.addAction("Remove from active devices");
        if (menu.exec(m_activeList->viewport()->mapToGlobal(pos)) == remove)
            delete m_activeList->takeItem(m_activeList->row(item));
    });
}

void LibraryPanel::populateTree(const QString& filter) {
    m_tree->clear();
    QString f = filter.trimmed().toLower();
    for (auto it = m_categories.begin(); it != m_categories.end(); ++it) {
        bool categoryMatches = it.key().toLower().contains(f);
        auto cat = new QTreeWidgetItem(QStringList{it.key()});
        cat->setIcon(0, libraryIcon(iconKeyForCategory(it.key())));
        cat->setFlags(cat->flags() & ~Qt::ItemIsSelectable);
        QFont bold; bold.setBold(true); cat->setFont(0, bold);
        int count=0;
        for (auto& t : it.value()) {
            if (!f.isEmpty() && !categoryMatches && !t.toLower().contains(f)) continue;
            auto item = new QTreeWidgetItem(cat, QStringList{t});
            item->setIcon(0, libraryIcon(iconKeyForComponent(t)));
            item->setData(0, Qt::UserRole, t);
            ++count;
        }
        if (count>0 || f.isEmpty()) m_tree->addTopLevelItem(cat); else delete cat;
    }
    m_tree->expandAll();
    if (m_tree->topLevelItemCount()==0) {
        auto none = new QTreeWidgetItem(QStringList{"No component found"});
        none->setFlags(none->flags() & ~Qt::ItemIsSelectable);
        m_tree->addTopLevelItem(none);
    }
}

void LibraryPanel::onSearchChanged(const QString& text) { populateTree(text); }

void LibraryPanel::addToActiveList(const QString& typeName) {
    for(int i=0;i<m_activeList->count();++i) if(m_activeList->item(i)->text()==typeName) return;
    m_activeList->addItem(new QListWidgetItem(libraryIcon(iconKeyForComponent(typeName)), typeName));
}

void LibraryPanel::onItemClicked(QTreeWidgetItem* item, int) {
    QString type = item->data(0, Qt::UserRole).toString();
    if (type.isEmpty()) return;
    m_preview->setText(previewFor(type));
    addToActiveList(type);
    emit componentSelected(type);
}

void LibraryPanel::onActiveItemClicked(QListWidgetItem* item) {
    if(!item) return;
    m_preview->setText(previewFor(item->text()));
    emit componentSelected(item->text());
}

QString LibraryPanel::previewFor(const QString& typeName) const {
    static QMap<QString,QString> p = {
        {"Resistor", "R: passive two-terminal component. Pins: A, B. Property: resistance."},
        {"Capacitor", "C: energy storage component. Pins: A, B. Property: capacitance."},
        {"Inductor", "L: energy storage component. Pins: A, B. Property: inductance."},
        {"Potentiometer", "Variable resistor with A/B ends and W wiper. Live wiper ratio can feed ADC."},
        {"DCVoltageSource", "DC source. Pins: POS, NEG. Property: voltage."},
        {"Battery", "Battery symbol. Pins: POS, NEG. Property: voltage."},
        {"Ground", "0V reference. At least one GND is recommended for simulation."},
        {"ClockGenerator", "Digital square-wave source. Pin: OUT. Property: frequency."},
        {"Switch", "Interactive toggle switch. Pins: A, B."},
        {"PushButton", "Momentary digital source. Pin: OUT. Mouse held = HIGH (5V), released = LOW (0V). Ctrl+drag moves it."},
        {"LED", "Visual output. Pins: A/K. Turns on during simulation."},
        {"SevenSegment", "7-segment LED display. Pins: A-G, DP, COM."},
        {"AndGate", "Digital AND gate. Pins: IN1, IN2, OUT."},
        {"OrGate", "Digital OR gate. Pins: IN1, IN2, OUT."},
        {"NotGate", "Digital inverter. Pins: IN1, OUT."},
        {"XorGate", "Digital XOR gate. Pins: IN1, IN2, OUT."},
        {"NandGate", "Digital NAND gate. Pins: IN1, IN2, OUT."},
        {"DFlipFlop", "D flip-flop with rising-edge CLK. Pins: D, CLK, Q, QB."},
        {"SimpleADC", "Ideal ADC. Inputs: VIN, VREF+, VREF-, CLK. Outputs: D0..D(N-1), each visibly labeled on the symbol."},
        {"SimpleDAC", "Ideal DAC. Inputs: D0..D(N-1), VREF+, VREF-. Output: VOUT. Pins are visibly labeled on the symbol."},
        {"Microcontroller", "Educational MCU with four 8-bit GPIO ports P0..P3. Every bit can be input or output; VCC/GND are labeled."},
        {"ExternalMemory", "256-byte external RAM with A0..A7, bidirectional D0..D7, active-low RD and active-low WR. Supports both read and write bus cycles."},
        {"LCD16x2", "Live 16x2 LCD with RS/RW/E/D0..D7 bus and basic clear/cursor/write commands."},
        {"Keypad", "Live clickable 4x4 matrix keypad. Hold the mouse on a key to close its row/column contact; release to open it. Ctrl+drag moves the component."},
        {"VoltageProbe", "Voltage probe / marker. Pin: IN."},
        {"Voltmeter", "Digital voltmeter. Pins: POS, NEG."},
        {"Ammeter", "Digital ammeter. Pins: IN, OUT."},
        {"Oscilloscope", "Simple oscilloscope display. Pins: CH1, GND."}
    };
    return p.value(typeName, typeName + ": preview not available.");
}
