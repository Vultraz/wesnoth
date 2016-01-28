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
#include "make_enum.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/combobox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/widget.hpp"

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
	void setup_friends_list(twindow& window);

	void add_tab(class tlistbox& tab_bar, const std::string& label);
	void add_pager_row(class tlistbox& selector, const std::string& icon, const std::string& label);
	void set_visible_page(twindow& window, unsigned int page, const std::string& pager_id);

	/** Callback for page selection changes. */
	void on_page_select(twindow& window);
	void on_tab_select(twindow& window, const std::string& widget_id);

	void handle_res_select(twindow& window);
	void fullscreen_toggle_callback(twindow& window);
	void accl_speed_slider_callback(twindow& window);

	template <typename T>
	void setup_single_toggle(
		  const std::string& widget_id
		, const bool start_value
		, boost::function<void(bool)> callback
		, T& window);

	template <typename T>
	void setup_toggle_slider_pair(
		  const std::string& toggle_widget
		, const std::string& slider_widget
		, const bool toggle_start_value
		, const int slider_state_value
		, boost::function<void(bool)> toggle_callback
		, boost::function<void(int)> slider_callback
		, T& window);

	template <typename T>
	void setup_single_slider(
		  const std::string& widget_id
		, const int start_value
		, boost::function<void(int)> slider_callback
		, T& window);

	typedef std::pair<std::vector<std::string>, std::vector<std::string> > combo_data;

	template <typename T>
	void setup_combobox(
		  const std::string& widget_id
		, combo_data options
		, const unsigned start_value
		, boost::function<void(std::string)> callback
		, T& window);

	template <typename T, typename W>
	void bind_status_label(
		  T& parent
		, const std::string& label_id
		, W& window);
		
	template <typename T>
	void bind_status_label(
		  tslider& parent
		, const std::string& label_id
		, T& window);

	void single_toggle_callback(const ttoggle_button& widget,
		boost::function<void(bool)>);

	void toggle_slider_pair_callback(const ttoggle_button& toggle_widget,
		tslider& slider_widget, boost::function<void(bool)> setter);

	void single_slider_callback(const tslider& widget,
		boost::function<void(int)> setter);

	void simple_combobox_callback(const tcombobox& widget,
		boost::function<void(std::string)> setter, std::vector<std::string>& vec);

	template <typename T>
	void status_label_callback(T& parent_widget,
		tcontrol& label_widget);

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

	void add_friend_list_entry(const bool is_friend,
		ttext_box& textbox, twindow& window);

	void remove_friend_list_entry(tlistbox& friends_list, 
		ttext_box& textbox, twindow& window);

	MAKE_ENUM(ADVANCED_PREF_TYPE, 
		(TOGGLE,  "boolean")
		(SLIDER,  "int")
		(COMBO,   "combo")
		(SPECIAL, "custom")
	)

	struct advanced_preferences_sorter
	{
		bool operator()(const config& lhs, const config& rhs) const
		{
			return lhs["name"].t_str().str() < rhs["name"].t_str().str();
		}
	};

	std::vector<std::pair<int,int> > resolutions_;
	std::vector<config> adv_preferences_cfg_;
	std::vector<std::string> friend_names_;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED */
