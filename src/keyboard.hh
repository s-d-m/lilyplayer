#ifndef KEYBOARD_HH
#define KEYBOARD_HH

#include <QColor>
#include <QGraphicsScene>
#include <QGraphicsRectItem>

#include "utils.hh"


static constexpr qreal WHITE_KEY_HEIGHT {120.0};
static constexpr qreal WHITE_KEY_WIDTH  {23.0};
static constexpr qreal BLACK_KEY_HEIGHT {80.0};
static constexpr qreal BLACK_KEY_WIDTH  {13.0};

static constexpr qreal OCTAVE_WIDTH { qreal{7} * WHITE_KEY_WIDTH };

struct keys_rects
{

#define OCTAVE(num_octave)			       \
  keys[note_kind::do_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH),                                y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::do_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::re_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) +             WHITE_KEY_WIDTH,  y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::re_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::mi_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{2} * WHITE_KEY_WIDTH), y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::mi_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::fa_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{3} * WHITE_KEY_WIDTH), y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::fa_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::sol_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{4} * WHITE_KEY_WIDTH), y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::sol_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::la_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{5} * WHITE_KEY_WIDTH), y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::la_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::si_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{6} * WHITE_KEY_WIDTH), y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
  keys[note_kind::si_##num_octave - note_kind::la_0]->setBrush(Qt::white); \
  \
  keys[note_kind::do_diese_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{43} / qreal{3}), y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT); \
  keys[note_kind::do_diese_##num_octave - note_kind::la_0]->setBrush(Qt::black); \
  \
  keys[note_kind::re_diese_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{125} / qreal{3}), y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT); \
  keys[note_kind::re_diese_##num_octave - note_kind::la_0]->setBrush(Qt::black); \
  \
  keys[note_kind::fa_diese_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{329} / qreal{4}), y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT); \
  keys[note_kind::fa_diese_##num_octave - note_kind::la_0]->setBrush(Qt::black); \
  \
  keys[note_kind::sol_diese_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{433} / qreal{4}), y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT); \
  keys[note_kind::sol_diese_##num_octave - note_kind::la_0]->setBrush(Qt::black); \
  \
  keys[note_kind::la_diese_##num_octave - note_kind::la_0] = \
    scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{num_octave - 1} * OCTAVE_WIDTH) + (qreal{539} / qreal{4}), y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT); \
  keys[note_kind::la_diese_##num_octave - note_kind::la_0]->setBrush(Qt::black);

    keys_rects(QGraphicsScene& scene, const qreal x = 0, const qreal y = 0)
    {
      keys[note_kind::la_0 - note_kind::la_0] =
	scene.addRect(x, y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
      keys[note_kind::la_0 - note_kind::la_0]->setBrush(Qt::white);

      keys[note_kind::si_0 - note_kind::la_0] =
	scene.addRect(x + WHITE_KEY_WIDTH, y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
      keys[note_kind::si_0 - note_kind::la_0]->setBrush(Qt::white);

      keys[note_kind::la_diese_0 - note_kind::la_0] =
	scene.addRect(x + (qreal{33} / qreal{2}), y, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT);
      keys[note_kind::la_diese_0 - note_kind::la_0]->setBrush(Qt::black);

      OCTAVE(1);
      OCTAVE(2);
      OCTAVE(3);
      OCTAVE(4);
      OCTAVE(5);
      OCTAVE(6);
      OCTAVE(7);

      keys[note_kind::do_8 - note_kind::la_0] =
	scene.addRect(x + (qreal{2} * WHITE_KEY_WIDTH) + (qreal{7} * OCTAVE_WIDTH), y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT); \
      keys[note_kind::do_8 - note_kind::la_0]->setBrush(Qt::white);
    }

#undef OCTAVE

    QGraphicsRectItem* keys[note_kind::do_8 - note_kind::la_0 + 1];
};



void reset_color(struct keys_rects& keyboard, enum note_kind note);
void reset_color(struct keys_rects& keyboard); // reset all keys
void set_color(struct keys_rects& keyboard, enum note_kind note, const QColor& normal_key_color, const QColor& diese_key_color);
void update_keyboard(const std::vector<key_down>& keys_down,
		     const std::vector<key_up>& keys_up,
		     struct keys_rects& keyboard);

#endif
