#ifndef KEYBOARD_HH
#define KEYBOARD_HH

#include <QColor>


#define OCTAVE(X) \
  do_##X,	  \
  do_diese##X,	  \
  re_##X,	  \
  re_diese_##X,   \
  mi_##X,	  \
  fa_##X,	  \
  fa_diese_##X,   \
  sol_##X,	  \
  sol_diese_##X,  \
  la_##X,	  \
  la_diese_##X,	  \
  si_##X	  \

enum note_kind : uint8_t
{
  /* scale 0 */
  la_0 = 21,
  la_diese_0,
  si_0,

  OCTAVE(1),
  OCTAVE(2),
  OCTAVE(3),
  OCTAVE(4),
  OCTAVE(5),
  OCTAVE(6),
  OCTAVE(7),

  /* ninth scale */
  do_8,
};

#undef OCTAVE


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

#endif
