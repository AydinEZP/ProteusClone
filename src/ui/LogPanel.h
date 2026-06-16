#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QStringList>

class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);
    void appendMessages(const QStringList& msgs);
    void clear();

private:
    QTextEdit* m_textEdit;
};