/*
   Copyright (C) 2011, 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED
#define GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED

#include "config.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/toggle_button.hpp"

// This file is not named preferences.hpp in order -I conflicts with
// src/preferences.hpp.

namespace gui2
{

class tpreferences : public tdialog
{
public:
	/**
	 * Constructor.
	 */
	tpreferences(const config& game_cfg);

	/** The execute function -- see @ref tdialog for more information. */
	//
	// TODO: we want to get status info out in case it's necessary, e.g.
	//       to refresh the game display or stuff, hence this is execute() and
	//       not display(). But we probably should drop this later and force
	//       callers communicate with the dialog directly. Undecided. -- shadowm
	//
	static bool execute(CVideo& video, const config& game_cfg)
	{
		tpreferences(game_cfg).show(video);
		return true;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** Initializers */
	void initialize_members(twindow& window);
	void initialize_tabs(twindow& window);

	void add_tab(class tlistbox& tab_bar, const std::string& label);
	void add_pager_row(class tlistbox& selector, const std::string& icon, const std::string& label);
	void set_visible_page(twindow& window, unsigned int page, const std::string& pager_id);

	/** Callback for page selection changes. */
	void on_page_select(twindow& window);
	void on_tab_select(twindow& window, const std::string& widget_id);

	void fullscreen_toggle_callback(twindow& window);
	void accl_speed_slider_callback(twindow& window);

	void setup_single_toggle(
		  const std::string& widget_id
		, const bool start_value
		, void (*callback) (bool)
		, twindow& window);

	void setup_toggle_slider_pair(
		  const std::string& toggle_widget
		, const std::string& slider_widget
		, const bool toggle_start_value
		, const int slider_state_value
		, boost::function<void(bool)> toggle_callback
		, void (*slider_callback) (int)
		, twindow& window);

	void setup_single_slider(
		  const std::string& widget_id
		, const int start_value
		, void (*callback) (int)
		, twindow& window);

	void setup_slider_label_pair(
		  const std::string& slider_widget
		, const std::string& label_widget
		, const int start_value
		, void (*callback) (int)
		, twindow& window);

	void single_toggle_callback(const std::string& widget,
		void (*setter) (bool), twindow& window);

	void toggle_slider_pair_callback(const std::string& toggle_widget,
		const std::string& slider_widget,
		boost::function<void(bool)> setter, twindow& window);

	void single_slider_callback(const std::string& widget,
		void (*setter) (int), twindow& window);

	void slider_label_pair_callback(const std::string& slider_widget,
		const std::string& label_widget, void (*setter) (int), twindow& window);

	// FIXME: remove. It's a dupe of the one in game_preferences.hpp, but that
	// is unnamed so I can't use it
	enum LOBBY_JOINS { SHOW_NONE, SHOW_FRIENDS, SHOW_ALL };

	typedef std::pair<ttoggle_button*, int> lobby_radio_toggle;
	std::vector<lobby_radio_toggle> lobby_joins_;

	void setup_radio_toggle(
		  const std::string& toggle_id
		, LOBBY_JOINS enum_value
		, int start_value
		, std::vector<std::pair<ttoggle_button*, int> >& vec
		, twindow& window);

	void toggle_radio_callback(
		  const std::vector<std::pair<ttoggle_button*, int> >& vec
		, int& value
		, ttoggle_button* active);

	const config& game_cfg_;

	struct advanced_preferences_sorter
	{
		bool operator()(const config& lhs, const config& rhs) const
		{
			return lhs["name"].t_str().str() < rhs["name"].t_str().str();
		}
	};

	std::vector<config> adv_preferences_cfg_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED */
