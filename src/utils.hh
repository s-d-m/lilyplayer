#ifndef UTILS_HH_
#define UTILS_HH_

#include <vector>
#include <limits>
#include <fstream>
#include "midi_reader.hh"
#include "keyboard_events_extractor.hh"


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


using midi_message = std::vector<uint8_t>;

struct music_event
{
    uint64_t time; // occuring time
    std::vector<midi_message> midi_messages;
    std::vector<struct key_data> key_events;

    music_event()
      : time (std::numeric_limits<decltype(time)>::max())
      , midi_messages ()
      , key_events ()
    {
    }

    music_event(decltype(music_event::time) init_time,
		decltype(music_event::midi_messages) init_midi_messages,
		decltype(music_event::key_events) init_key_events)
      : time (init_time)
      , midi_messages (init_midi_messages)
      , key_events (init_key_events)
    {
    }
};

// a song is just a succession of music_event to be played
using song_t = std::vector<struct music_event>;

song_t group_events_by_time(const std::vector<struct midi_event>& midi_events,
			    const std::vector<struct key_event>& key_events);


bool is_key_down_event(const struct midi_event& ev) __attribute__((pure));
bool is_key_release_event(const struct midi_event& ev) __attribute__((pure));
bool is_key_down_event(const std::vector<uint8_t>& data) __attribute__((pure));
bool is_key_release_event(const std::vector<uint8_t>& data) __attribute__((pure));

std::vector<struct key_data>
midi_to_key_events(const std::vector<uint8_t>& message_stream);

void list_midi_ports(std::ostream& out);
unsigned int get_port(const std::string& s);

#endif /* UTILS_HH_ */
