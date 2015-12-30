#include <ostream>
#include <QApplication>
#include "signals_handler.hh"
#include "mainwindow.hh"

static const char* stylesheet =
  #include "../qdarkstyle/style.qss"
  ;

static void usage(const char* const prog_name, std::ostream& out_stream = std::cerr)
{
  out_stream << "Usage: " << prog_name << " [midi_file]\n";
}

int main(int argc, const char* const * const argv)
{
  if (argc > 2)
  {
    usage(argv[0]);
    return 2;
  }

  set_signal_handler();

  int dummy { 0 };
  QApplication a(dummy, nullptr);
  a.setStyleSheet(stylesheet);
  MainWindow w;
  w.show();

  if (argc == 2)
  {
    const std::string filename { argv[1] };
    w.open_file( filename );
  }

  return a.exec();
}
