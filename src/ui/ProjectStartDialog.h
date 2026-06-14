#pragma once
#include <QDialog>
#include <QString>
class QListWidget;

class ProjectStartDialog : public QDialog {
    Q_OBJECT
public:
    enum class Choice { None, NewProject, OpenProject, RecentProject };
    explicit ProjectStartDialog(QWidget* parent = nullptr);
    Choice choice() const { return m_choice; }
    QString recentPath() const { return m_recentPath; }
private:
    Choice m_choice {Choice::None};
    QString m_recentPath;
    QListWidget* m_recent {nullptr};
};
