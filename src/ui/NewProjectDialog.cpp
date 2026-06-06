#include "NewProjectDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSize>

NewProjectDialog::NewProjectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("New Circuit Project");
    auto layout = new QVBoxLayout(this);
    auto form = new QFormLayout();
    m_name = new QLineEdit("Untitled", this);
    m_width = new QSpinBox(this); m_width->setRange(400, 10000); m_width->setValue(1600);
    m_height = new QSpinBox(this); m_height->setRange(300, 10000); m_height->setValue(1000);
    auto preset = new QComboBox(this);
    preset->addItem("Custom"); preset->addItem("A4 Landscape 1123x794", QSize(1123,794)); preset->addItem("A3 Landscape 1587x1123", QSize(1587,1123));
    form->addRow("Project name:", m_name);
    form->addRow("Preset:", preset);
    form->addRow("Canvas width:", m_width);
    form->addRow("Canvas height:", m_height);
    layout->addLayout(form);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(preset, &QComboBox::currentIndexChanged, this, [this,preset](int idx){ QSize s=preset->itemData(idx).toSize(); if(s.isValid()){ m_width->setValue(s.width()); m_height->setValue(s.height()); }});
}
QString NewProjectDialog::projectName() const { return m_name->text().trimmed().isEmpty()?"Untitled":m_name->text().trimmed(); }
int NewProjectDialog::canvasWidth() const { return m_width->value(); }
int NewProjectDialog::canvasHeight() const { return m_height->value(); }
