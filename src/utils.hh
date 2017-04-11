#ifndef UTILS_HH_
#define UTILS_HH_

#include <vector>
#include <limits>
#include <fstream>
#include <string>
#include <rtmidi/RtMidi.h>

struct key_down
{
    key_down(uint8_t _pitch, uint8_t _staff_num)
      : pitch(_pitch)
      , staff_num(_staff_num)
    {
    }

    uint8_t pitch;
    uint8_t staff_num;
};

struct key_up
{
    explicit key_up(uint8_t _pitch)
      : pitch(_pitch)
    {
    }

    uint8_t pitch;
};

#define OCTAVE(X) \
  do_##X,	  \
  do_diese_##X,	  \
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


using midi_message_t = std::vector<uint8_t>;

bool is_key_down_event(const std::vector<uint8_t>& data) __attribute__((pure));
bool is_key_release_event(const std::vector<uint8_t>& data) __attribute__((pure));

struct key_events
{
    key_events()
      : keys_down()
      , keys_up()
    {
    }

    std::vector<key_down> keys_down;
    std::vector<key_up> keys_up;
};

key_events
midi_to_key_events(const std::vector<uint8_t>& message_stream) __attribute__((pure));

std::vector<midi_message_t>
get_midi_from_keys_events(const std::vector<key_down>& keys_down,
			  const std::vector<key_up>& keys_up);


void list_midi_ports(std::ostream& out);
unsigned int get_port(const std::string& s);

std::string get_first_svg_line(const std::vector<uint8_t>& data);

struct music_sheet_event;
uint16_t find_last_measure(const std::vector<music_sheet_event>& events);
uint16_t find_music_sheet_pos(const std::vector<music_sheet_event>& events, unsigned int event_pos);


const char* rt_error_type_as_str(RtMidiError::Type value);

std::vector<std::string> get_input_midi_ports_name(RtMidiIn& midi_listener);
std::vector<std::string> get_output_midi_ports_name(RtMidiOut& midi_player);


#endif /* UTILS_HH_ */
