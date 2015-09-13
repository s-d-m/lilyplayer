#ifndef MIDI_READER_HH_
#define MIDI_READER_HH_

#include <vector>
#include <string>
#include <limits>

struct midi_event
{
    uint64_t		 time; // nanosec
    std::vector<uint8_t> data;

    midi_event()
      : time (std::numeric_limits<decltype(time)>::max())
      , data ()
    {
    }
};

std::vector<struct midi_event>
get_midi_events(const std::string& filename);

#endif /* MIDI_READER_HH_ */
