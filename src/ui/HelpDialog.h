#pragma once

#include <QDialog>

class QTextBrowser;

class HelpDialog : public QDialog {
    Q_OBJECT
public:
    explicit HelpDialog(QWidget* parent = nullptr);

private:
    QTextBrowser* m_browser {nullptr};
};
