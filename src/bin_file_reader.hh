#ifndef BIN_FILE_READER_HH
#define BIN_FILE_READER_HH

#include <vector>
#include <cstdint>
#include <string>

struct key_down
{
    uint8_t pitch;
    uint8_t staff_num;
};

struct key_up
{
    uint8_t pitch;
};

#ifndef __has_attribute
  #define __has_attribute(x) 0  // Compatibility with non-clang compilers.
#endif


#if __has_attribute(flag_enum)
#define FLAG_ENUM_ATTRIBUTE __attribute__((flag_enum))
#else
#define FLAG_ENUM_ATTRIBUTE
#endif

enum has_event : uint8_t
{
    bar_number_change = 1 << 0,
    cursor_pos_change = 1 << 1,
    svg_file_change   = 1 << 2,
} FLAG_ENUM_ATTRIBUTE;

struct cursor_box_t
{
    uint32_t left;
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
};

struct music_sheet_event
{
    music_sheet_event()
      : time()
      , keys_down()
      , keys_up()
      , sheet_events()
      , new_cursor_box()
      , new_bar_number()
      , new_svg_file()
    {
    }

    uint64_t time; // occuring time relative to beginning of the song
		   // (in ns)
    std::vector<key_down> keys_down;
    std::vector<key_up>   keys_up;
    enum has_event sheet_events;
    cursor_box_t new_cursor_box;
    uint16_t new_bar_number;
    uint16_t new_svg_file;
};

struct svg_data
{
    svg_data()
        : data ()
    {
    }

    std::vector<uint8_t> data;
};

struct bin_song_t
{
    bin_song_t()
      : events()
      , instr_names()
      , svg_files ()
    {
    }

    std::vector<music_sheet_event> events;
    std::vector<std::string> instr_names;
    std::vector<svg_data> svg_files;
};



bin_song_t get_song(const std::string& filename);

#endif /* BIN_FILE_READER_HH */
