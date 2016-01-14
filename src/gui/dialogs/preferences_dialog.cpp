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
#include "gui/dialogs/advanced_graphics_options.hpp"
#include "gui/dialogs/game_cache_options.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/mp_alerts_options.hpp"
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

/**
 * Helper functions to return associative values between accelerated speed
 * values (double) and the setter slider's steps (int).
 *
 * SLIDER AT: 1    2    3    4    5    6    7    8    9    10   11   12
 * SPEED VAL: 0.25 0.50 0.75 1.00 1.25 1.50 1.75 2.00 3.00 4.00 8.00 16.00
 */
static double int_to_accl_speed(int value)
{
	double res = 2;

	if (value >= 1 && value <= 8) {
		res = 0.25 * value;
	} else if (value == 9 || value == 10) {
		res = value - 6;
	} else if (value == 11) {
		res = 8;
	} else if (value == 12) {
		res = 16;
	}

	return res;
}

static int accl_speed_to_int(double value)
{
	int res = 2;

	if (value >= 0.25 && value <= 2) {
		res = value * 4;
	} else if (value == 3 || value == 4) {
		res = value + 6;
	} else if (value == 8) {
		res = 11;
	} else if (value == 16) {
		res = 12;
	}

	return res;
}

/**
 * Small helper function to display stored resolution
 */
static void set_res_string(twindow& window)
{
	const std::string& res =
		lexical_cast<std::string>(resolution().first) + " x " +
		lexical_cast<std::string>(resolution().second);
	find_widget<tscroll_label>(&window, "resolution", false).set_label(res);
}

/**
 * Sets the initial state and callback for a simple bool-state toggle button
 */
void tpreferences::setup_single_toggle(
		  const std::string& widget_id
		, const bool start_value
		, void (*callback) (bool)
		, twindow& window)
{
	ttoggle_button& widget =
		find_widget<ttoggle_button>(&window, widget_id, false);

	widget.set_value(start_value);

	connect_signal_mouse_left_click(widget, boost::bind(
		  &tpreferences::single_toggle_callback
		, this, widget_id
		, callback, boost::ref(window)));
}

/**
 * Sets the initial state and callback for a bool-state toggle button/slider pair
 */
void tpreferences::setup_toggle_slider_pair(
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
		  &tpreferences::toggle_slider_pair_callback
		, this, toggle_widget, slider_widget
		, toggle_callback, boost::ref(window)));

	connect_signal_notify_modified(slider, boost::bind(
		  &tpreferences::single_slider_callback
		, this, slider_widget
		, slider_callback, boost::ref(window)));
}

/**
 * Sets the initial state and callback for a standalone slider
 */
void tpreferences::setup_single_slider(
		  const std::string& widget_id
		, const int start_value
		, void (*callback) (int)
		, twindow& window)
{
	tslider& widget = find_widget<tslider>(&window, widget_id, false);

	widget.set_value(start_value);

	connect_signal_notify_modified(widget, boost::bind(
		  &tpreferences::single_slider_callback
		, this, widget_id
		, callback, boost::ref(window)));
}

/**
 * Sets the initial state and callback for a standalone slider
 */
void tpreferences::setup_slider_label_pair(
		  const std::string& slider_widget
		, const std::string& label_widget
		, const int start_value
		, void (*callback) (int)
		, twindow& window)
{
	tslider& slider = find_widget<tslider>(&window, slider_widget, false);
	tscroll_label& label = find_widget<tscroll_label>(&window, label_widget, false);

	slider.set_value(start_value);
	label.set_label(lexical_cast<std::string>(start_value));

	connect_signal_notify_modified(slider, boost::bind(
		  &tpreferences::slider_label_pair_callback
		, this, slider_widget, label_widget
		, callback, boost::ref(window)));
}

/**
 * Sets the a radio button group
 * Since (so far) there is only one group of these, I'm leaving the code specified.
 * If (at a later date) I need to add more groups, this will have to be changed
 */
void tpreferences::setup_radio_toggle(
		  const std::string& toggle_id
		, LOBBY_JOINS enum_value
		, int start_value
		, std::vector<std::pair<ttoggle_button*, int> >& vec
		, twindow& window)
{
	ttoggle_button* button = &find_widget<ttoggle_button>(&window, toggle_id, false);

	button->set_value(enum_value == start_value);

	connect_signal_mouse_left_click(*button, boost::bind(
			&tpreferences::toggle_radio_callback,
			this, boost::ref(vec), boost::ref(start_value), button));

	vec.push_back(std::make_pair(button, enum_value));
}

static void accel_slider_setter_helper(int speed)
{
	set_turbo_speed(int_to_accl_speed(speed));
}

/**
 * Sets up states and callbacks for each of the widgets
 */
void tpreferences::initialize_members(twindow& window)
{
	/**
	 * GENERAL PANEL
	 */

	/** SCROLL SPEED **/
	setup_single_slider("scroll_speed",
		scroll_speed(), set_scroll_speed, window);

	/** ACCELERATED SPEED **/
	setup_toggle_slider_pair("turbo_toggle", "turbo_slider",
		turbo(), accl_speed_to_int(turbo_speed()),
		set_turbo, accel_slider_setter_helper, window);

	find_widget<tscroll_label>(&window, "turbo_value", false).set_label(
			lexical_cast<std::string>(turbo_speed()));

	connect_signal_notify_modified(
			  find_widget<tslider>(&window, "turbo_slider", false)
			, boost::bind(&tpreferences::accl_speed_slider_callback
			, this, boost::ref(window)));

	/** SKIP AI MOVES **/
	setup_single_toggle("skip_ai_moves",
		show_ai_moves(), set_show_ai_moves, window);

	/** DISABLE AUTO MOVES **/
	setup_single_toggle("disable_auto_moves",
		disable_auto_moves(), set_disable_auto_moves, window);

	/** TURN DIALOG **/
	setup_single_toggle("show_turn_dialog",
		turn_dialog(), set_turn_dialog, window);

	/** ENABLE PLANNING MODE **/
	setup_single_toggle("whiteboard_on_start",
		enable_whiteboard_mode_on_start(), set_enable_whiteboard_mode_on_start, window);

	/** HIDE ALLY PLANS **/
	setup_single_toggle("whiteboard_hide_allies",
		hide_whiteboard(), set_hide_whiteboard, window);

	/** INTERRUPT ON SIGHTING **/
	setup_single_toggle("interrupt_move_when_ally_sighted",
		interrupt_when_ally_sighted(), set_interrupt_when_ally_sighted, window);

	/** SAVE REPLAYS **/
	setup_single_toggle("save_replays",
		save_replays(), set_save_replays, window);

	/** DELETE AUTOSAVES **/
	setup_single_toggle("delete_saves",
		delete_saves(), set_delete_saves, window);

	/** MAX AUTO SAVES **/
	setup_slider_label_pair("max_saves_slider", "max_saves_value",
		autosavemax(), set_autosavemax, window);

	/** SET HOTKEYS **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "hotkeys", false)
			, boost::bind(&preferences::show_hotkeys_preferences_dialog
			, boost::ref(window.video())));

	/** CACHE MANAGE **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "cachemg", false)
			, boost::bind(&gui2::tgame_cache_options::display
			, boost::ref(window.video())));

	/**
	 * DISPLAY PANEL
	 */

	/** FULLSCREEN TOGGLE **/
	ttoggle_button& toggle_fullscreen =
			find_widget<ttoggle_button>(&window, "fullscreen", false);

	toggle_fullscreen.set_value(fullscreen());

	// We bind a special callback function, so setup_single_toggle() is not used
	connect_signal_mouse_left_click(toggle_fullscreen, boost::bind(
			  &tpreferences::fullscreen_toggle_callback
			, this, boost::ref(window)));

	/** RESOLUTION LABEL **/
	set_res_string(window);

	/** SET RESOLUTION **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "resolution_set", false)
			, boost::bind(&preferences::show_video_mode_dialog
			, boost::ref(window.video())));

	/** SHOW FLOATING LABELS **/
	setup_single_toggle("show_floating_labels",
		show_floating_labels(), set_show_floating_labels, window);

	/** SHOW HALOES **/
	setup_single_toggle("show_halos",
		show_haloes(), set_show_haloes, window);

	/** SHOW TEAM COLORS **/
	setup_single_toggle("show_ellipses",
		show_side_colors(), set_show_side_colors, window);

	/** SHOW GRID **/
	setup_single_toggle("show_grid",
		grid(), set_grid, window);

	/** ANIMATE MAP **/
	setup_single_toggle("animate_terrains",
		show_haloes(), set_show_haloes, window);

	/** SHOW UNIT STANDING ANIMS **/
	setup_single_toggle("animate_units_standing",
		show_standing_animations(), set_show_standing_animations, window);

	/** SHOW UNIT IDLE ANIMS **/
	setup_toggle_slider_pair("animate_units_idle", "idle_anim_frequency",
		idle_anim(), idle_anim_rate(),
		set_idle_anim, set_idle_anim_rate, window);

	/** SELECT THEME **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "choose_theme", false)
			, boost::bind(&preferences::show_theme_dialog
			, boost::ref(window.video())));

	/** ORB COLORS **/
	// TODO
	//connect_signal_mouse_left_click(
	//		  find_widget<tbutton>(&window, "orbs_setup", false)
	//		, boost::bind(&preferences::show_theme_dialog
	//		, boost::ref(window.video())));

	/** ADVANCED DISPLAY OPTIONS **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "disp_advanced", false)
			, boost::bind(&gui2::tadvanced_graphics_options::display
			, boost::ref(window.video())));


	/**
	 * SOUND PANEL
	 */

	/** SOUND FX **/
	setup_toggle_slider_pair("sound_toggle_sfx", "sound_volume_sfx",
		sound_on(), sound_volume(),
		set_sound, set_sound_volume, window);

	/** MUSIC **/
	setup_toggle_slider_pair("sound_toggle_music", "sound_volume_music",
		music_on(), music_volume(),
		set_music, set_music_volume, window);

	/** TURN BELL **/
	setup_toggle_slider_pair("sound_toggle_bell", "sound_volume_bell",
		turn_bell(), bell_volume(),
		set_turn_bell, set_bell_volume, window);

	/** UI FX **/
	setup_toggle_slider_pair("sound_toggle_uisfx", "sound_volume_uisfx",
		UI_sound_on(), UI_volume(),
		set_UI_sound, set_UI_volume, window);

	/** ADVANCED SOUND OPTIONS **/
	// TODO
	//connect_signal_mouse_left_click(
	//		  find_widget<tbutton>(&window, "sound_advanced", false)
	//		, boost::bind(&gui2::tadvanced_graphics_options::display
	//		, boost::ref(window.video())));


	/**
	 * MULTIPLAYER PANEL
	 */

	/** CHAT LINES **/
	setup_single_slider("chat_lines",
		chat_lines(), set_chat_lines, window);

	/** CHAT TIMESTAMPPING **/
	setup_single_toggle("chat_timestamps",
		chat_timestamping(), set_chat_timestamping, window);

	/** SAVE PASSWORD **/
	setup_single_toggle("remember_password",
		remember_password(), set_remember_password, window);

	/** SORT LOBBY LIST **/
	setup_single_toggle("lobby_sort_players",
		sort_list(), _set_sort_list, window);

	/** ICONIZE LOBBY LIST **/
	setup_single_toggle("lobby_player_icons",
		iconize_list(), _set_iconize_list, window);

	/** LOBBY JOIN NOTIFICATIONS **/
	setup_radio_toggle("lobby_joins_none", SHOW_NONE,
		lobby_joins(), lobby_joins_, window);
	setup_radio_toggle("lobby_joins_friends", SHOW_FRIENDS,
		lobby_joins(), lobby_joins_, window);
	setup_radio_toggle("lobby_joins_all", SHOW_ALL,
		lobby_joins(), lobby_joins_, window);

	/** FRIENDS LIST **/
	// TODO
	//connect_signal_mouse_left_click(
	//		  find_widget<tbutton>(&window, "mp_friends", false)
	//		, boost::bind(&gui2::tadvanced_graphics_options::display
	//		, boost::ref(window.video())));

	/** ALERTS **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "mp_alerts", false)
			, boost::bind(&gui2::tmp_alerts_options::display
			, boost::ref(window.video())));

	/** SET WESNOTHD PATH **/
	connect_signal_mouse_left_click(
			  find_widget<tbutton>(&window, "mp_wesnothd", false)
			, boost::bind(&preferences::show_wesnothd_server_search
			, boost::ref(window.video())));
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
	initialize_members(window);

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
void tpreferences::single_toggle_callback(const std::string& widget,
		void (*setter) (bool), twindow& window)
{
	setter(find_widget<ttoggle_button>(&window, widget, false).get_value_bool());
}

/**
 * Sets a toggle button callback that also toggles a slider on/off
 * The bool value of the widget is passeed to the setter
 */
void tpreferences::toggle_slider_pair_callback(const std::string& toggle_widget,
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
void tpreferences::single_slider_callback(const std::string& widget,
		void (*setter) (int), twindow& window)
{
	setter(find_widget<tslider>(&window, widget, false).get_value());
}

/**
 * Sets a slider callback that also sets a lable to the value of the slider
 * The int value of the widget is passeed to the setter
 */
void tpreferences::slider_label_pair_callback(const std::string& slider_widget,
		const std::string& label_widget, void (*setter) (int), twindow& window)
{
	const int value = find_widget<tslider>(&window, slider_widget, false).get_value();
	setter(value);

	find_widget<tscroll_label>(&window, label_widget, false).set_label(lexical_cast<std::string>(value));
}

// Special fullsceen callback
void tpreferences::fullscreen_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "fullscreen", false).get_value_bool();
	video_.set_fullscreen(ison);
	set_res_string(window);
}

// Special Accelerated Speed slider callback
void tpreferences::accl_speed_slider_callback(twindow& window)
{
	const double speed = int_to_accl_speed(
		find_widget<tslider>(&window, "turbo_slider", false).get_value());

	find_widget<tscroll_label>(&window, "turbo_value", false).set_label(lexical_cast<std::string>(speed));
}

void tpreferences::toggle_radio_callback(
		  const std::vector<std::pair<ttoggle_button*, int> >& vec
		, int& value
		, ttoggle_button* active)
{
	FOREACH(const AUTO & e, vec)
	{
		ttoggle_button* const b = e.first;
		if(b == NULL) {
			continue;
		} else if(b == active && !b->get_value()) {
			b->set_value(true);
		} else if(b == active) {
			value = e.second;
			_set_lobby_joins(value);
		} else if(b != active && b->get_value()) {
			b->set_value(false);
		}
	}
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
