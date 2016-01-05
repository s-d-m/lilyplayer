#include <signal.h>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
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

void MainWindow::keyPressEvent(QKeyEvent* event)
{
  const auto pressed_key = event->key();

  if ((pressed_key == Qt::Key_Space) or
      (pressed_key == Qt::Key_P) or
      (pressed_key == Qt::Key_Pause))
  {
    // toggle play pause
    is_in_pause = not is_in_pause;
  }
}

void MainWindow::process_keyboard_event(const music_event& keys_event)
{
  update_keyboard(keys_event, this->keyboard);
  draw_keyboard(*(this->scene), this->keyboard);

  for (const auto& message : keys_event.midi_messages)
  {
    auto tmp = message; // can't use message directly since message is const and
			// sendMessage doesn't take a const vector
    sound_player.sendMessage(&tmp);

    // could use the following to cast the const away: but since there is no
    // guarantee that the libRtMidi doesn't modify the data ... (I know the I
    // can read the code)
    //
    //sound_player.sendMessage(const_cast<midi_message*>(&message));
  }
}

void MainWindow::song_event_loop()
{
  if (song_pos == INVALID_SONG_POS)
  {
    return;
  }

  if (is_in_pause)
  {
    QTimer::singleShot(100, this, SLOT(song_event_loop()));
    return;
  }

  if ((song_pos != 0) and (song_pos >= song.size()))
  {
    throw std::runtime_error("Invalid song position found");
  }

  process_keyboard_event( song[song_pos] );

  if (song_pos + 1 == song.size())
  {
    // song is finished
    song_pos = INVALID_SONG_POS;
  }
  else
  {
    // call this function back for the next event
    song_pos++;
    QTimer::singleShot( static_cast<int>((song[song_pos].time - song[song_pos - 1].time) / 1'000'000), this, SLOT(song_event_loop()) );
  }

}

void MainWindow::stop_song()
{
  // for each key, create a release key event. No matter if the key was actually
  // pressed or not
  music_event all_keys_up;
  constexpr auto nb_keys = note_kind::do_8 - note_kind::la_0 + 1;
  all_keys_up.key_events.reserve(nb_keys);
  all_keys_up.midi_messages.reserve(nb_keys);

  for (auto key = static_cast<uint8_t>(note_kind::la_0);
       key <= static_cast<uint8_t>(note_kind::do_8);
       ++key)
  {
    all_keys_up.key_events.push_back(key_data{ .pitch = key,
					        .ev_type = key_data::type::released });
    all_keys_up.midi_messages.push_back(
      std::vector<uint8_t>{ 0x80, key, 0x00 } );
  }

  // reinitialise the song field
  this->song.resize(0);
  this->song_pos = INVALID_SONG_POS;

  process_keyboard_event(all_keys_up);
}

void MainWindow::open_file(const std::string& filename)
{
  try
  {
    stop_song();
    const auto midi_events = get_midi_events(filename);
    const auto keyboard_events = get_key_events(midi_events);
    this->song = group_events_by_time(midi_events, keyboard_events);
    this->song_pos = 0;
    song_event_loop();
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

      open_file(filename);
    }
  }
}

void MainWindow::set_output_port(const unsigned int i)
{
  sound_player.closePort();
  sound_player.openPort(i);
  const auto port_name = sound_player.getPortName(i);
  this->selected_output_port = port_name;
  this->update_output_ports();
}

void MainWindow::output_port_change()
{
  // find out which output port is now checked.
  auto menu_bar = this->menuBar();
  if (menu_bar == nullptr)
  {
    std::cerr << "Error: couldn't find the menu bar\n";
    return;
  }

  auto menu_output_port = menu_bar->findChild<QMenu*>("menuOutput_port",
						      Qt::FindDirectChildrenOnly);
  if (menu_output_port == nullptr)
  {
    std::cerr << "Error: couldn't find the output ports menu\n";
    return;
  }

  auto button_list = menu_output_port->findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
  for (auto& button : button_list)
  {
    if (button->isChecked())
    {
      this->selected_output_port = button->text().toStdString();
      const auto nb_ports = sound_player.getPortCount();
      for (unsigned int i = 0; i < nb_ports; ++i)
      {
	const auto port_name = sound_player.getPortName(i);
	if (port_name == selected_output_port)
	{
	  sound_player.closePort();
	  sound_player.openPort(i);
	}
      }
    }
  }
}

void MainWindow::update_output_ports()
{
  // find out which output port is now checked.
  auto menu_bar = this->menuBar();
  if (menu_bar == nullptr)
  {
    std::cerr << "Error: couldn't find the menu bar\n";
    return;
  }

  auto menu_output_port = menu_bar->findChild<QMenu*>("menuOutput_port",
						      Qt::FindDirectChildrenOnly);
  if (menu_output_port == nullptr)
  {
    std::cerr << "Error: couldn't find the output ports menu\n";
    return;
  }

  // remove all the children!
  menu_output_port->clear();

  // find the action group.
  auto action_group = menu_output_port->findChild<QActionGroup*>("",
								 Qt::FindDirectChildrenOnly);
  if (action_group == nullptr)
  {
    action_group = new QActionGroup( menu_output_port );
  }

  const auto nb_ports = sound_player.getPortCount();
  if (nb_ports == 0)
  {
    std::cerr << "Sorry: can't populate menu, no output midi port found\n";
  }
  else
  {
    for (unsigned int i = 0; i < nb_ports; ++i)
    {
      const auto port_name = sound_player.getPortName(i);
      const auto label = QString::fromStdString( port_name );
      auto button = menu_output_port->addAction(label);
      button->setCheckable(true);
      const auto select_this_port = ( port_name == selected_output_port );
      button->setChecked( select_this_port );

      button->setActionGroup( action_group );
      connect(button, SIGNAL(triggered()), this, SLOT(output_port_change()));
    }

  }
}

void MainWindow::input_change()
{
  // find out which menu item has been clicked.
  const auto menu_bar = this->menuBar();
  if (menu_bar == nullptr)
  {
    std::cerr << "Error: couldn't find the menu bar\n";
    return;
  }

  const auto menu_input = menu_bar->findChild<QMenu*>("menuInput",
						      Qt::FindDirectChildrenOnly);
  if (menu_input == nullptr)
  {
    std::cerr << "Error: couldn't find the output ports menu\n";
    return;
  }

  const auto button_list = menu_input->findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
  for (const auto& button : button_list)
  {
    if (button->isChecked())
    {
      this->stop_song();
      this->selected_input = button->text().toStdString();
    }
  }
}


void MainWindow::update_input_entries()
{
  // find out which input is currently selected
  auto menu_bar = this->menuBar();
  if (menu_bar == nullptr)
  {
    std::cerr << "Error: couldn't find the menu bar\n";
    return;
  }

  auto menu_input = menu_bar->findChild<QMenu*>("menuInput",
						Qt::FindDirectChildrenOnly);
  if (menu_input == nullptr)
  {
    std::cerr << "Error: couldn't find the input menu\n";
    return;
  }

  // remove all the children!
  menu_input->clear();

  // find the action group.
  auto action_group = menu_input->findChild<QActionGroup*>("",
							   Qt::FindDirectChildrenOnly);
  if (action_group == nullptr)
  {
    action_group = new QActionGroup( menu_input );
  }

  {
    // add the file entry in the input menu.  Warning: this implementation takes as
    // assumption that "select file" will never be the name of an input MIDI port.
    std::string label { "select file" };
    const auto Qlabel = QString::fromStdString( label );
    auto button = menu_input->addAction(Qlabel);
    button->setCheckable(true);
    const auto select_this_one = ( label == selected_input );
    button->setChecked( select_this_one );

    button->setActionGroup(action_group);
    connect(button, SIGNAL(triggered()), this, SLOT(input_change()));
  }

  {
    // Add one entry per input midi port
    const auto nb_ports = sound_listener.getPortCount();
    for (unsigned int i = 0; i < nb_ports; ++i)
    {
      const auto port_name = sound_listener.getPortName(i);
      const auto label = QString::fromStdString( port_name );
      auto button = menu_input->addAction(label);
      button->setCheckable(true);
      const auto select_this_port = ( port_name == selected_input );
      button->setChecked( select_this_port );

      button->setActionGroup( action_group );
      connect(button, SIGNAL(triggered()), this, SLOT(input_change()));
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
  song(),
  sound_player(RtMidi::LINUX_ALSA),
  sound_listener(RtMidi::LINUX_ALSA)
{
  ui->setupUi(this);
  ui->keyboard->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  ui->keyboard->setScene(scene);
  draw_keyboard(*scene, this->keyboard);

  connect(&signal_checker_timer, SIGNAL(timeout()), this, SLOT(look_for_signals_change()));
  signal_checker_timer.start(100 /* ms */);

  {
    // automatically open an output midi port if possible
    const auto nb_ports = sound_player.getPortCount();
    if (nb_ports == 0)
    {
      std::cerr << "Sorry: no output midi port found\n";
    }
    else
    {
      // automatically open an output midi port.
      const unsigned int port_to_use = (nb_ports == 1) ? 0 : 1;
      sound_player.openPort( port_to_use );
      this->selected_output_port = sound_player.getPortName( port_to_use );
    }
  }

  {
    // setting up the signal on_output_ports_menu_clicked->update_outputs_ports.
    // update_outputs_ports is the function that fills up the menu entries with all the ports
    // output ports available.
    auto menu_bar = this->menuBar();
    auto menu_output_port = menu_bar->findChild<QMenu*>("menuOutput_port",
							Qt::FindDirectChildrenOnly);
    if (menu_output_port == nullptr)
    {
      std::cerr << "Error: couldn't find the output port menu\n";
    }
    else
    {
      connect(menu_output_port, SIGNAL(aboutToShow()), this, SLOT(update_output_ports()));
    }
  }

  {
    // setting up the signal on_input_menu_clicked->update_input_entries. Update_input_entries fills up
    // the input menu with the available entries
    // an important difference between the input_menu and the output_menu is that the input is not necessary
    // a midi input port. It can be a file. And therefore, if there is no midi inputs detected, it's not a problem.
    // and also, there is no need to automatically select an input port.
    auto menu_bar = this->menuBar();
    auto menu_input = menu_bar->findChild<QMenu*>("menuInput",
						  Qt::FindDirectChildrenOnly);
    if (menu_input == nullptr)
    {
      std::cerr << "Error: couldn't find the input menu\n";
    }
    else
    {
      connect(menu_input, SIGNAL(aboutToShow()), this, SLOT(update_input_entries()));
    }
  }
}

#pragma GCC diagnostic pop

MainWindow::~MainWindow()
{
  delete ui;
  sound_player.closePort();
}
