#include <iostream>
#include <QFileDialog>


#include "mainwindow.hh"
#include "ui_mainwindow.hh"

static constexpr qreal WHITE_KEY_HEIGHT {120.0};
static constexpr qreal WHITE_KEY_WIDTH  {23.0};
static constexpr qreal BLACK_KEY_HEIGHT {80.0};
static constexpr qreal BLACK_KEY_WIDTH  {13.0};


static
void draw_piano_key(QGraphicsScene& scene, qreal x, qreal y, qreal width, qreal height, QColor color)
{
  auto rect = scene.addRect(x, y, width, height);
  rect->setBrush(color);
}

static inline
void draw_white_key(QGraphicsScene& scene, qreal x, qreal y, QColor color)
{
  draw_piano_key(scene, x, y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, color);
}

static inline
void draw_black_key(QGraphicsScene& scene, qreal x, qreal y, QColor color)
{
  draw_piano_key(scene, x, y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, color);
}

static
void draw_octave(QGraphicsScene& scene, qreal x, qreal y, const struct octave_color& notes_color)
{
  draw_white_key(scene, x,                         y, notes_color.do_color);  // do
  draw_white_key(scene, x +      WHITE_KEY_WIDTH,  y, notes_color.re_color);  // re
  draw_white_key(scene, x + (2 * WHITE_KEY_WIDTH), y, notes_color.mi_color);  // mi

  draw_white_key(scene, x + (3 * WHITE_KEY_WIDTH), y, notes_color.fa_color); // fa
  draw_white_key(scene, x + (4 * WHITE_KEY_WIDTH), y, notes_color.sol_color); // sol
  draw_white_key(scene, x + (5 * WHITE_KEY_WIDTH), y, notes_color.la_color); // la
  draw_white_key(scene, x + (6 * WHITE_KEY_WIDTH), y, notes_color.si_color); // si

  draw_black_key(scene, x + (qreal{43} / qreal{3}), y, notes_color.do_diese_color);  // do#
  draw_black_key(scene, x + (qreal{125} / qreal{3}), y, notes_color.re_diese_color);  // re#

  draw_black_key(scene, x + (qreal{329} / qreal{4}), y, notes_color.fa_diese_color); // fa#
  draw_black_key(scene, x + (qreal{433} / qreal{4}), y, notes_color.sol_diese_color); // sol#
  draw_black_key(scene, x + (qreal{539} / qreal{4}), y, notes_color.la_diese_color); // la#
}

static void draw_keyboard(QGraphicsScene& scene, const struct keys_color& keyboard)
{
  scene.clear();

  constexpr qreal pos_x {0};
  constexpr qreal pos_y {0};

  draw_white_key(scene, pos_x, pos_y, keyboard.la_0_color); // la 0
  draw_white_key(scene, pos_x + WHITE_KEY_WIDTH, pos_y, keyboard.si_0_color); // si 0
  draw_black_key(scene, pos_x + (qreal{33} / qreal{2}), pos_y, keyboard.la_diese_0_color); // la# 0

  constexpr qreal octave_width { qreal{7} * WHITE_KEY_WIDTH };

  for (int i = 0; i < 7; ++i)
  {
    draw_octave(scene, pos_x + (qreal{2} * WHITE_KEY_WIDTH) + (octave_width * i), pos_y, keyboard.octaves[i]);
  }

  draw_white_key(scene, pos_x + (qreal{2} * WHITE_KEY_WIDTH) + (octave_width * qreal{7}), pos_y, keyboard.do_8_color); // do 8
}


#define OCTAVE_COLOR(X)						\
  case note_kind::do_##X:					\
  keyboard.octaves[(X - 1)].do_color = normal_key_color;	\
  break;							\
  case note_kind::do_diese##X:					\
  keyboard.octaves[(X - 1)].do_diese_color = diese_key_color;	\
  break;							\
  case note_kind::re_##X:					\
  keyboard.octaves[(X - 1)].re_color = normal_key_color;	\
  break;							\
  case note_kind::re_diese_##X:					\
  keyboard.octaves[(X - 1)].re_diese_color = diese_key_color;	\
  break;							\
  case note_kind::mi_##X:					\
  keyboard.octaves[(X - 1)].mi_color = normal_key_color;	\
  break;							\
  case note_kind::fa_##X:					\
  keyboard.octaves[(X - 1)].fa_color = normal_key_color;	\
  break;							\
  case note_kind::fa_diese_##X:					\
  keyboard.octaves[(X - 1)].fa_diese_color = diese_key_color;	\
  break;							\
  case note_kind::sol_##X:					\
  keyboard.octaves[(X - 1)].sol_color = normal_key_color;	\
  break;							\
  case note_kind::sol_diese_##X:				\
  keyboard.octaves[(X - 1)].sol_diese_color = diese_key_color;	\
  break;							\
  case note_kind::la_##X:					\
  keyboard.octaves[(X - 1)].la_color = normal_key_color;	\
  break;							\
  case note_kind::la_diese_##X:					\
  keyboard.octaves[(X - 1)].la_diese_color = diese_key_color;	\
  break;							\
  case note_kind::si_##X:					\
  keyboard.octaves[(X - 1)].si_color = normal_key_color;	\
  break								\


#if defined(__clang__)
  // clang will complain in a switch that the default case is useless
  // because all possible values in the enum are already taken into
  // account.  however, since the value can come from an unrestricted
  // uint8_t, the default is actually a necessary safe-guard.
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

static void set_color(struct keys_color& keyboard, enum note_kind note, QColor normal_key_color, QColor diese_key_color)
{
  switch (note)
  {
    case note_kind::la_0:
      keyboard.la_0_color = normal_key_color;
      break;

    case note_kind::la_diese_0:
      keyboard.la_diese_0_color = diese_key_color;
      break;

    case note_kind::si_0:
      keyboard.si_0_color = normal_key_color;
      break;

      OCTAVE_COLOR(1);
      OCTAVE_COLOR(2);
      OCTAVE_COLOR(3);
      OCTAVE_COLOR(4);
      OCTAVE_COLOR(5);
      OCTAVE_COLOR(6);
      OCTAVE_COLOR(7);

    case note_kind::do_8:
      keyboard.do_8_color = normal_key_color;
      break;

    default:
      std::cerr << "Warning key " << static_cast<long unsigned int>(note)
		<< " is not representable in a 88 key piano" << std::endl;
      break;
  }
}
#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#undef OCTAVE_COLOR

static void reset_color(struct keys_color& keyboard, enum note_kind note)
{
  set_color(keyboard, note, Qt::white, Qt::black);
}

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
