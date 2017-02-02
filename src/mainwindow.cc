#include <signal.h>
#include <iostream>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QGraphicsSvgItem>
#include <QGraphicsRectItem>
#include "mainwindow.hh"
#include "ui_mainwindow.hh"
#include "measures_sequence_extractor.hh"

// Global variables to "share" state between the signal handler and
// the main event loop.  Only these two pieces should be allowed to
// use these global variables.  To avoid any other piece piece of code
// from using it, the declaration is not written on a header file on
// purpose.
extern volatile sig_atomic_t pause_requested;
extern volatile sig_atomic_t continue_requested;
extern volatile sig_atomic_t exit_requested;
extern volatile sig_atomic_t new_signal_received;


void MainWindow::look_for_signals_change()
{
  if (not new_signal_received)
  {
    return;
  }

  if (exit_requested)
  {
    close();
  }

  if (pause_requested)
  {
    pause_requested = 0;
    pause_music();
  }

  if (continue_requested)
  {
    continue_requested = 0;
    is_in_pause = false;
  }

  new_signal_received = 0;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
  const auto pressed_key = event->key();

  if ((pressed_key == Qt::Key_Space) or
      (pressed_key == Qt::Key_P) or
      (pressed_key == Qt::Key_Pause))
  {
    // toggle play pause
    if (is_in_pause)
    {
      is_in_pause = false;
    }
    else
    {
      pause_music();
    }
  }
}

void MainWindow::process_keyboard_event(const std::vector<key_down>& keys_down,
					const std::vector<key_up>& keys_up,
					const std::vector<midi_message_t>& messages)
{
  update_keyboard(keys_down, keys_up, this->keyboard);
  this->update();

  for (auto message : messages)
  {
    sound_player.sendMessage(&message);
  }
}

void MainWindow::display_music_sheet(const unsigned music_sheet_pos)
{
  // remove all the music sheets
  music_sheet_scene->clear();

  const auto nb_svg = this->song.svg_files.size();
  const auto nb_rendered = rendered_sheets.size();

  if (nb_svg != nb_rendered)
  {
    throw std::runtime_error("Invalid state detected. The numbered of rendered file should match the number"
			     " of svg files");
  }

  if (music_sheet_pos >= nb_rendered)
  {
    throw std::runtime_error("Invalid file format: it doesn't have enough music sheets.\n"
			     "This should have been prevented from happening while reading the input file.");
  }

  auto sheet = new QGraphicsSvgItem;
  sheet->setSharedRenderer(rendered_sheets[music_sheet_pos].rendered);
  sheet->setFlags(QGraphicsItem::ItemClipsToShape);
  sheet->setCacheMode(QGraphicsItem::NoCache);
  sheet->setZValue(0);
  music_sheet_scene->addItem(sheet);

  // if there was a rectangle displayed the clear function would have called the destructor
  svg_rect = new QGraphicsSvgItem;
  svg_rect->setFlags(QGraphicsItem::ItemClipsToShape);
  svg_rect->setCacheMode(QGraphicsItem::NoCache);
  svg_rect->setZValue(1);
  svg_rect->setSharedRenderer( cursor_rect );
  music_sheet_scene->addItem(svg_rect);

  QByteArray svg_str_rectangle (rendered_sheets[music_sheet_pos].svg_first_line.c_str());
  current_page_viewbox = rendered_sheets[music_sheet_pos].rendered->viewBoxF();
  svg_str_rectangle +=
    "<rect x=\"0.0000\" y=\"0.0000\" width=\"0.0000\" height=\"0.0000\""
    " ry=\"0.0000\" fill=\"currentColor\" fill-opacity=\"0.4\"/></svg>";
  cursor_rect->load(svg_str_rectangle);
}

void MainWindow::process_music_sheet_event(const music_sheet_event& event)
{
  // process the keyboard event. Must have one.
  this->process_keyboard_event(event.keys_down, event.keys_up, event.midi_messages);

  // is there a svg file change?
  if (event.has_svg_file_change())
  {
    display_music_sheet(event.new_svg_file);
  }

  // is there a cursor pos change here?
  if (event.has_cursor_pos_change())
  {
    cursor_rect->load(event.new_cursor_box);

    const auto scene_bounding_rect = svg_rect->sceneBoundingRect();
    const auto scene_bounding_rect_height = scene_bounding_rect.height();
    const auto half_cursor_box_height = event.cursor_box_coord.height() / 2;
    const auto current_page_viewbox_height = current_page_viewbox.height();

    const auto to_scene_y = [&] (const auto y) {
      return y * scene_bounding_rect_height / current_page_viewbox_height;
    };

    const auto rect_to_center = QRectF{scene_bounding_rect.left(),
				       to_scene_y(std::max(event.cursor_box_coord.top() - half_cursor_box_height, 0.0)),
				       scene_bounding_rect.width(),
				       to_scene_y(3 * half_cursor_box_height)};

    this->ui->music_sheet->setSceneRect(rect_to_center);
  }
}

void MainWindow::song_event_loop()
{
  if (is_in_pause or (song_pos == INVALID_SONG_POS))
  {
    QTimer::singleShot(100, this, SLOT(song_event_loop()));
    return;
  }

  if (song_pos == stop_pos)
  {
    stop_song();
    QTimer::singleShot(100, this, SLOT(song_event_loop()));
    return;
  }

  if (song_pos > song.nb_events)
  {
    throw std::runtime_error("Invalid song position found");
  }

  process_music_sheet_event( song.events[song_pos] );

  const auto time_to_wait = static_cast<int>(song.events[song_pos].time);
  song_pos++;
  QTimer::singleShot(time_to_wait, this, SLOT(song_event_loop()));
  return;
}

void MainWindow::clear_music_scheet()
{
  stop_song();

  music_sheet_scene->clear();
  const auto nb_rendered = rendered_sheets.size();
  for (auto i = decltype(nb_rendered){0}; i < nb_rendered; ++i)
  {
    delete rendered_sheets[i].rendered;
  }
  rendered_sheets.clear();

  this->ui->start_measure->setMinimum(1);
  this->ui->start_measure->setValue(1);
  this->ui->start_measure->setMaximum(1);
  this->ui->stop_measure->setMinimum(1);
  this->ui->stop_measure->setMaximum(1);
  this->ui->stop_measure->setValue(1);

  // reinitialise the song field
  this->song = bin_song_t();
  this->song_pos = INVALID_SONG_POS;
  this->start_pos = INVALID_SONG_POS;
  this->stop_pos = INVALID_SONG_POS;

  this->update();
}

void MainWindow::pause_music()
{
  this->is_in_pause = true;

  // todo, compute the midi_messages vector at compile time
  std::vector<key_up> keys;
  constexpr const uint8_t nb_keys = static_cast<uint8_t>(note_kind::do_8) - static_cast<uint8_t>(note_kind::la_0) + 1;
  keys.reserve(nb_keys);
  for (auto key = static_cast<uint8_t>(note_kind::la_0); key <= static_cast<uint8_t>(note_kind::do_8); ++key)
  {
    keys.push_back(key_up{key});
  }

  const auto midi_messages = get_midi_from_keys_events(std::vector<key_down>(), keys);
  for (auto message : midi_messages)
  {
      sound_player.sendMessage(&message);
  }
}

void MainWindow::stop_song()
{
  pause_music();

  // reset all keys to up on the keyboard (doesn't play key_released events).
  reset_color(keyboard);
  this->update();
}

void MainWindow::replay()
{
  stop_song();
  this->start_pos = 0;
  this->stop_pos = static_cast<decltype(stop_pos)>(this->song.nb_events);
  this->song_pos = this->start_pos;
  is_in_pause = false;
}

void MainWindow::open_file(const std::string& filename)
{
  try
  {
    clear_music_scheet();
    this->song = get_song(filename);
    this->start_pos = 0;
    this->stop_pos = static_cast<decltype(stop_pos)>(this->song.nb_events);

    const auto max_measure = find_last_measure(song.events);
    this->ui->start_measure->setMinimum(1);
    this->ui->start_measure->setValue(1);
    this->ui->start_measure->setMaximum(max_measure);
    this->ui->stop_measure->setMinimum(1);
    this->ui->stop_measure->setMaximum(max_measure);
    this->ui->stop_measure->setValue(max_measure);

    // compute waiting time
    for (unsigned i = 0; i + 1 < song.nb_events; ++i)
    {
      song.events[i].time = ((song.events[i + 1].time - song.events[i].time) / 1'000'000);
    }
    if (song.nb_events > 0)
    {
      song.events[ song.nb_events - 1].time = 3000; // to wait 3 seconds after last event
    }

    this->song_pos = this->start_pos;
    sound_listener.closePort();
    this->selected_input.clear();

    const auto nb_svg = song.svg_files.size();
    if (rendered_sheets.size() != 0)
    {
	throw std::runtime_error("Invalid state detected. There should be no rendered_sheets.");
    }

    // pre-render each svg files first, so when there will be a turn page event, it is already parsed.
    for (unsigned int i = 0; i < nb_svg; ++i)
    {
      const auto& this_sheet = this->song.svg_files[i];
      const char* const sheet_data = static_cast<const char*>(static_cast<const void*>(this_sheet.data.data()));
      const auto sheet_size = this_sheet.data.size();
      const QByteArray music_sheet (sheet_data, static_cast<int>(sheet_size));

      auto current_renderer = new QSvgRenderer;
      const auto is_load_successfull = current_renderer->load(music_sheet);
      if (not is_load_successfull)
      {
	delete current_renderer;
	throw std::runtime_error("Invalid file format: failed to parse a music sheet page.");
      }

      rendered_sheets.emplace_back(sheet_property{ current_renderer,
						   get_first_svg_line(this_sheet.data) });
    }

    display_music_sheet(0);
    is_in_pause = false;
  }
  catch (std::exception& e)
  {
    // delete already rendered sheets
    const auto nb_elts = rendered_sheets.size();
    for (unsigned i = 0; i < nb_elts; ++i)
    {
      delete rendered_sheets[i].rendered;
    }
    rendered_sheets.clear();

    const auto err_msg = e.what();
    QMessageBox::critical(this, tr("Failed to open file."),
			  err_msg,
			  QMessageBox::Ok,
			  QMessageBox::Ok);
    clear_music_scheet();
  }
}

void MainWindow::open_file()
{
  const QStringList filters = [] () { QStringList tmp;
				      tmp << "Binary files (*.bin)"
				          << "Any files (*)";
				      return tmp;
				    }();

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

void MainWindow::sub_sequence_click()
{
  stop_song();

  const auto start_measure = this->ui->start_measure->value();
  const auto stop_measure = this->ui->stop_measure->value();
  const auto sequences = get_measures_sequence_pos(song,
						   static_cast<unsigned short>(start_measure),
						   static_cast<unsigned short>(stop_measure));

  if (sequences.empty())
  {
    std::cerr << "Error, no sequence going from measure " << start_measure << " to measure " << stop_measure << " found\n";
  }
  else
  {
    if (sequences.size() != 1)
    {
      std::cerr << "several possibilities found. picking first one\n";
    }

    this->start_pos = static_cast<decltype(song_pos)>(sequences[0].first);
    this->stop_pos = static_cast<decltype(song_pos)>(sequences[0].second);
    this->song_pos = this->start_pos;
    const auto music_sheet_pos = find_music_sheet_pos(song.events, song_pos);
    display_music_sheet(music_sheet_pos);
    is_in_pause = false;
  }

}

void MainWindow::set_output_port(const unsigned int i)
{
  try
  {
    sound_player.closePort();
    sound_player.openPort(i);
    const auto port_name = sound_player.getPortName(i);
    this->selected_output_port = port_name;
    this->update_output_ports();
  }
  catch (std::exception& e)
  {
    const auto err_msg = e.what();
    QMessageBox::critical(this, tr("Failed to change the output port."),
			  err_msg,
			  QMessageBox::Ok,
			  QMessageBox::Ok);

    // failed to change port, clear the selected item. There is no need to set the button
    // to unchecked as the menu is automatically closed after selecting an item, and the
    // entries are regenerated when the menu is opened again.
    this->selected_output_port.clear();

    // make sure to close all output ports
    this->sound_player.closePort();
  }
}

void MainWindow::output_port_change()
{
  // find out which output port is now checked.
  auto menu_output_port = ui->menuOutput_port;
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
  const auto nb_ports = sound_player.getPortCount();
  if (nb_ports == 0)
  {
    std::cerr << "Sorry: can't populate menu, no output midi port found\n";
    return;
  }

  // remove all the children!
  auto menu_output_port = ui->menuOutput_port;
  menu_output_port->clear();

  // find the action group.
  auto action_group = menu_output_port->findChild<QActionGroup*>("",
								 Qt::FindDirectChildrenOnly);
  if (action_group == nullptr)
  {
    action_group = new QActionGroup( menu_output_port );
  }

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

void MainWindow::handle_input_midi(const std::vector<unsigned char> message)
{
  const auto key_events = midi_to_key_events(message);
  const std::vector<midi_message_t> messages { { message } };
  this->process_keyboard_event(key_events.keys_down, key_events.keys_up, messages);
}

void MainWindow::on_midi_input(double timestamp __attribute__((unused)), std::vector<unsigned char> *message, void* param)
{
  if (message == nullptr)
  {
    throw std::invalid_argument("Error, invalid input message");
  }

  if (param == nullptr)
  {
    throw std::invalid_argument("Error, invalid argument for input listener");
  }

  auto window = static_cast<class MainWindow*>(param);
  window->handle_input_midi(*message);
  message->clear();
}

void MainWindow::set_input_port(unsigned int i)
{
  const auto port_name = sound_listener.getPortName(i);
  this->selected_input = port_name;
  sound_listener.closePort();
  sound_listener.setCallback(&MainWindow::on_midi_input, this);
  sound_listener.openPort(i);
}


void MainWindow::input_change()
{
  // find out which menu item has been clicked.
  const auto menu_input = ui->menuInput;
  const auto button_list = menu_input->findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);

  try
  {
    for (const auto& button : button_list)
    {
      if (button->isChecked())
      {
	this->clear_music_scheet();
	this->selected_input = button->text().toStdString();
	const auto nb_ports = sound_listener.getPortCount();
	for (unsigned int i = 0; i < nb_ports; ++i)
	{
	  const auto port_name = sound_listener.getPortName(i);
	  if (port_name == selected_input)
	  {
	    this->set_input_port(i);
	  }
	}
      }
    }
  }
  catch (std::exception& e)
  {
    const auto err_msg = e.what();
    QMessageBox::critical(this, tr("Failed to change the input."),
			  err_msg,
			  QMessageBox::Ok,
			  QMessageBox::Ok);

    // failed to change port, clear the selected item. There is no need to set the button
    // to unchecked as the menu is automatically closed after selecting an item, and the
    // entries are regenerated when the menu is opened again.
    this->selected_input.clear();

    // make sure to close all inputs ports
    sound_listener.closePort();
  }
}

void MainWindow::update_input_entries()
{
  // find out which input is currently selected
  auto menu_input = ui->menuInput;

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
    // add the file entry in the input menu.
    auto button = menu_input->addAction("select file");

    button->setActionGroup(action_group);
    connect(button, SIGNAL(triggered()), this, SLOT(open_file()));
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


#if !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant" // Qt is not effective-C++ friendy
#endif

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  keyboard_scene(new QGraphicsScene(this)),
  keyboard(*keyboard_scene),
  music_sheet_scene(new QGraphicsScene(this)),
  rendered_sheets(),
  current_page_viewbox(),
  cursor_rect(new QSvgRenderer(this)),
  svg_rect(nullptr),
  signal_checker_timer(),
  song(),
  sound_player(RtMidi::LINUX_ALSA),
  sound_listener(RtMidi::LINUX_ALSA),
  is_in_pause(true)
{
  ui->setupUi(this);
  ui->keyboard->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  ui->keyboard->setScene(keyboard_scene);

  ui->music_sheet->setScene(music_sheet_scene);

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
    auto menu_output_port = ui->menuOutput_port;
    connect(menu_output_port, SIGNAL(aboutToShow()), this, SLOT(update_output_ports()));
  }

  {
    // setting up the signal on_input_menu_clicked->update_input_entries. Update_input_entries fills up
    // the input menu with the available entries
    // an important difference between the input_menu and the output_menu is that the input is not necessary
    // a midi input port. It can be a file. And therefore, if there is no midi inputs detected, it's not a problem.
    // and also, there is no need to automatically select an input port.
    auto menu_input = ui->menuInput;
    connect(menu_input, SIGNAL(aboutToShow()), this, SLOT(update_input_entries()));
  }

  {
    qRegisterMetaType<std::vector<unsigned char>>("std::vector<unsigned char>");
    connect(this, SIGNAL(midi_message_received(std::vector<unsigned char>)), this, SLOT(handle_input_midi(std::vector<unsigned char>)));
  }

  {
    connect(this->ui->Playsubsequence, SIGNAL(clicked()), this, SLOT(sub_sequence_click()));
  }

  {
    connect(this->ui->replay, SIGNAL(clicked()), this, SLOT(replay()));
  }

  {
    song_event_loop();
  }
}

#if !defined(__clang__)
  #pragma GCC diagnostic pop
#endif

MainWindow::~MainWindow()
{
  clear_music_scheet();
  delete ui;
  sound_player.closePort();
}
