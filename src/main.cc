#include <QApplication>
#include "signals_handler.hh"
#include "mainwindow.hh"

int main(int argc, char** argv)
{
  set_signal_handler();

  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
