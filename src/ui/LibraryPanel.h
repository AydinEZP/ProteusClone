#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QMap>

/**
 * Left-side component library with categorised tree, real-time search/filter,
 * schematic preview text, and active devices list similar to Proteus.
 */
class LibraryPanel : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPanel(QWidget* parent = nullptr);

signals:
    void componentSelected(const QString& typeName);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onSearchChanged(const QString& text);
    void onActiveItemClicked(QListWidgetItem* item);

private:
    QTreeWidget* m_tree {nullptr};
    QListWidget* m_activeList {nullptr};
    QLineEdit*   m_search {nullptr};
    QLabel*      m_preview {nullptr};
    QMap<QString, QStringList> m_categories;

    void populateTree(const QString& filter = QString());
    void addToActiveList(const QString& typeName);
    QString previewFor(const QString& typeName) const;
};
