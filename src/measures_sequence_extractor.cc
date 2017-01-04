#include "measures_sequence_extractor.hh"


// returns the positions of events having a bar_change_number where the new number is the
// requested measure
static
std::vector<std::vector<music_sheet_event>::size_type>
get_starting_measure_event_pos(const bin_song_t& song,
			       decltype(music_sheet_event::new_bar_number) measure)
{
  std::vector< std::vector<music_sheet_event>::size_type > res;

  const auto& events = song.events;
  const auto events_begin = events.begin();
  const auto events_end = events.end();
  {
    auto it = events_begin;
    while (it != events_end)
    {
      it = std::find_if(it, events_end, [=] (const auto& elt) {
	  return elt.has_bar_number_change() and (elt.new_bar_number == measure);
	});

      if (it != events_end)
      {
	const auto distance = std::distance(events_begin, it);
	if (distance < 0)
	{
	  throw std::runtime_error("Error, how come an iterator appears before the beginning of a vector???");
	}

	res.push_back(static_cast<decltype(res)::value_type>(distance));
	++it;
      }
    }
  }

  // sanity check: At this point, res must be sorted in ascending order
  if (not std::is_sorted(res.begin(), res.end()))
  {
    throw std::runtime_error("Postcondition failed: vector should be sorted");
  }

  // sanity check: there shouldn't be the same element twice in the vector.
  const auto nb_elts = res.size();
  for (auto i = decltype(nb_elts){0}; i + 1 < nb_elts; ++i)
  {
    if (res[i] == res[i + 1])
    {
      throw std::runtime_error("postcondition failed: elements in vector should be unique!");
    }
  }

  return res;
}

static
std::vector<std::vector<music_sheet_event>::size_type>
get_end_measures_sequence_pos(const bin_song_t& song,
			      decltype(music_sheet_event::new_bar_number) last_measure)
{
  const auto stop_positions = get_starting_measure_event_pos(song, last_measure);

  // for each stop positions, we will go to the next starting measure position.  doing this means
  // that when user ask to "play measure x to y", it means 'x' and 'y' inclusive
  const auto& events = song.events;

  const auto nb_events = events.size();
  const auto events_begin = events.begin();
  const auto events_end = events.end();

  auto res = decltype(stop_positions){};
  res.reserve(stop_positions.size());

  for (const auto stop_pos : stop_positions)
  {
    if (stop_pos >= static_cast<decltype(stop_pos)>(std::distance(events_begin, events_end)))
    {
      throw std::runtime_error("stop event position is not in the event vector");
    }

    auto it = std::next(events_begin, static_cast<decltype(events_begin)::difference_type>(stop_pos));
    if (it == events_end)
    {
      throw std::runtime_error("stop pos iterator should point to the beginning of parameter last_measure");
    }

    if (not it->has_bar_number_change())
    {
      throw std::runtime_error("stop pos iterator should point to an event with \"last_measure\" bar number change");
    }

    // look for the next position with a has_bar_number_change
    it = std::find_if( std::next(it), events_end, [] (const auto& elt) {
	return elt.has_bar_number_change();
      });

    if (it == events_end)
    {
      res.push_back(nb_events - 1); // last element in vector
    }
    else
    {
      res.push_back(static_cast<decltype(stop_pos)>(std::distance(events_begin, it)));
    }
  }

  // sanity check: At this point, res must be sorted in ascending order
  if (not std::is_sorted(res.begin(), res.end()))
  {
    throw std::runtime_error("Postcondition failed: vector should be sorted");
  }

  // sanity check: there shouldn't be twice the same element in the vector.
  const auto nb_elts = res.size();
  for (auto i = decltype(nb_elts){0}; i + 1 < nb_elts; ++i)
  {
    if (res[i] == res[i + 1])
    {
      throw std::runtime_error("postcondition failed: elements in vector should be unique!");
    }
  }

  return res;
}


std::vector<std::pair<std::vector<music_sheet_event>::size_type,
		      std::vector<music_sheet_event>::size_type>>
get_measures_sequence_pos(const bin_song_t& song,
			  decltype(music_sheet_event::new_bar_number) first_measure,
			  decltype(music_sheet_event::new_bar_number) last_measure)
{
  std::vector<std::pair<std::vector<music_sheet_event>::size_type,
			std::vector<music_sheet_event>::size_type>> res;

  const auto start_positions = get_starting_measure_event_pos(song, first_measure);
  const auto end_positions = get_end_measures_sequence_pos(song, last_measure);

  for (const auto start : start_positions)
  {
    for (const auto end : end_positions)
    {
      if (end > start)
      {
	res.push_back({start, end});
      }
    }
  }


  // sanity check: At this point, res must be sorted in ascending order
  if (not std::is_sorted(res.begin(), res.end(), [] (const auto& elt1, const auto& elt2) {
	return (elt1.first < elt2.first) or ((elt1.first == elt2.first) and (elt1.second < elt2.second));
     }))
  {
    throw std::runtime_error("Postcondition failed: vector should be sorted");
  }

  // sanity check: there should not be two times the same element in the vector.
  const auto nb_elts = res.size();
  for (auto i = decltype(nb_elts){0}; i + 1 < nb_elts; ++i)
  {
    if (res[i] == res[i + 1])
    {
      throw std::runtime_error("postcondition failed: elements in vector should be unique!");
    }
  }


  return res;
}
