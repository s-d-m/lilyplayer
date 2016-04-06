#include <algorithm> // for min
#include <iostream>
#include <QGraphicsRectItem>

#include "keyboard.hh"


#define OCTAVE_COLOR(X)							\
  case note_kind::do_##X:						\
  keyboard.keys[note_kind::do_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break;								\
  case note_kind::do_diese_##X:						\
  keyboard.keys[note_kind::do_diese_##X - note_kind::la_0]->setBrush(diese_key_color); \
  break;							\
  case note_kind::re_##X:					\
  keyboard.keys[note_kind::re_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break;								\
  case note_kind::re_diese_##X:					\
  keyboard.keys[note_kind::re_diese_##X - note_kind::la_0]->setBrush(diese_key_color); \
  break;							\
  case note_kind::mi_##X:					\
  keyboard.keys[note_kind::mi_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break;							\
  case note_kind::fa_##X:					\
  keyboard.keys[note_kind::fa_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break;							\
  case note_kind::fa_diese_##X:					\
  keyboard.keys[note_kind::fa_diese_##X - note_kind::la_0]->setBrush(diese_key_color); \
  break;							\
  case note_kind::sol_##X:					\
  keyboard.keys[note_kind::sol_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break;							\
  case note_kind::sol_diese_##X:				\
  keyboard.keys[note_kind::sol_diese_##X - note_kind::la_0]->setBrush(diese_key_color); \
  break;							\
  case note_kind::la_##X:					\
  keyboard.keys[note_kind::la_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break;							\
  case note_kind::la_diese_##X:					\
  keyboard.keys[note_kind::la_diese_##X - note_kind::la_0]->setBrush(diese_key_color); \
  break;							\
  case note_kind::si_##X:					\
  keyboard.keys[note_kind::si_##X - note_kind::la_0]->setBrush(normal_key_color); \
  break								\


#if defined(__clang__)
  // clang will complain in a switch that the default case is useless
  // because all possible values in the enum are already taken into
  // account.  however, since the value can come from an unrestricted
  // uint8_t, the default is actually a necessary safe-guard.
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

void set_color(struct keys_rects& keyboard, enum note_kind note, const QColor& normal_key_color, const QColor& diese_key_color)
{
  switch (note)
  {
    case note_kind::la_0:
      keyboard.keys[note_kind::la_0 - note_kind::la_0]->setBrush(normal_key_color);
      break;

    case note_kind::la_diese_0:
      keyboard.keys[note_kind::la_diese_0 - note_kind::la_0]->setBrush(diese_key_color);
      break;

    case note_kind::si_0:
      keyboard.keys[note_kind::si_0 - note_kind::la_0]->setBrush(normal_key_color);
      break;

      OCTAVE_COLOR(1);
      OCTAVE_COLOR(2);
      OCTAVE_COLOR(3);
      OCTAVE_COLOR(4);
      OCTAVE_COLOR(5);
      OCTAVE_COLOR(6);
      OCTAVE_COLOR(7);

    case note_kind::do_8:
      keyboard.keys[note_kind::do_8 - note_kind::la_0]->setBrush(normal_key_color);
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
    set_color(keyboard, static_cast<note_kind>(key), Qt::white, Qt::black);
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
