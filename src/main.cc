#include <QApplication>
#include "signals_handler.hh"
#include "mainwindow.hh"

static const char* stylesheet =
  #include "../qdarkstyle/style.qss"
  ;

int main(int argc, char** argv)
{
  set_signal_handler();

  QApplication a(argc, argv);
  a.setStyleSheet(stylesheet);
  MainWindow w;
  w.show();

  return a.exec();
}
