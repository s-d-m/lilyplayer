#include "mainwindow.hh"
#include "ui_mainwindow.hh"

// void display_keyboard(QGraphicsScene& scene __attribute__((unused)));
// void display_keyboard(QGraphicsScene& scene __attribute__((unused)))
// {
//   // http://www.clipartbest.com/blank-piano-keyboard-printout
// }

#pragma GCC diagnostic push
#if !defined(__clang__)
  #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant" // Qt is not effective-C++ friendy
#endif

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  scene(new QGraphicsScene(this))
{
  ui->setupUi(this);

  ui->keyboard->setScene(scene);
}

#pragma GCC diagnostic pop

MainWindow::~MainWindow()
{
  delete ui;
}
