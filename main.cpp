#include <QApplication>
#include "GUI.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    GUI window;
    window.setWindowTitle("Image Redactor");
    window.resize(1000, 800);
    window.show();
    
    return app.exec();
}