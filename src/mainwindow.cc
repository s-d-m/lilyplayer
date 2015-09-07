#include <iostream>
#include <QFileDialog>


#include "mainwindow.hh"
#include "ui_mainwindow.hh"


void MainWindow::process_keyboard_event()
{
  static uint8_t current_key = 0;

  set_color(this->keyboard, static_cast<enum note_kind>(21 + current_key), Qt::blue, Qt::yellow);
  // The + 88 is necessary for the case current_key == 0. Just so current_key - 1 doesn't wrap
  // and the computation remains accurate
  reset_color(this->keyboard, static_cast<enum note_kind>(21 + ((current_key + 88 - 1) % 88)));


  draw_keyboard(*(this->scene), this->keyboard);

  current_key = static_cast<decltype(current_key)>((current_key + 1) % 88);
}



void MainWindow::open_file()
{
  QStringList filters;
  filters << "Midi files (*.midi *.mid)"
  	  << "Any files (*)";

  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setViewMode(QFileDialog::List);
  dialog.setNameFilters(filters);

  const auto dialog_ret = dialog.exec();
  if (dialog_ret == QDialog::Accepted)
  {
    const auto files = dialog.selectedFiles();
    if (files.length() == 1)
    {
      const auto file = files[0];
      this->current_midi_file = file.toStdString();
      std::cout << "selected: " << this->current_midi_file << std::endl;
    }
  }
}

#pragma GCC diagnostic push
#if !defined(__clang__)
  #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant" // Qt is not effective-C++ friendy
#endif

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  scene(new QGraphicsScene(this)),
  keyboard(),
  timer(),
  current_midi_file()
{
  ui->setupUi(this);
  ui->keyboard->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  ui->keyboard->setScene(scene);
  draw_keyboard(*scene, this->keyboard);

  connect(&timer, SIGNAL(timeout()), this, SLOT(process_keyboard_event()));
  timer.start(1000 /* ms */);
}

#pragma GCC diagnostic pop

MainWindow::~MainWindow()
{
  delete ui;
}
