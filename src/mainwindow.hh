#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QGraphicsScene>

#include <QGraphicsView>
#include <QGraphicsItem>

#include <QTimer>

#include <limits>

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

  private slots:
    void process_keyboard_event();
    void open_file();
    void look_for_signals_change();

  private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    struct keys_color keyboard;
    QTimer timer;
    std::vector<struct midi_event> song;
    unsigned int song_pos = std::numeric_limits<unsigned int>::max();
    bool is_in_pause = false;
};

#pragma GCC diagnostic pop

#endif // MAINWINDOW_HH
