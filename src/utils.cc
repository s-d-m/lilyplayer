#include <algorithm>
#include <stdexcept>
#include <string>
#include <cstddef> // for std::size_t
#include <rtmidi/RtMidi.h>
#include "utils.hh"
#include "bin_file_reader.hh"

bool is_key_down_event(const std::vector<uint8_t>& data)
{
  return (data.size() == 3) and
         ((data[0] & 0xF0) == 0x90) and (data[2] != 0x00);
}

bool is_key_release_event(const std::vector<uint8_t>& data)
{
  return (data.size() == 3) and
    (((data[0] & 0xF0) == 0x80) or
     (((data[0] & 0xF0) == 0x90) and (data[2] == 0x00)));
}



static
std::size_t get_variable_data_length(const std::vector<uint8_t>& message_stream, std::size_t start_pos)
{
  const auto max_res = message_stream.size() - start_pos;
  const auto end = message_stream.end();
  const auto begin = message_stream.begin();
  auto array_size = decltype(max_res){0};
  auto nb_read = decltype(array_size){0};

  for (auto it = std::next(begin, static_cast<int>(start_pos)); it != end; ++it)
  {
    nb_read++;
    array_size = (array_size << 7) + ((*it) & 0x7F);
    if (((*it) & 0x80) == 0)
    {
      return std::min(max_res, array_size + nb_read);
    }
  }

  // at this point input is invalid
  return max_res;
}

static std::size_t get_next_event_size(const std::vector<uint8_t>& message_stream, std::size_t start_pos)
{
  const auto stream_size = message_stream.size() - start_pos;
  const auto stream = std::next(message_stream.begin(), static_cast<int>(start_pos));

  if (stream_size < 2)
  {
    // minimal possible length (midi channel event 0xC0 and 0xD0
    return stream_size;
  }

  std::size_t res = 1; // the first byte which the event type (channel, meta, sysex)
  const auto ev_type = stream[0];
  if (ev_type == 0xFF)
  {
    // META event has one byte more than sysex
    res += 1;
  }

  if ((ev_type == 0xFF) or (ev_type == 0xF0) or (ev_type == 0xF7))
  {
    // end of META or sysex
    res += get_variable_data_length(message_stream, res + start_pos);

    // sanity check: in case of wrong input, simply discard data
    return std::min(res, stream_size);
  }

  if (((ev_type & 0xF0) >= 0x80) and (ev_type & 0xF0) != 0xF0)
  {
    if (((ev_type & 0xF0) == 0xC0)     /* Program Change Event */
	or ((ev_type & 0xF0) == 0xD0)) /* or Channel Aftertouch Event */
    {
      // one more byte
      res += 1;
    }
    else
    {
      // this is a MIDI channel event (more two bytes)
      res += 2;
    }
    return std::min(res, stream_size);
  }

  // at this point there is an error, discard data
  return stream_size;
}

key_events
midi_to_key_events(const std::vector<uint8_t>& message_stream)
{
  key_events res;

  const auto size = message_stream.size();
  auto nb_read = decltype(size){0};
  const auto stream_begin = message_stream.begin();

  while (nb_read < size)
  {
    const auto this_event_size = get_next_event_size(message_stream, nb_read);

    if (this_event_size == 0)
    {
      // this is an error. return what we have found so far to avoid
      // an infinite loop. This can happen with variable length array.
      // If the computation of the size overflow and falls to 0, then
      // ...
      return res;
    }

    if (this_event_size == 3) // can it be a midi key press or key release event?
    {
      const auto tmp = std::vector<uint8_t>(std::next(stream_begin, static_cast<int>(nb_read)),
					    std::next(stream_begin, static_cast<int>(nb_read + 3)));
      if (is_key_release_event(tmp))
      {
	res.keys_up.emplace_back(tmp[1] /* pitch */);
      }
      else if (is_key_down_event(tmp))
      {
	res.keys_down.emplace_back(/* pitch */ tmp[1], /* staff_num */ 0);
      }
    }

    nb_read += this_event_size;
  }

  return res;
}

std::vector<midi_message_t>
get_midi_from_keys_events(const std::vector<key_down>& keys_down,
			  const std::vector<key_up>& keys_up)
{
  std::vector<midi_message_t> res;
  res.reserve(keys_down.size() + keys_up.size());

  for (const auto& key : keys_down)
  {
    res.emplace_back(std::vector<uint8_t>{ { 0x90 /* down event */,
					     key.pitch,
					     100 /* volume */ } });
  }

  for (const auto& key : keys_up)
  {
    res.emplace_back( std::vector<uint8_t>{ { 0x80, // up event,
	                                      key.pitch,
				 	      0 /* volume */ } } );
  }

  return res;
}

template <typename T>
static
void list_midi_ports(std::ostream& out, T& player, const char* direction)
{

  const auto nb_ports = player.getPortCount();
  if (nb_ports == 0)
  {
    out << "Sorry: no " << direction << " midi port found\n";
  }
  else
  {
    if (nb_ports == 1)
    {
      out << "1 " << direction << " port found:\n";
    }
    else
    {
      out << nb_ports << " " << direction << " ports found:\n";
    }

    for (auto i = decltype(nb_ports){0}; i < nb_ports; ++i)
    {
      out << "  " << i << " -> " << player.getPortName(i) << "\n";
    }
  }

}

void list_midi_ports(std::ostream& out)
{
  RtMidiOut out_player;
  list_midi_ports(out, out_player, "output");

  out << "\n";

  RtMidiIn in_player;
  list_midi_ports(out, in_player, "input");

}

static unsigned int get_nb_output_ports()
{
  RtMidiOut player;
  return player.getPortCount();
}

unsigned int get_port(const std::string& s)
{
  const auto nb_outputs = get_nb_output_ports();

  try
  {
    const auto res = std::stoi(s);
    if ((res < 0) or (static_cast<unsigned int>(res) >= nb_outputs))
    {
      std::cerr << "Warning: invalid port\n";
      return 0;
    }
    return static_cast<unsigned int>(res);
  }
  catch (std::invalid_argument&)
  {
    // argument is not a number, let's see if it matches the name of one of the output
    for (auto i = decltype(nb_outputs){0}; i < nb_outputs; ++i)
    {
      RtMidiOut player;
      if (s == player.getPortName(i))
      {
	return i;
      }
    }

    std::cerr << "Warning: invalid port\n";
    return 0;
  }
}


std::string get_first_svg_line(const std::vector<uint8_t>& data)
{
  const char* const sheet_data = static_cast<const char*>(static_cast<const void*>(data.data()));
  const auto size = data.size();

  // load is successful, so it is a proper svg file, so let's find the first line
  unsigned closing_angle_pos = 0;
  while ((closing_angle_pos < size) and (sheet_data[closing_angle_pos] != '>'))
  {
    closing_angle_pos++;
  }

  return std::string{sheet_data, closing_angle_pos + 1};
}


uint16_t find_last_measure(const std::vector<music_sheet_event>& events)
{
  const auto it = std::find_if(events.crbegin(), events.crend(), [] (const auto& elt) {
	return elt.has_bar_number_change();
  });

  if (it == events.crend())
  {
    throw std::runtime_error("Error: couldn't find the last measure number of the music sheet");
  }

  return it->new_bar_number;
}


uint16_t find_music_sheet_pos(const std::vector<music_sheet_event>& events, unsigned int event_pos)
{
  if (event_pos >= events.size())
  {
    throw std::runtime_error("Error: trying to find in which page appears an out-of-bound event");
  }

  const auto last_event = events.size() - 1;
  const auto dst_to_end = static_cast<std::vector<music_sheet_event>::difference_type>(last_event - event_pos);

  const auto it = std::find_if(std::next(events.crbegin(), dst_to_end), events.crend(), [] (const auto& elt) {
    return elt.has_svg_file_change();
  });

  if (it == events.crend())
  {
    throw std::runtime_error("Couldn't find the svg_file for some event");
  }

  return it->new_svg_file;
}



#if defined(__clang__)
  // clang will complain in a switch that the default case is useless
  // because all possible values in the enum are already taken into
  // account.  however, since the value can come from an unrestricted
  // uint8_t, the default is actually a necessary safe-guard.
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

const char* rt_error_type_as_str(RtMidiError::Type value)
{
  switch (value) {
    case RtMidiError::Type::WARNING:             return "A non-critical error.";
    case RtMidiError::Type::DEBUG_WARNING:       return "A non-critical error which might be useful for debugging.";
    case RtMidiError::Type::UNSPECIFIED:         return "The default, unspecified error type.";
    case RtMidiError::Type::NO_DEVICES_FOUND:    return "No devices found on system.";
    case RtMidiError::Type::INVALID_DEVICE:      return "An invalid device ID was specified.";
    case RtMidiError::Type::MEMORY_ERROR:        return "An error occured during memory allocation.";
    case RtMidiError::Type::INVALID_PARAMETER:   return "An invalid parameter was specified to a function.";
    case RtMidiError::Type::INVALID_USE:         return "The function was called incorrectly.";
    case RtMidiError::Type::DRIVER_ERROR:        return "A system driver error occured.";
    case RtMidiError::Type::SYSTEM_ERROR:        return "A system error occured.";
    case RtMidiError::Type::THREAD_ERROR:        return "A thread error occured.";
    default:                                     return "Something completely unknown to RtMidi.";
  }
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif


template <typename T>
static std::vector<std::string> get_midi_ports_name(T& midi)
{
  std::vector<std::string> res;

  const auto nb_ports = midi.getPortCount();

  for (auto i = decltype(nb_ports){0}; i < nb_ports; ++i)
  {
    const auto port_name = midi.getPortName(i);
    res.emplace_back(std::move(port_name));
  }

  return res;
}


std::vector<std::string> get_input_midi_ports_name(RtMidiIn& midi_listener)
{
  return get_midi_ports_name(midi_listener);
}


std::vector<std::string> get_output_midi_ports_name(RtMidiOut& midi_player)
{
  return get_midi_ports_name(midi_player);
}
