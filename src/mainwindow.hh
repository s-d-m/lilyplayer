#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>

#include <QGraphicsView>
#include <QGraphicsItem>

#include <QTimer>

#include <limits>


extern "C" {
#include <RtMidi.h>
}

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

  private:
    void stop_song();
    void process_keyboard_event(const music_event& keys_event);

  private slots:
    void song_event_loop();
    void open_file();
    void look_for_signals_change();

  private:
    static constexpr unsigned int INVALID_SONG_POS = std::numeric_limits<unsigned int>::max();

    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    struct keys_color keyboard;
    QTimer signal_checker_timer;
    song_t song;
    RtMidiOut sound_player;
    unsigned int song_pos = INVALID_SONG_POS;
    bool is_in_pause = false;
};

#pragma GCC diagnostic pop

#endif // MAINWINDOW_HH
