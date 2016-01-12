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

#include "preferences.hpp"
//#include "preferences_display.hpp"
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

REGISTER_DIALOG(preferences)

tpreferences::tpreferences(display* disp) :
	 disp_(disp)
{
}

/**
 * Sets up states and callbacks for each of the widgets
 */
void tpreferences::initialize_states_and_callbacks(twindow& window)
{
	/**
	 * DISPLAY PANEL
	 */

	/************ FULLSCREEN TOGGLE ************/
	ttoggle_button& toggle_fullscreen =
			find_widget<ttoggle_button>(&window, "fullscreen", false);

	toggle_fullscreen.set_value(preferences::fullscreen());

	connect_signal_mouse_left_click(
			toggle_fullscreen,
			boost::bind(&tpreferences::fullscreen_toggle_callback,
			this, boost::ref(window)));

	/************ RESOLUTION LABEL ************/
	std::stringstream res;
	res << preferences::resolution().first << " x " <<
		   preferences::resolution().second;

	find_widget<tscroll_label>(&window, "resolution", false).set_label(
			res.str());

	/************ SET RESOLUTION BUTTON ************/
	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "resolution_set", false),
			boost::bind(&tpreferences::show_video_mode_dialog, this));


	/**
	 * SOUND PANEL
	 */

	/************ SOUND FX ************/
	ttoggle_button& sfx_toggle =
			find_widget<ttoggle_button>(&window, "sound_toggle_sfx", false);
	tslider& sfx_volume =
			find_widget<tslider>(&window, "sound_volume_sfx", false);

	sfx_toggle.set_value(preferences::sound_on());
	sfx_volume.set_value(preferences::sound_volume());
	sfx_volume.set_active(preferences::sound_on());

	connect_signal_mouse_left_click(
			sfx_toggle,
			boost::bind(&tpreferences::sfx_toggle_callback,
			this, boost::ref(window)));

	/************ MUSIC ************/
	ttoggle_button& music_toggle =
			find_widget<ttoggle_button>(&window, "sound_toggle_music", false);
	tslider& music_volume =
			find_widget<tslider>(&window, "sound_volume_music", false);

	music_toggle.set_value(preferences::music_on());
	music_volume.set_value(preferences::music_volume());
	music_volume.set_active(preferences::music_on());

	connect_signal_mouse_left_click(
			music_toggle,
			boost::bind(&tpreferences::music_toggle_callback,
			this, boost::ref(window)));

	/************ TURN BELL ************/
	ttoggle_button& turn_bell_toggle =
			find_widget<ttoggle_button>(&window, "sound_toggle_bell", false);
	tslider& turn_bell_volume =
			find_widget<tslider>(&window, "sound_volume_bell", false);

	turn_bell_toggle.set_value(preferences::turn_bell());
	turn_bell_volume.set_value(preferences::bell_volume());
	turn_bell_volume.set_active(preferences::turn_bell());

	connect_signal_mouse_left_click(
			turn_bell_toggle,
			boost::bind(&tpreferences::turn_bell_toggle_callback,
			this, boost::ref(window)));

	/************ UI FX ************/
	ttoggle_button& uisfx_toggle =
			find_widget<ttoggle_button>(&window, "sound_toggle_uisfx", false);
	tslider& uisfx_volume =
			find_widget<tslider>(&window, "sound_volume_uisfx", false);

	uisfx_toggle.set_value(preferences::UI_sound_on());
	uisfx_volume.set_value(preferences::UI_volume());
	uisfx_volume.set_active(preferences::UI_sound_on());

	connect_signal_mouse_left_click(
			uisfx_toggle,
			boost::bind(&tpreferences::ui_sfx_toggle_callback,
			this, boost::ref(window)));

#if 0
	tbutton& test_button = find_widget<tbutton>(&window, "button1", false);

	connect_signal_mouse_left_click(
			test_button,
			boost::bind(&tpreferences::button_test_callback,
			this));
#endif
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

/** Callback functions. **/
 
/**
 * DISPLAY PANEL CALLBACKS
 */

/** FULLSCREEN **/
void tpreferences::fullscreen_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "fullscreen", false).get_value_bool();
	disp_->video().set_fullscreen(ison);
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
		= disp_->video().get_available_resolutions();

	if(resolutions.empty()) {
		gui2::show_transient_message(disp_->video() , "",
			_("There are no alternative video modes available"));

		return;
	}

	std::vector<std::string> options;
	unsigned current_choice = 0;

	for(size_t k = 0; k < resolutions.size(); ++k) {
		std::pair<int, int> const& res = resolutions[k];

		if (res == disp_->video().current_resolution())
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
	dlg.show(disp_->video());

	int choice = dlg.selected_index();

	if(choice == -1 || resolutions[static_cast<size_t>(choice)] == disp_->video().current_resolution()) {
		return;
	}

	if (disp_->get_singleton()) {
		disp_->get_singleton()->video().set_resolution(resolutions[static_cast<size_t>(choice)]);
	}
}

/**
 * SOUND PANEL CALLBACKS
 */
void tpreferences::sfx_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "sound_toggle_sfx", false).get_value_bool();
	preferences::set_sound(ison);

	find_widget<tslider>(&window, "sound_volume_sfx", false).set_active(ison);
}

void tpreferences::music_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "sound_toggle_music", false).get_value_bool();
	preferences::set_music(ison);

	find_widget<tslider>(&window, "sound_volume_music", false).set_active(ison);
}

void tpreferences::turn_bell_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "sound_toggle_bell", false).get_value_bool();
	preferences::set_turn_bell(ison);

	find_widget<tslider>(&window, "sound_volume_bell", false).set_active(ison);
}

void tpreferences::ui_sfx_toggle_callback(twindow& window)
{
	const bool ison =
		find_widget<ttoggle_button>(&window, "sound_toggle_uisfx", false).get_value_bool();
	preferences::set_UI_sound(ison);

	find_widget<tslider>(&window, "sound_volume_uisfx", false).set_active(ison);
}

void tpreferences::button_test_callback()
{
	std::cerr << "hi from button1\n";
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
