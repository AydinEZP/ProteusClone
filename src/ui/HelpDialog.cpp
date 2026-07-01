#include "HelpDialog.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

HelpDialog::HelpDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("ProteusClone Help");
    resize(920, 720);

    auto* root = new QVBoxLayout(this);

    auto* header = new QLabel("<h2>ProteusClone User Help</h2>", this);
    header->setTextFormat(Qt::RichText);
    root->addWidget(header);

    m_browser = new QTextBrowser(this);
    // Follow the current application palette so Help remains readable in both
    // Light and Dark themes. The guide HTML intentionally avoids fixed body text colors.
    m_browser->setStyleSheet(
        "QTextBrowser { background: palette(base); color: palette(text); }");
    m_browser->setOpenExternalLinks(false);
    m_browser->setSearchPaths({":/help"});

    QFile file(":/help/user_guide.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_browser->setHtml(QString::fromUtf8(file.readAll()));
    } else {
        m_browser->setHtml(
            "<h1>ProteusClone Help</h1>"
            "<p>The embedded help file could not be loaded from the Qt resource system.</p>"
            "<p>Check <code>resources/resources.qrc</code> and make sure "
            "<code>resources/help/user_guide.html</code> is included in the build.</p>"
        );
    }
    root->addWidget(m_browser, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);
}
