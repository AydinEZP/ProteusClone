#include "PropertiesPanel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

PropertiesPanel::PropertiesPanel(QWidget* parent)
    : QWidget(parent)
{
    auto vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(4,4,4,4);
    vbox->addWidget(new QLabel("Properties"));
    auto formWidget = new QWidget(this);
    m_form = new QFormLayout(formWidget);
    vbox->addWidget(formWidget);
    vbox->addStretch();
}

void PropertiesPanel::clear()
{
    m_current.reset();
    m_fields.clear();
    while (m_form->rowCount() > 0)
        m_form->removeRow(0);
}

void PropertiesPanel::showComponent(std::shared_ptr<Component> comp)
{
    clear();
    if (!comp) return;
    m_current = comp;

    auto typeLabel = new QLabel(comp->type(), this);
    typeLabel->setStyleSheet("font-weight:bold;");
    m_form->addRow("Type:", typeLabel);

    // IMPORTANT: keep the properties map alive while iterating.
    // Iterating directly over comp->properties().begin()/end() iterates over
    // different temporaries and can crash as soon as a component is selected.
    const QMap<QString, QString> props = comp->properties();
    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
        auto edit = new QLineEdit(it.value(), this);
        m_form->addRow(it.key() + ":", edit);
        m_fields[it.key()] = edit;

        const QString key = it.key();
        connect(edit, &QLineEdit::editingFinished, this, [this, edit, key](){
            if (!m_current) return;
            emit propertyChanged(m_current->id(), key, edit->text());
        });
    }
}
