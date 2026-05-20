#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;

    lightPalette.setColor(QPalette::Window, QColor(244, 246, 249));
    lightPalette.setColor(QPalette::WindowText, QColor(47, 54, 64));
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(245, 246, 250));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(47, 54, 64));
    lightPalette.setColor(QPalette::Text, QColor(47, 54, 64));
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, QColor(47, 54, 64));

    QApplication::setPalette(lightPalette);

    MainWindow w;
    w.show();
    return a.exec();
}