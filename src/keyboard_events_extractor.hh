#ifndef KEYBOARD_EVENTS_EXTRACTOR_HH_
#define KEYBOARD_EVENTS_EXTRACTOR_HH_

#include <vector>
#include "midi_reader.hh"

struct key_data
{
    enum type : bool
    {
      pressed,
      released,
    };

    uint8_t  pitch; // the key that is pressed or released
    type     ev_type; // was the key pressed or released?

    key_data(decltype(key_data::pitch) init_pitch,
	     decltype(key_data::ev_type) init_type)
      : pitch(init_pitch)
      , ev_type(init_type)
    {
    }

};

struct key_event
{
    uint64_t time; // the time the event occurs during the sond
    key_data data;

    key_event(const key_event& other) = default;
    key_event(key_event&& other) = default;

    key_event(decltype(key_event::time) init_time,
	      decltype(key_data::pitch) init_pitch,
	      decltype(key_data::ev_type) init_type)
      : time(init_time)
      , data(key_data(init_pitch, init_type))
    {
    }
};

// extracts the key_pressed / key_released from the midi events
std::vector<struct key_event>
get_key_events(const std::vector<struct midi_event>& midi_events);


#endif /* KEYBOARD_EVENTS_EXTRACTOR_HH_ */
