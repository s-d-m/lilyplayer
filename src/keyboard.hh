#ifndef KEYBOARD_HH
#define KEYBOARD_HH

#include <QColor>
#include <QGraphicsScene>

#include "utils.hh"


struct octave_color
{
    octave_color()
      : do_color (Qt::white)
      , re_color (Qt::white)
      , mi_color (Qt::white)
      , fa_color (Qt::white)
      , sol_color (Qt::white)
      , la_color (Qt::white)
      , si_color (Qt::white)

      , do_diese_color (Qt::black)
      , re_diese_color (Qt::black)
      , fa_diese_color (Qt::black)
      , sol_diese_color (Qt::black)
      , la_diese_color (Qt::black)
    {
    }

    QColor do_color;
    QColor re_color;
    QColor mi_color;
    QColor fa_color;
    QColor sol_color;
    QColor la_color;
    QColor si_color;

    QColor do_diese_color;
    QColor re_diese_color;
    QColor fa_diese_color;
    QColor sol_diese_color;
    QColor la_diese_color;
};


struct keys_color
{
    keys_color()
      : la_0_color (Qt::white)
      , si_0_color (Qt::white)
      , la_diese_0_color (Qt::black)
      ,	octaves ()
      , do_8_color (Qt::white)
    {
    }

    QColor la_0_color;
    QColor si_0_color;
    QColor la_diese_0_color;
    struct octave_color octaves[7];
    QColor do_8_color;
};

void draw_keyboard(QGraphicsScene& scene, const struct keys_color& keyboard);
void reset_color(struct keys_color& keyboard, enum note_kind note);
void reset_color(struct keys_color& keyboard); // reset all keys
void set_color(struct keys_color& keyboard, enum note_kind note, const QColor& normal_key_color, const QColor& diese_key_color);
void update_keyboard(const std::vector<key_down>& keys_down,
		     const std::vector<key_up>& keys_up,
		     struct keys_color& keyboard);

#endif
