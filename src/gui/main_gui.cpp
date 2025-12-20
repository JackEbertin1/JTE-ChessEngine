#include <QApplication>
#include "gui/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("BlunderBot");

    MainWindow window;
    window.show();

    return app.exec();
}
