#include "ProjectStartDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSettings>

ProjectStartDialog::ProjectStartDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("ProteusClone Start Menu");
    resize(520, 360);
    auto layout = new QVBoxLayout(this);
    auto title = new QLabel("ProteusClone - OOP Circuit Simulator", this);
    title->setStyleSheet("font-size:18px; font-weight:bold;");
    layout->addWidget(title);
    layout->addWidget(new QLabel("Create a new project, open a file, or resume a recent project.", this));
    auto row = new QHBoxLayout();
    auto newBtn = new QPushButton("New Project", this);
    auto openBtn = new QPushButton("Open Project", this);
    auto cancelBtn = new QPushButton("Continue Empty", this);
    row->addWidget(newBtn); row->addWidget(openBtn); row->addWidget(cancelBtn);
    layout->addLayout(row);
    layout->addWidget(new QLabel("Recent Projects", this));
    m_recent = new QListWidget(this);
    QSettings s; QStringList recent=s.value("recentProjects").toStringList();
    for(const auto& path: recent) m_recent->addItem(path);
    if(recent.isEmpty()) m_recent->addItem("No recent projects");
    layout->addWidget(m_recent);
    connect(newBtn,&QPushButton::clicked,this,[this](){m_choice=Choice::NewProject; accept();});
    connect(openBtn,&QPushButton::clicked,this,[this](){m_choice=Choice::OpenProject; accept();});
    connect(cancelBtn,&QPushButton::clicked,this,[this](){m_choice=Choice::None; accept();});
    connect(m_recent,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem* item){ if(!item || item->text().startsWith("No recent")) return; m_recentPath=item->text(); m_choice=Choice::RecentProject; accept(); });
}
