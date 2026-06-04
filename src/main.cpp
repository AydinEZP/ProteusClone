#include <QApplication>
#include "ui/MainWindow.h"
#include "ui/IconProvider.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(resources);

    app.setApplicationName("ProteusClone");
    app.setOrganizationName("OOPCourse");
    app.setWindowIcon(IconProvider::icon("app"));

    MainWindow window;
    window.resize(1280, 800);
    window.show();

    return app.exec();
}
