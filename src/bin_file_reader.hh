#ifndef BIN_FILE_READER_HH
#define BIN_FILE_READER_HH

#include <vector>
#include <cstdint>
#include <string>
#include <QByteArray>
#include <QRectF>

#include "utils.hh"

enum has_event : uint8_t
{
    bar_number_change = 1 << 0,
    cursor_pos_change = 1 << 1,
    svg_file_change   = 1 << 2,
};

struct music_sheet_event
{
    music_sheet_event()
      : time()
      , keys_down()
      , keys_up()
      , midi_messages()
      , new_cursor_box()
      , cursor_box_coord()
      , sheet_events(static_cast<has_event>(0))
      , new_bar_number()
      , new_svg_file()
    {
    }

    uint64_t time; // occuring time relative to beginning of the song
		   // (in ns)
    std::vector<key_down> keys_down;
    std::vector<key_up>   keys_up;
    std::vector<midi_message_t> midi_messages;
    QByteArray new_cursor_box;
    QRectF cursor_box_coord;
  private:
    enum has_event sheet_events;
  public:
    uint16_t new_bar_number;
    uint16_t new_svg_file;

    bool has_bar_number_change() const
    {
      return (sheet_events & has_event::bar_number_change) != 0;
    }

    bool has_cursor_pos_change() const
    {
      return (sheet_events & has_event::cursor_pos_change) != 0;
    }

    bool has_svg_file_change() const
    {
      return (sheet_events & has_event::svg_file_change) != 0;
    }

    // todo add overload that takes parameter with move semantics
    void add_cursor_change(const char * const _new_cursor_box, const QRectF& _cursor_box_coord)
    {
      new_cursor_box = _new_cursor_box;
      cursor_box_coord = _cursor_box_coord;
      sheet_events = static_cast<has_event>(sheet_events | has_event::cursor_pos_change);
    }

    void add_svg_file_change(const uint16_t new_svg_file_pos)
    {
      new_svg_file = new_svg_file_pos;
      sheet_events = static_cast<has_event>(sheet_events | has_event::svg_file_change);
    }

    void add_bar_number_change(const uint16_t _new_bar_number)
    {
      new_bar_number = _new_bar_number;
      sheet_events = static_cast<has_event>(sheet_events | has_event::bar_number_change);
    }


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
      , nb_events(0)
      , instr_names()
      , svg_files ()
    {
    }

    std::vector<music_sheet_event> events;
    decltype(events.size()) nb_events; // stores the number of events to avoid
                                       // calling events.size() at each loop
    std::vector<std::string> instr_names;
    std::vector<svg_data> svg_files;
};



bin_song_t get_song(const std::string& filename);

#endif /* BIN_FILE_READER_HH */
