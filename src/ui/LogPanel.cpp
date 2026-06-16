#include "LogPanel.h"
#include <QLabel>

#include <QFont>
LogPanel::LogPanel(QWidget* parent)
    : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);
    layout->addWidget(new QLabel("Simulation Log / DRC"));
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFont(QFont("Monospace", 8));
    layout->addWidget(m_textEdit);
}

void LogPanel::appendMessages(const QStringList& msgs)
{
    for (auto& m : msgs)
        m_textEdit->append(m);
}

void LogPanel::clear()
{
    m_textEdit->clear();
}
