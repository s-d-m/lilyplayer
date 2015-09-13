#include <signal.h>
#include <iostream>
#include <QFileDialog>


#include "mainwindow.hh"
#include "ui_mainwindow.hh"


// Global variables to "share" state between the signal handler and
// the main event loop.  Only these two pieces should be allowed to
// use these global variables.  To avoid any other piece piece of code
// from using it, the declaration is not written on a header file on
// purpose.
extern volatile sig_atomic_t pause_requested;
extern volatile sig_atomic_t continue_requested;
extern volatile sig_atomic_t exit_requested;


void MainWindow::look_for_signals_change()
{

  if (exit_requested)
  {
    close();
  }

  if (pause_requested)
  {
    pause_requested = 0;
    is_in_pause = true;
  }

  if (continue_requested)
  {
    continue_requested = 0;
    is_in_pause = false;
  }

}

void MainWindow::process_keyboard_event()
{
  if (not is_in_pause)
  {
    static uint8_t current_key = 0;

    set_color(this->keyboard, static_cast<enum note_kind>(21 + current_key), Qt::blue, Qt::yellow);
    // The + 88 is necessary for the case current_key == 0. Just so current_key - 1 doesn't wrap
    // and the computation remains accurate
    reset_color(this->keyboard, static_cast<enum note_kind>(21 + ((current_key + 88 - 1) % 88)));


    draw_keyboard(*(this->scene), this->keyboard);

    current_key = static_cast<decltype(current_key)>((current_key + 1) % 88);
  }
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
  connect(&timer, SIGNAL(timeout()), this, SLOT(look_for_signals_change()));
  timer.start(1000 /* ms */);
}

#pragma GCC diagnostic pop

MainWindow::~MainWindow()
{
  delete ui;
}
