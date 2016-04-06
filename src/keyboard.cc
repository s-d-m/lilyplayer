#include <algorithm> // for min
#include <iostream>
#include <QGraphicsRectItem>

#include "keyboard.hh"



void set_color(struct keys_rects& keyboard, enum note_kind note, const QColor& normal_key_color, const QColor& diese_key_color)
{
  if ((static_cast<uint8_t>(note) < static_cast<uint8_t>(note_kind::la_0)) or
      (static_cast<uint8_t>(note) > static_cast<uint8_t>(note_kind::do_8)))
  {
    std::cerr << "Warning key " << static_cast<long unsigned int>(note)
	      << " is not representable in a 88 key piano" << std::endl;
    return;
  }

#define OCTAVE_COLOR(X)		\
  true,  /* do_##X */		\
  false, /* do_diese_##X */	\
  true,  /* re_##X */		\
  false, /* re_diese_##X */	\
  true,  /* mi_##X */		\
  true,  /* fa_##X */		\
  false, /* fa_diese_##X */	\
  true,  /* sol_##X */		\
  false, /* sol_diese_##X */	\
  true,  /* la_##X */		\
  false, /* la_diese_##X */	\
  true   /* si_##X */

  static constexpr const bool is_normal_keys[static_cast<uint8_t>(note_kind::do_8) - static_cast<uint8_t>(note_kind::la_0) + 1] = {
    true, // la_0
    false, // la_diese_0
    true, //si_0
    OCTAVE_COLOR(1),
    OCTAVE_COLOR(2),
    OCTAVE_COLOR(3),
    OCTAVE_COLOR(4),
    OCTAVE_COLOR(5),
    OCTAVE_COLOR(6),
    OCTAVE_COLOR(7),
    true // do_8
  };

#undef OCTAVE_COLOR

  const auto pos = static_cast<uint8_t>(note) - static_cast<uint8_t>(note_kind::la_0);
  const auto is_normal_key = is_normal_keys[pos];
  const auto &key_color = is_normal_key ? normal_key_color : diese_key_color;
  keyboard.keys[pos]->setBrush(key_color);
}

void reset_color(struct keys_rects& keyboard, enum note_kind note)
{
  set_color(keyboard, note, Qt::white, Qt::black);
}

void reset_color(struct keys_rects& keyboard)
{
  for (auto key = static_cast<uint8_t>(note_kind::la_0);
       key <= static_cast<uint8_t>(note_kind::do_8);
       ++key)
  {
    reset_color(keyboard, static_cast<note_kind>(key));
  }
}


#pragma GCC diagnostic push
#if !defined(__clang__)
// keyboard.cc: In function ‘void update_keyboard(const std::vector<key_down>&, const std::vector<key_up>&, keys_color&)’:
// keyboard.cc:188:3: error: assuming that the loop counter does not overflow [-Werror=unsafe-loop-optimizations]
//    for (const auto& key : keys_down)
//    ^
  #pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
#endif

void update_keyboard(const std::vector<key_down>& keys_down,
		     const std::vector<key_up>& keys_up,
		     struct keys_rects& keyboard)
{
  static const QColor white_key_colors[] = { Qt::blue, Qt::red,     Qt::green,  Qt::gray };
  static const QColor black_key_colors[] = { Qt::cyan, Qt::magenta, Qt::yellow, Qt::darkYellow };

  constexpr const uint8_t nb_colors = sizeof(white_key_colors) / sizeof(white_key_colors[0]);

  /* for each key pressed */
  for (const auto& key : keys_down)
  {
    const auto color_pos = std::min(key.staff_num, static_cast<uint8_t>(nb_colors - 1));
    const QColor& white_keys_color = white_key_colors[color_pos];
    const QColor& black_keys_color = black_key_colors[color_pos];

    set_color(keyboard, static_cast<enum note_kind>(key.pitch),
	      white_keys_color, black_keys_color);
  }

  /* for each key released */
  for (const auto& key : keys_up)
  {
    reset_color(keyboard, static_cast<enum note_kind>(key.pitch));
  }
}

#pragma GCC diagnostic pop
