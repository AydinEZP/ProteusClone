#pragma once
#include <QDialog>
class QLineEdit;
class QSpinBox;

class NewProjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget* parent = nullptr);
    QString projectName() const;
    int canvasWidth() const;
    int canvasHeight() const;
private:
    QLineEdit* m_name {nullptr};
    QSpinBox* m_width {nullptr};
    QSpinBox* m_height {nullptr};
};
