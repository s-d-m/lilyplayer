#include <signal.h>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>

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
  if (song_pos == INVALID_SONG_POS)
  {
    return;
  }

  if (is_in_pause)
  {
    QTimer::singleShot(100, this, SLOT(process_keyboard_event()));
    return;
  }

  if ((song_pos != 0) and (song_pos >= song.size()))
  {
    throw std::runtime_error("Invalid song position found");
  }

  const auto& key_events = song[song_pos].key_events;

  /* update the keyboard */
  for (const auto& k_ev : key_events)
  {
    switch (k_ev.ev_type)
    {
      case key_data::type::pressed:
	set_color(keyboard, static_cast<enum note_kind>(k_ev.pitch), Qt::blue, Qt::cyan);
	break;

      case key_data::type::released:
	reset_color(keyboard, static_cast<enum note_kind>(k_ev.pitch));
	break;

#if !defined(__clang__)
// clang complains that all values are handled in the switch and issue
// a warning for the default case
// gcc complains about a missing default
      default:
	  break;
#endif
    }
  }

  draw_keyboard(*(this->scene), this->keyboard);

  if (song_pos + 1 == song.size())
  {
    // song is finished
    song_pos = INVALID_SONG_POS;
  }
  else
  {
    // call this function back for the next event
    song_pos++;
    QTimer::singleShot( static_cast<int>((song[song_pos].time - song[song_pos - 1].time) / 1'000'000), this, SLOT(process_keyboard_event()) );
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
      const auto filename = file.toStdString();

      try
      {
	const auto midi_events = get_midi_events(filename);
	const auto keyboard_events = get_key_events(midi_events);
	this->song = group_events_by_time(midi_events, keyboard_events);
	this->song_pos = 0;
	process_keyboard_event();
      }
      catch (std::exception& e)
      {
	const auto err_msg = e.what();
	QMessageBox::critical(this, tr("Failed to open file."),
			      err_msg,
			      QMessageBox::Ok,
			      QMessageBox::Ok);
      }

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
  signal_checker_timer(),
  song()
{
  ui->setupUi(this);
  ui->keyboard->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  ui->keyboard->setScene(scene);
  draw_keyboard(*scene, this->keyboard);

  connect(&signal_checker_timer, SIGNAL(timeout()), this, SLOT(look_for_signals_change()));
  signal_checker_timer.start(100 /* ms */);
}

#pragma GCC diagnostic pop

MainWindow::~MainWindow()
{
  delete ui;
}
