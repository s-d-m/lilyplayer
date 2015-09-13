#include <algorithm>
#include <iostream>
#include <cstring> // for std::memcmp and std::memset
#include <stdexcept>
#include <fstream>

#include "midi_reader.hh"

template <typename T>
static T read_big_endian(std::fstream& file)
{
  T res = 0;
  for (unsigned int i = 0; i < sizeof(T); ++i)
  {
    uint8_t tmp;
    file.read(static_cast<char*>(static_cast<void*>(&tmp)), sizeof(tmp));
    res = static_cast<decltype(res)>( (res << 8) | tmp);
  }
  return res;
};

static inline uint16_t read_big_endian16(std::fstream& file)
{
  return read_big_endian<uint16_t>(file);
}

static inline uint32_t read_big_endian32(std::fstream& file)
{
  return read_big_endian<uint32_t>(file);
}

static std::vector<uint8_t> get_variable_length_array(std::fstream& file)
{
  std::vector<uint8_t> res;
  uint8_t value;

  do
  {
    value = read_big_endian<uint8_t>(file);
    res.push_back(value);
  } while ((value & 0x80) != 0); // while the continuation bit is set.

  return res;
}

static uint64_t get_variable_length_value(const std::vector<uint8_t>& array)
{
  // recreate the right value by removing the continuation bits
  uint64_t res = 0;

  if (array.size() > sizeof(res))
  {
    throw std::invalid_argument("This program can't handle a variable length value with more than 8 bytes. Bytes used: " + std::to_string(array.size()));
  }

  const auto nb_elts = array.size();
  for (auto i = decltype(nb_elts){0}; i < nb_elts; ++i)
  {
    res |= static_cast<decltype(res)>((array[nb_elts - i - 1] & 0x7F) << 7 * i);
  }

  return res;
}

// reads a variable length value. BUT it must be four bytes maximum.
// otherwise it is not valid.
static uint32_t get_relative_time(std::fstream& file)
{
  // recreate the right value by removing the continuation bits
  const auto buffer = get_variable_length_array(file);

  if (buffer.size() > 4)
  {
    throw std::invalid_argument("Invalid relative timing found.\nMaximum size allowed is 4 bytes. Bytes used: " + std::to_string(buffer.size()));
  }

  return static_cast<uint32_t>(get_variable_length_value(buffer));
}


static bool is_header_correct(std::fstream& file, const char expected[4])
{
  char buffer[4];
  file.read(buffer, sizeof(buffer));

  return (file.gcount() == sizeof(buffer)) and
    (std::memcmp(buffer, expected, sizeof(buffer)) == 0);
}

enum MIDI_TYPE : uint8_t
{
  single_track = 0,
  multiple_track = 1,
  multiple_song = 2, // i.e. a series of type 0
};

static enum MIDI_TYPE get_midi_type(std::fstream& file)
{
   switch (read_big_endian16(file))
   {
      case 0:
	return MIDI_TYPE::single_track;
      case 1:
	return MIDI_TYPE::multiple_track;
      case 2:
	return MIDI_TYPE::multiple_song; // i.e. a series of type 0
      default:
	throw std::invalid_argument("Error: invalid MIDI file type");
   }
}

enum tempo_style : bool
{
  metrical_timing,
  timecode,
};

static uint16_t get_tickdiv(std::fstream& file, /* out param */ enum tempo_style& timing_type)
{
  // http://midi.mathewvp.com/aboutMidi.htm

  // The last two bytes indicate how many Pulses (i.e. clocks) Per Quarter Note
  // (abbreviated as PPQN) resolution the time-stamps are based upon,
  // Division. For example, if your sequencer has 96 ppqn, this field would be
  // (in hex): 00 60 Alternately, if the first byte of Division is negative,
  // then this represents the division of a second that the time-stamps are
  // based upon. The first byte will be -24, -25, -29, or -30, corresponding to
  // the 4 SMPTE standards representing frames per second. The second byte (a
  // positive number) is the resolution within a frame (ie, subframe). Typical
  // values may be 4 (MIDI Time Code), 8, 10, 80 (SMPTE bit resolution), or 100.
  // You can specify millisecond-based timing by the data bytes of -25 and 40
  // subframes.
  int8_t bytes[2];
  file.read(static_cast<char*>(static_cast<void*>(bytes)), sizeof(bytes));
  if (bytes[0] >= 0)
  {
    timing_type = tempo_style::metrical_timing;

    uint16_t res = static_cast<uint8_t>(bytes[0]);
    res = static_cast<decltype(res)>(  res << 8  );
    res = static_cast<decltype(res)>(  res | static_cast<uint8_t>(bytes[1])  );
    return res;
  }
  else
  {
    const uint8_t frames_per_sec = static_cast<decltype(frames_per_sec)>(  -(bytes[0])  );
    if ((frames_per_sec != 24) and
	(frames_per_sec != 25) and
	(frames_per_sec != 29) and
	(frames_per_sec != 30))
    {
      throw std::invalid_argument("Error: midi file contain an invalid number of frames per second");
    }

    timing_type = tempo_style::timecode;

    const auto resolution = static_cast<uint8_t>(bytes[1]);
    uint16_t res = frames_per_sec;
    res = static_cast<decltype(res)>(  res * resolution  );
    return res;
  }
}


static struct midi_event get_event(std::fstream& file, uint8_t last_status_byte)
{
  struct midi_event res;
  res.time = get_relative_time(file);

  // an event can be of three form: Control event, META event, or System
  // Exclusive event. See http://www.sonicspot.com/guide/midifiles.html
  const auto event_type = ((file.peek() & 0x80) == 0)
			  ? last_status_byte
			  : read_big_endian<uint8_t>(file);

  res.data.push_back(event_type);

  if (event_type == 0xFF)
  {
    // this is a META Event
    res.data.push_back(read_big_endian<uint8_t>(file)); // type of META event
    // for the rest, a META event is just like a sysex one.
  }

  if ((event_type == 0xFF) or (event_type == 0xF0) or (event_type == 0xF7))
  {
    // this is a sysex event, or the end of a META event.

    const auto length_array = get_variable_length_array(file);
    const auto length = get_variable_length_value(length_array);

    // Append the length array at the end of res.data
    std::move(length_array.begin(), length_array.end(), std::back_inserter(res.data));

    // the data
    for (auto i = decltype(length){0}; i < length; ++i)
    {
      res.data.push_back(read_big_endian<uint8_t>(file));
    }

    return res;
  }

  if (((event_type & 0xF0) >= 0x80) and (event_type & 0xF0) != 0xF0)
  {
    if (((event_type & 0xF0) == 0xC0)     /* Program Change Event */
	or ((event_type & 0xF0) == 0xD0)) /* or Channel Aftertouch Event */
    {
      // one more byte
      res.data.push_back(read_big_endian<uint8_t>(file));
    }
    else
    {
      // this is a MIDI channel event (more two bytes)
      res.data.push_back(read_big_endian<uint8_t>(file));
      res.data.push_back(read_big_endian<uint8_t>(file));
    }
    return res;
  }

  throw std::invalid_argument("Error: invalid type of MIDI event");
}

// read the midi events from the track and pushes them at the end of res
//
// MIDI format 1 (multiple track) can't have tempo event after the first track.
// call with the last to true when reading track 2+ from a format 1 to ensure
// validity check.
static void get_track_events(std::vector<struct midi_event>& res,
			     std::fstream& file,
			     bool fail_on_tempo_event)
{
  // http://www.ccarh.org/courses/253/handout/smf/
  //
  // A track chunk consists of a literal identifier string, a length indicator
  // specifying the size of the track, and actual event data making up the track.
  //
  //    track_chunk = "MTrk" + <length> + <track_event> [+ <track_event> ...]
  //
  // "MTrk" 4 bytes
  //     the literal string MTrk. This marks the beginning of a track.
  // <length> 4 bytes
  //     the number of bytes in the track chunk following this number.
  // <track_event>
  //     a sequenced track event.

  const char track_header[4] = { 'M', 'T', 'r', 'k' };
  if (!is_header_correct(file, track_header))
  {
    throw std::invalid_argument("Error: not a midi file (wrong track header)");
  }

  const auto track_length = read_big_endian32(file);
  const auto track_start = file.tellg();

  // reset the current time for the beginning of the track
  bool end_of_track_found = false;
  uint8_t last_status_byte = 0x00; // 0 is an invalid status byte (i.e. type of midi event)
  uint64_t this_time = 0;

  do
  {
    res.emplace_back( get_event(file, last_status_byte) );
    auto& event = res.back();
    event.time += this_time;
    this_time = event.time;

    last_status_byte = event.data[0];
    end_of_track_found = (event.data[0] == 0xff) and (event.data[1] == 0x2f);

    if ((event.data[0] == 0xff) and (event.data[1] == 0x51) // this is a tempo event
	and fail_on_tempo_event)
    {
      throw std::invalid_argument("Error: tempo event found at a forbidden place.");
    }

  }
  while (!end_of_track_found);

  const auto track_end = file.tellg();

  if ((track_end - track_start) != track_length)
  {
    throw std::invalid_argument("Error in midi file: incoherent track length detected.");
  }
}

// the time correspond to midi tics when calling the function.
// it is replaced by real time (dimension of a second)
static void set_real_timings(std::vector<struct midi_event>& events,
			     const uint16_t tickdiv,
			     const enum tempo_style timing_type)
{
  // precondition: the events must be sorted by ticks
  if (! std::is_sorted( events.begin(), events.end(), [] (const struct midi_event& a, const struct midi_event& b) {
	return a.time < b.time;
      }))
  {
    throw std::runtime_error("Error: the events are not sorted.");
  }

  switch (timing_type)
  {
    case timecode:
    {
      for (auto& ev : events)
      {
	ev.time = ev.time * tickdiv * 1000 * 1000; // nanosecond
      }
      break;
    }
    case metrical_timing:
    {
      uint64_t ref_ticks = 0;
      uint64_t ref_time = 0;

      // default tempo is 120 beats per minutes
      // 1 minute -> 60 000 000 microseconds
      // 60000000 / 120 -> 500 000 microseconds per quarter note
      uint64_t us_per_quarter_note = 500000;

      for (auto& ev : events)
      {
	const auto last_ticks = ev.time;
	const auto delta_ticks = last_ticks - ref_ticks;
	ev.time = ref_time + (delta_ticks * us_per_quarter_note * 1000 / tickdiv);

	if ((ev.data[0] == 0xff) and (ev.data[1] == 0x51))
	{
	  // this is a tempo event
	  if (ev.data.size() != 6)
	  {
	    throw std::invalid_argument("Error: tempo event has an invalid size");
	  }

	  ref_ticks = last_ticks;
	  ref_time = ev.time;

	  us_per_quarter_note = static_cast<decltype(us_per_quarter_note)>(
	    (ev.data[3] << 16) | (ev.data[4] << 8) | (ev.data[5]));
	}
      }
      break;
    }
#if !defined(__clang__)
    // clang will complain that the default case is useless because all
    // possible values in the enum are already taken into account.
    // g++ complains of a missing one
    default:
      __builtin_unreachable();
      break;
#endif
  }
}


std::vector<struct midi_event> get_midi_events(const std::string& filename)
{

  std::fstream file(filename, std::ios::binary | std::ios::in);

  if (!file.is_open())
  {
    std::string err_msg = "Error: unable to open midi file [";
    err_msg += filename;
    err_msg += "]";

    throw std::invalid_argument(err_msg);
  }

  // http://www.ccarh.org/courses/253/handout/smf/
  //
  //    header_chunk = "MThd" + <header_length> + <format> + <n> + <division>
  //
  // "MThd" 4 bytes
  //     the literal string MThd, or in hexadecimal notation: 0x4d546864. These
  //     four characters at the start of the MIDI file indicate that this is a
  //     MIDI file.
  //
  // <header_length> 4 bytes
  //     length of the header chunk (always 6 bytes long--the size of the next
  //     three fields which are considered the header chunk).
  //
  // <format> 2 bytes
  //     0 = single track file format
  //     1 = multiple track file format
  //     2 = multiple song file format (i.e., a series of type 0 files)
  //
  // <n> 2 bytes
  //     number of track chunks that follow the header chunk
  //
  // <division> 2 bytes
  //     unit of time for delta timing. If the value is positive, then it
  //     represents the units per beat. For example, +96 would mean 96 ticks
  //     per beat. If the value is negative, delta times are in SMPTE
  //     compatible units.

  const char midi_header[4] = { 'M', 'T', 'h', 'd' };
  if (!is_header_correct(file, midi_header))
  {
    throw std::invalid_argument("Error: not a midi file (wrong file header)");
  }

  // read header size
  if (read_big_endian<uint32_t>(file) != 6)
  {
    throw std::invalid_argument("Error: not a midi file (wrong header size)");
  }

  // read MIDI type
  const enum MIDI_TYPE type = get_midi_type(file);
  if (type == MIDI_TYPE::multiple_song)
  {
    throw std::invalid_argument("This program does not handle multiple song midi file - yet -");
  }

  // read number of tracks
  const auto nb_tracks = read_big_endian16(file);
  if ((type == MIDI_TYPE::single_track) and (nb_tracks != 1))
  {
    throw std::invalid_argument("Error: midi file of type \"single track\" contains several tracks");
  }

  enum tempo_style timing_type;
  // read pulses per quarter note
  const auto tickdiv = get_tickdiv(file, timing_type);
  if (tickdiv == 0)
  {
    throw std::invalid_argument("Error: a quarter note is made of 0 pulses (which is impossible) according to the midi data");
  }

  std::vector<struct midi_event> events; // the return value

  // read the tracks
  for (auto i = decltype(nb_tracks){0}; i < nb_tracks; i++)
  {
    get_track_events(events, file, (type == MIDI_TYPE::multiple_track) and (i != 0));
  }

  // sanity check: the midi file should have been entirely read by now (no more
  // remaining bytes)
  file.peek(); // just to set the eof bit.
  if (not file.eof())
  {
    throw std::invalid_argument("Error: invalid midi file (extra bytes after end of MIDI data)");
  }

  // sort the events by time
  std::stable_sort(events.begin(), events.end(), [] (const struct midi_event& a, const struct midi_event& b) {
      return a.time < b.time;
    });

  set_real_timings(events, tickdiv, timing_type);

  // only keep MIDI events (filter out sysex and meta events)
  decltype(events) res;
  for (const auto& ev : events)
  {
    if ((ev.data[0] & 0xF0) != 0xF0)
    {
      res.emplace_back(ev);
    }
  }

  return res;
}
