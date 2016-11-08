#ifndef MEASURES_SEQUENCE_EXTRACTOR_HH
#define MEASURES_SEQUENCE_EXTRACTOR_HH

#include "bin_file_reader.hh"

std::vector<std::pair<std::vector<music_sheet_event>::size_type,
		      std::vector<music_sheet_event>::size_type>>
get_measures_sequence_pos(const bin_song_t& song,
			  decltype(music_sheet_event::new_bar_number) first_measure,
			  decltype(music_sheet_event::new_bar_number) last_measure);


#endif
