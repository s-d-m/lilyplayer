#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QSvgRenderer>

#include <QTimer>

#include <limits>
#include <atomic>

#include <rtmidi/RtMidi.h>

#include "utils.hh"
#include "keyboard.hh"
#include "bin_file_reader.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++" // Qt is not effective-C++ friendy



namespace Ui {
  class MainWindow;
}

class QGraphicsSvgItem;
class QGraphicsRectItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void open_file(const std::string& filename);
    void set_output_port(const unsigned int i);
    void set_input_port(const unsigned int i);

  private:
    void pause_music();
    void stop_song();
    void close_input_port();
    void clear_music_scheet();
    void process_music_sheet_event(const music_sheet_event& keys_event);
    void display_music_sheet(const unsigned music_sheet_pos);
    void keyPressEvent(QKeyEvent * event) override;
    static void on_midi_input(double timestamp __attribute__((unused)), std::vector<unsigned char> *message, void* param);
    static void on_midi_error(RtMidiError::Type type, const std::string &errorText, const char* const direction);
    static void on_midi_input_error(RtMidiError::Type type, const std::string &errorText, void* param __attribute__((unused)));
    static void on_midi_output_error(RtMidiError::Type type, const std::string &errorText, void* param __attribute__((unused)));

    void process_keyboard_event(const std::vector<key_down>& keys_down,
				const std::vector<key_up>& keys_up,
				const std::vector<midi_message_t>& messages);


  signals:
    void midi_message_received(std::vector<unsigned char> bytes);

  private slots:
    void song_event_loop();
    void replay();
    void open_file(); // open the window dialog to select a file
    void look_for_signals_change();
    void output_port_change();
    void update_output_ports();
    void update_input_entries();
    void input_change();
    void handle_input_midi(const std::vector<unsigned char> bytes);
    void sub_sequence_click();

  private:
    static constexpr const unsigned int INVALID_SONG_POS = std::numeric_limits<unsigned int>::max();

  private:
    struct sheet_property
    {
	QSvgRenderer* rendered;
	std::string svg_first_line;
    };

    Ui::MainWindow *ui;
    QGraphicsScene *keyboard_scene;
    keys_rects keyboard;
    QGraphicsScene *music_sheet_scene;
    std::vector<sheet_property> rendered_sheets;
    QRectF current_page_viewbox;
    QSvgRenderer* cursor_rect;
    QGraphicsSvgItem* svg_rect;
    QTimer signal_checker_timer;
    bin_song_t song;
    RtMidiOut sound_player;
    RtMidiIn  sound_listener;
    std::string selected_output_port = "";
    std::string selected_input_port = "";


    unsigned int start_pos = INVALID_SONG_POS;
    unsigned int stop_pos = INVALID_SONG_POS;
    unsigned int song_pos = INVALID_SONG_POS;
    std::atomic<bool> is_in_pause;
};

#pragma GCC diagnostic pop

#endif // MAINWINDOW_HH
