#include <stdexcept>
#include <fstream>
#include <string>
#include <algorithm>
#include <cstring> // for std::memcmp

#include <QSvgRenderer> // to ensure reading the svg files won't cause any problem

#include "bin_file_reader.hh"
#include "utils.hh"

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

static bool is_header_correct(std::fstream& file, const char expected[4])
{
  char buffer[4];
  file.read(buffer, sizeof(buffer));

  return (file.gcount() == sizeof(buffer)) and
    (std::memcmp(buffer, expected, sizeof(buffer)) == 0);
}

static
std::string read_string(std::fstream& file)
{
  std::string res;

  uint8_t c = read_big_endian<uint8_t>(file);
  while (c != '\0')
  {
    res.push_back(static_cast<char>(c)); // don't add '\0' by hand in a std::string
    c = read_big_endian<uint8_t>(file);
  }

  return res;
}


static
music_sheet_event read_grouped_event(std::fstream& file)
{
  music_sheet_event res;

  res.time = read_big_endian<uint64_t>(file);
  const auto nb_events = read_big_endian<uint8_t>(file);

  if (nb_events == 0)
  {
    throw std::invalid_argument("Error: a group of events must have at least one event!");
  }

  res.sheet_events = static_cast<decltype(res.sheet_events)>(0);

  for (auto i = decltype(nb_events){0} ; i < nb_events; ++i)
  {
    const auto event_id = read_big_endian<uint8_t>(file);

    enum event_type : uint8_t
    {
      press_key      = 0,
      release_key    = 1,
      set_bar_number = 2,
      set_cursor     = 3,
      set_svg_file   = 4,
    };

    switch (event_id)
    {
      case event_type::press_key:
      {
	const auto pitch = read_big_endian<uint8_t>(file);
	const auto staff_number = read_big_endian<uint8_t>(file);
	res.keys_down.emplace_back( key_down{pitch,  staff_number} );
	break;
      }

      case event_type::release_key:
      {
	const auto pitch = read_big_endian<uint8_t>(file);
	res.keys_up.emplace_back( key_up{ pitch });
	break;
      }

      case event_type::set_bar_number:
      {
	if (res.sheet_events & has_event::bar_number_change)
	{
	  throw std::invalid_argument("Error: two 'bar number change' happening in the same music-sheet event");
	}

	res.new_bar_number = read_big_endian<uint16_t>(file);
	res.sheet_events = static_cast<has_event>(res.sheet_events | has_event::bar_number_change);
	break;
      }

      case event_type::set_cursor:
      {
	if (res.sheet_events & has_event::cursor_pos_change)
	{
	  throw std::invalid_argument("Error: two 'cursor pos change' happening in the same music-sheet event");
	}

	const auto left = read_big_endian<uint32_t>(file);
	const auto right = read_big_endian<uint32_t>(file);
	const auto top = read_big_endian<uint32_t>(file);
	const auto bottom = read_big_endian<uint32_t>(file);

	res.new_cursor_box = cursor_box_t{ .left = left,
					   .right = right,
					   .top = top,
					   .bottom = bottom };

	res.sheet_events = static_cast<has_event>(res.sheet_events | has_event::cursor_pos_change);
	break;
      }

      case event_type::set_svg_file:
      {
	if (res.sheet_events & has_event::svg_file_change)
	{
	  throw std::invalid_argument("Error: two 'file change' happening in the same music-sheet event");
	}

	res.new_svg_file = read_big_endian<uint16_t>(file);
	res.sheet_events = static_cast<has_event>(res.sheet_events | has_event::svg_file_change);
	break;
      }

      default:
	throw std::invalid_argument("Error: invalid event type");
    }
  }

  if ((res.sheet_events & svg_file_change) and
      (not (res.sheet_events & cursor_pos_change)))
  {
    throw std::invalid_argument("Error: How come an change of a page is not linked to a change of ");
  }

  return res;
}

bin_song_t get_song(const std::string& filename)
{
  std::fstream file(filename, std::ios::binary | std::ios::in);

  if (not file.is_open())
  {
    throw std::runtime_error(std::string{"Error: unable to open midi file ["}
			     + filename + "]");
  }

  // file must start by the magic number 'LPYP'
  const char file_header[4] = { 'L', 'P', 'Y', 'P' };
  if (not is_header_correct(file, file_header))
  {
    throw std::invalid_argument("Error: wrong file format (wrong header)");
  }

  // one byte representing the format number
  const auto format_version = read_big_endian<uint8_t>(file);
  if (format_version != 0) // only one format supported for now
  {
    throw std::invalid_argument("Error: unknown file format");
  }

  // one byte for the number of staff_number->instrument name (nb_staff_num)
  const auto nb_instr = read_big_endian<uint8_t>(file);
  if (nb_instr == 0)
  {
    throw std::invalid_argument("Error: at least one instrument must be played");
  }

  bin_song_t res;
  // for nb_instr do
  for (auto i = decltype(nb_instr){0}; i < nb_instr; ++i)
  {
    // read the instrument name
    auto instr_name = read_string(file);
    res.instr_names.emplace_back( std::move(instr_name) );
  }

  // read the number of music_sheet_event
  const auto nb_group_of_events = read_big_endian<uint64_t>(file);
  if (nb_group_of_events == 0)
  {
    throw std::invalid_argument("Error: a song with nothing happening? That doesn't make sense");
  }

  // read all the music_sheet_event (aka group of events)
  for (auto i = decltype(nb_group_of_events){0}; i < nb_group_of_events; ++i)
  {
    auto grouped_event = read_grouped_event(file);
    res.events.emplace_back( std::move(grouped_event) );
  }

  // sanity check: the events must appear in chronological order
  if (not std::is_sorted( res.events.cbegin(), res.events.cend(), [] (const auto& a, const auto& b) {
	return a.time < b.time;
      }))
  {
    throw std::invalid_argument("Error: The events should appear in chronological order");
  }

  // read the svg files
  const auto nb_svg_files = read_big_endian<uint16_t>(file);
  for (auto i = decltype(nb_svg_files){0}; i < nb_svg_files; ++i)
  {
    const auto file_size = read_big_endian<uint32_t>(file);

    svg_data this_file;
    this_file.data.resize( file_size );

    auto ptr = static_cast<void*>(this_file.data.data());
    file.read( static_cast<char*>(ptr), static_cast<int>(file_size) );

    res.svg_files.emplace_back( std::move(this_file) );
  }

  // sanity check: make sure parsing the svg_files won't cause any problem
  for (auto i = decltype(nb_svg_files){0}; i < nb_svg_files; ++i)
  {
    const QByteArray sheet (static_cast<const char*>(static_cast<const void*>(res.svg_files[i].data.data())),
			    static_cast<int>(res.svg_files[i].data.size()));

    QSvgRenderer renderer;
    const auto is_load_successfull = renderer.load(sheet);
    if (not is_load_successfull)
    {
      throw std::runtime_error(std::string{"Error: Failed to read svg file "} +
			       std::to_string(static_cast<long unsigned int>(i)));
    }
  }

  // // sanity check: make sure all "change_music_sheet/turn page" events are valids
  // // todo: rewrite this using the following code:
  // // reason it is not used is that some version of g++ segfault when compiling this code.
  // // therefore, check from time to time if an updated version of g++ fixed this bug.
  // const auto is_missing_svgs = std::any_of(res.events.cbegin(), res.events.cend(), [=] (const auto& ev) {
  //     return ((ev.sheet_events & has_event::svg_file_change) != 0) and (ev.new_svg_file >= nb_svg_files);
  //   });
  // if (is_missing_svgs)
  // {
  //   throw std::runtime_error("Error: the file is missing some pages of the music sheet inside");
  // }
  for (const auto& elt : res.events)
  {
    if (((elt.sheet_events & has_event::svg_file_change) != 0) and
	(elt.new_svg_file >= nb_svg_files))
    {
      throw std::runtime_error("Error: the file is missing some pages of the music sheet inside");
    }
  }

  // sanity check: file must have been entirely read by now (no more remaining
  // bytes)
  file.peek(); // just to set the eof bit.
  if (not file.eof())
  {
    throw std::invalid_argument("Error: invalid file (extra bytes after end of data)");
  }


  return res;
}
