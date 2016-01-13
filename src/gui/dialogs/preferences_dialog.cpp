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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/preferences_dialog.hpp"

#include "game_preferences.hpp"
#include "preferences.hpp"
#include "preferences_display.hpp"
#include "lobby_preferences.hpp"
#include "formatter.hpp"
#include "video.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "gettext.hpp"

#include <sstream>
#include <boost/bind.hpp>
#include <boost/math/common_factor_rt.hpp>

namespace {
	//const unsigned int num_pages = 5;
}

namespace gui2 {

using namespace preferences;

REGISTER_DIALOG(preferences)

tpreferences::tpreferences(CVideo& video) :
	 video_(video)
{
}

static std::string hacky_to_string_hack(int num)
{
	// FIXME: use std::to_string when we switch to C++11 - if that ever happens
	std::stringstream hacky_to_string_hack;
	hacky_to_string_hack << num;
	return hacky_to_string_hack.str();
}

/**
 * Small helper function to display stored resolution
 */
static void set_res_string(twindow& window)
{
	std::stringstream res;
	res << resolution().first << " x " << resolution().second;
	find_widget<tscroll_label>(&window, "resolution", false).set_label(res.str());
}

/**
 * Sets the initial state and callback for a simple bool-state toggle button
 */
void tpreferences::simple_button_setup(
		  const std::string& widget_id
		, const bool start_value
		, void (*callback) (bool)
		, twindow& window)
{
	ttoggle_button& widget =
		find_widget<ttoggle_button>(&window, widget_id, false);

	widget.set_value(start_value);

	connect_signal_mouse_left_click(widget, boost::bind(
		  &tpreferences::simple_toggle_callback
		, this, widget_id
		, callback, boost::ref(window)));
}

/**
 * Sets the initial state and callback for a bool-state toggle button/slider pair
 */
void tpreferences::simple_button_slider_pair_setup(
		  const std::string& toggle_widget
		, const std::string& slider_widget
		, const bool toggle_start_value
		, const int slider_state_value
		, boost::function<void(bool)> toggle_callback
		, void (*slider_callback) (int)
		, twindow& window)
{
	ttoggle_button& button =
		find_widget<ttoggle_button>(&window, toggle_widget, false);
	tslider& slider =
		find_widget<tslider>(&window, slider_widget, false);

	button.set_value(toggle_start_value);
	slider.set_value(slider_state_value);
	slider.set_active(toggle_start_value);

	connect_signal_mouse_left_click(button, boost::bind(
		  &tpreferences::simple_toggle_slider_callback
		, this, toggle_widget, slider_widget
		, toggle_callback, boost::ref(window)));

	connect_signal_notify_modified(slider, boost::bind(
		  &tpreferences::simple_slider_callback
		, this, slider_widget
		, slider_callback, boost::ref(window)));
}

/**
 * Sets the initial state and callback for a standalone slider
 */
void tpreferences::simple_slider_setup(
		  const std::string& widget_id
		, const int start_value
		, void (*callback) (int)
		, twindow& window)
{
	tslider& widget = find_widget<tslider>(&window, widget_id, false);

	widget.set_value(start_value);

	connect_signal_notify_modified(widget, boost::bind(
		  &tpreferences::simple_slider_callback
		, this, widget_id
		, callback, boost::ref(window)));
}

/**
 * Sets the initial state and callback for a standalone slider
 */
void tpreferences::simple_slider_label_setup(
		  const std::string& slider_widget
		, const std::string& label_widget
		, const int start_value
		, void (*callback) (int)
		, twindow& window)
{
	tslider& slider = find_widget<tslider>(&window, slider_widget, false);
	tlabel& label = find_widget<tlabel>(&window, label_widget, false);

	slider.set_value(start_value);
	label.set_label(hacky_to_string_hack(start_value));

	connect_signal_notify_modified(slider, boost::bind(
		  &tpreferences::simple_slider_label_callback
		, this, slider_widget, label_widget
		, callback, boost::ref(window)));
}

/**
 * Sets up states and callbacks for each of the widgets
 */
void tpreferences::initialize_states_and_callbacks(twindow& window)
{
	/**
	 * GENERAL PANEL
	 */

	/** SCROLL SPEED **/
	simple_slider_setup("scroll_speed",
		scroll_speed(), set_scroll_speed, window);

	/** ACCELERATED SPEED **/
	// TODO: figure out how to deal with double-type values to int
	//simple_button_slider_pair_setup("turbo_toggle", "turbo_slider",
	//	turbo(), turbo_speed(),
	//	set_turbo, set_turbo_speed, window);

	/** SKIP AI MOVES **/
	simple_button_setup("skip_ai_moves",
		show_ai_moves(), set_show_ai_moves, window);

	/** DISABLE AUTO MOVES **/
	simple_button_setup("disable_auto_moves",
		disable_auto_moves(), set_disable_auto_moves, window);

	/** TURN DIALOG **/
	simple_button_setup("show_turn_dialog",
		turn_dialog(), set_turn_dialog, window);

	/** ENABLE PLANNING MODE **/
	simple_button_setup("whiteboard_on_start",
		enable_whiteboard_mode_on_start(), set_enable_whiteboard_mode_on_start, window);

	/** HIDE ALLY PLANS **/
	simple_button_setup("whiteboard_hide_allies",
		hide_whiteboard(), set_hide_whiteboard, window);

	/** INTERRUPT ON SIGHTING **/
	simple_button_setup("interrupt_move_when_ally_sighted",
		interrupt_when_ally_sighted(), set_interrupt_when_ally_sighted, window);

	/** SAVE REPLAYS **/
	simple_button_setup("save_replays",
		save_replays(), set_save_replays, window);

	/** DELETE AUTOSAVES **/
	simple_button_setup("delete_saves",
		delete_saves(), set_delete_saves, window);

	/** MAX AUTO SAVES **/
	simple_slider_label_setup("max_saves_slider", "max_saves_value",
		autosavemax(), set_autosavemax, window);


	/**
	 * DISPLAY PANEL
	 */

	/** FULLSCREEN TOGGLE **/
	ttoggle_button& toggle_fullscreen =
			find_widget<ttoggle_button>(&window, "fullscreen", false);

	toggle_fullscreen.set_value(fullscreen());

	// We bind a special callback function, so simple_button_setup() is not used
	connect_signal_mouse_left_click(toggle_fullscreen, boost::bind(
			  &tpreferences::fullscreen_toggle_callback
			, this, boost::ref(window)));

	/** RESOLUTION LABEL **/
	set_res_string(window);

	/** SET RESOLUTION BUTTON **/
	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "resolution_set", false),
			boost::bind(&tpreferences::show_video_mode_dialog, this));

	/** SHOW FLOATING LABELS **/
	simple_button_setup("show_floating_labels",
		show_floating_labels(), set_show_floating_labels, window);

	/** SHOW HALOES **/
	simple_button_setup("show_halos",
		show_haloes(), set_show_haloes, window);

	/** SHOW TEAM COLORS **/
	simple_button_setup("show_ellipses",
		show_side_colors(), set_show_side_colors, window);

	/** SHOW GRID **/
	simple_button_setup("show_grid",
		grid(), set_grid, window);

	/** ANIMATE MAP **/
	simple_button_setup("animate_terrains",
		show_haloes(), set_show_haloes, window);

	/** SHOW UNIT STANDING ANIMS **/
	simple_button_setup("animate_units_standing",
		show_standing_animations(), set_show_standing_animations, window);

	/** SHOW UNIT IDLE ANIMS **/
	simple_button_slider_pair_setup("animate_units_idle", "idle_anim_frequency",
		sound_on(), sound_volume(),
		set_idle_anim, set_idle_anim_rate, window);


	/**
	 * SOUND PANEL
	 */

	/** SOUND FX **/
	simple_button_slider_pair_setup("sound_toggle_sfx", "sound_volume_sfx",
		sound_on(), sound_volume(),
		set_sound, set_sound_volume, window);

	/** MUSIC **/
	simple_button_slider_pair_setup("sound_toggle_music", "sound_volume_music",
		music_on(), music_volume(),
		set_music, set_music_volume, window);

	/** TURN BELL **/
	simple_button_slider_pair_setup("sound_toggle_bell", "sound_volume_bell",
		turn_bell(), bell_volume(),
		set_turn_bell, set_bell_volume, window);

	/** UI FX **/
	simple_button_slider_pair_setup("sound_toggle_uisfx", "sound_volume_uisfx",
		UI_sound_on(), UI_volume(),
		set_UI_sound, set_UI_volume, window);


	/**
	 * MULTIPLAYER PANEL
	 */

	/** CHAT LINES **/
	simple_slider_setup("chat_lines",
		chat_lines(), set_chat_lines, window);

	/** CHAT TIMESTAMPPING **/
	simple_button_setup("chat_timestamps",
		chat_timestamping(), set_chat_timestamping, window);

	/** SAVE PASSWORD **/
	simple_button_setup("remember_password",
		remember_password(), set_remember_password, window);

	/** SORT LOBBY LIST **/
	simple_button_setup("lobby_sort_players",
		sort_list(), _set_sort_list, window);

	/** ICONIZE LOBBY LIST **/
	simple_button_setup("lobby_player_icons",
		iconize_list(), _set_iconize_list, window);
}

void tpreferences::add_pager_row(tlistbox& selector, const std::string& icon, const std::string& label)
{
	std::map<std::string, string_map> data;
	data["icon"]["label"] = "icons/icon-" + icon;
	data["label"]["label"] = label;
	selector.add_row(data);
}

void tpreferences::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& selector = find_widget<tlistbox>(&window, "selector", false);
	tstacked_widget& pager = find_widget<tstacked_widget>(&window, "pager", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(selector, boost::bind(
			  &tpreferences::on_page_select
			, this
			, boost::ref(window)));
#else
	selector.set_callback_value_change(dialog_callback
			<tpreferences
			, &tpreferences::on_page_select>);
#endif
	window.keyboard_capture(&selector);

	add_pager_row(selector, "general.png", _("Prefs section^General"));
	add_pager_row(selector, "display.png", _("Prefs section^Display"));
	add_pager_row(selector, "music.png",  _("Prefs section^Sound"));
	add_pager_row(selector, "multiplayer.png", _("Prefs section^Multiplayer"));
	add_pager_row(selector, "advanced.png", _("Advanced section^Advanced"));

	//
	// We need to establish callbacks before selecting the initial page,
	// otherwise widgets from other pages cannot be found afterwards.
	//
	initialize_states_and_callbacks(window);

	assert(selector.get_item_count() == pager.get_layer_count());

	selector.select_row(0);
	pager.select_layer(0);
}

void tpreferences::set_visible_page(twindow& window, unsigned int page)
{
	tstacked_widget& pager = find_widget<tstacked_widget>(&window, "pager", false);
	pager.select_layer(page);
}

/**
 * Generic callback functions
 */

/**
 * Sets a simple toggle button callback
 * The bool value of the widget is passeed to the setter
 */
void tpreferences::simple_toggle_callback(const std::string& widget,
		void (*setter) (bool), twindow& window)
{
	setter(find_widget<ttoggle_button>(&window, widget, false).get_value_bool());
}

/**
 * Sets a toggle button callback that also toggles a slider on/off
 * The bool value of the widget is passeed to the setter
 */
void tpreferences::simple_toggle_slider_callback(const std::string& toggle_widget,
		const std::string& slider_widget, boost::function<void(bool)> setter, twindow& window)
{
	const bool ison = find_widget<ttoggle_button>(&window, toggle_widget, false).get_value_bool();
	setter(ison);

	find_widget<tslider>(&window, slider_widget, false).set_active(ison);
}

/**
 * Sets a slider callback
 * The int value of the widget is passeed to the setter
 */
void tpreferences::simple_slider_callback(const std::string& widget,
		void (*setter) (int), twindow& window)
{
	setter(find_widget<tslider>(&window, widget, false).get_value());
}

/**
 * Sets a slider callback that also sets a lable to the value of the slider
 * The int value of the widget is passeed to the setter
 */
void tpreferences::simple_slider_label_callback(const std::string& slider_widget,
		const std::string& label_widget, void (*setter) (int), twindow& window)
{
	const int value = find_widget<tslider>(&window, slider_widget, false).get_value();
	setter(value);

	find_widget<tlabel>(&window, label_widget, false).set_label(hacky_to_string_hack(value));
}

/**
 * DISPLAY PANEL CALLBACKS
 */

/** FULLSCREEN **/
void tpreferences::fullscreen_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "fullscreen", false).get_value_bool();
	video_.set_fullscreen(ison);
	set_res_string(window);
}

/** SET RESOLUTION **/
// TODO: not having the implementation of this function here results
// in the dialog not opening at all, possibly because of different display
// instances. Determine cause and if to remove.
void tpreferences::show_video_mode_dialog()
{
	//const resize_lock prevent_resizing;
	//const events::event_context dialog_events_context;

	std::vector<std::pair<int,int> > resolutions
		= video_.get_available_resolutions();

	if(resolutions.empty()) {
		gui2::show_transient_message(video_ , "",
			_("There are no alternative video modes available"));

		return;
	}

	std::vector<std::string> options;
	unsigned current_choice = 0;

	for(size_t k = 0; k < resolutions.size(); ++k) {
		std::pair<int, int> const& res = resolutions[k];

		if (res == video_.current_resolution())
			current_choice = static_cast<unsigned>(k);

		std::ostringstream option;
		option << res.first << utils::unicode_multiplication_sign << res.second;
		const int div = boost::math::gcd(res.first, res.second);
		const int ratio[2] = {res.first/div, res.second/div};

		if (ratio[0] <= 10 || ratio[1] <= 10) {
			option << " (" << ratio[0] << ':' << ratio[1] << ')';
		}

		options.push_back(option.str());
	}

	gui2::tsimple_item_selector dlg(_("Choose Resolution"), "", options);
	dlg.set_selected_index(current_choice);
	dlg.show(video_);

	int choice = dlg.selected_index();

	if(choice == -1 || resolutions[static_cast<size_t>(choice)] == video_.current_resolution()) {
		return;
	}

	video_.set_resolution(resolutions[static_cast<size_t>(choice)]);
}

void tpreferences::on_page_select(twindow& window)
{
	const int selected_row =
		std::max(0, find_widget<tlistbox>(&window, "selector", false).get_selected_row());
	set_visible_page(window, static_cast<unsigned int>(selected_row));
}

void tpreferences::post_show(twindow& /*window*/)
{
}

} // end namespace gui2
