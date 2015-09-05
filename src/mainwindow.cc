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

  draw_white_key(scene, pos_x + 1, pos_y, keyboard.la_0_color); // la 0
  draw_white_key(scene, pos_x + 1 + WHITE_KEY_WIDTH, pos_y, keyboard.si_0_color); // si 0
  draw_black_key(scene, pos_x + (qreal{33} / qreal{2}), pos_y, keyboard.la_diese_0_color); // la# 0

  constexpr qreal octave_width { qreal{7} * WHITE_KEY_WIDTH };

  for (int i = 0; i < 7; ++i)
  {
    draw_octave(scene, pos_x + (qreal{2} * WHITE_KEY_WIDTH) + (octave_width * i), pos_y, keyboard.octaves[i]);
  }

  draw_white_key(scene, pos_x + (qreal{2} * WHITE_KEY_WIDTH) + (octave_width * qreal{7}), pos_y, keyboard.do_8_color); // do 8
}


#pragma GCC diagnostic push
#if !defined(__clang__)
  #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant" // Qt is not effective-C++ friendy
#endif

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  scene(new QGraphicsScene(this)),
  keyboard()
{
  ui->setupUi(this);
  ui->keyboard->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  ui->keyboard->setScene(scene);
  draw_keyboard(*scene, this->keyboard);
}

#pragma GCC diagnostic pop

MainWindow::~MainWindow()
{
  delete ui;
}
