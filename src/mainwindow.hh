#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>

#include <QGraphicsView>
#include <QGraphicsItem>

#include <QTimer>

#include <limits>

#include <RtMidi.h>

#include "keyboard.hh"
#include "midi_reader.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++" // Qt is not effective-C++ friendy



namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void open_file(const std::string& filename);
    void set_output_port(const unsigned int i);
    void set_input_port(const unsigned int i);

  private:
    void stop_song();
    void process_keyboard_event(const music_event& keys_event);
    void keyPressEvent(QKeyEvent * event);
    static void on_midi_input(double timestamp __attribute__((unused)), std::vector<unsigned char> *message, void* param);

  signals:
    void midi_message_received(std::vector<unsigned char> bytes);

  private slots:
    void song_event_loop();
    void open_file(); // open the window dialog to select a file
    void look_for_signals_change();
    void output_port_change();
    void update_output_ports();
    void update_input_entries();
    void input_change();
    void handle_input_midi(std::vector<unsigned char> bytes);

  private:
    static constexpr unsigned int INVALID_SONG_POS = std::numeric_limits<unsigned int>::max();

    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    struct keys_color keyboard;
    QTimer signal_checker_timer;
    song_t song;
    RtMidiOut sound_player;
    RtMidiIn  sound_listener;
    std::string selected_output_port = "";
    std::string selected_input = "";
    unsigned int song_pos = INVALID_SONG_POS;
    bool is_in_pause = false;
};

#pragma GCC diagnostic pop

#endif // MAINWINDOW_HH
