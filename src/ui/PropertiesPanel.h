#pragma once
#include <QWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <memory>
#include <QMap>
#include "../domain/Component.h"

/**
 * Right-side panel: shows and edits selected component properties.
 */
class PropertiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesPanel(QWidget* parent = nullptr);
    void showComponent(std::shared_ptr<Component> comp);
    void clear();

signals:
    void propertyChanged(ComponentID id, const QString& key, const QString& value);

private:
    QFormLayout* m_form;
    std::shared_ptr<Component> m_current;
    QMap<QString, QLineEdit*> m_fields;
};