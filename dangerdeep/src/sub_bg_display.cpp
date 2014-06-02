/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// user display: submarine's bg hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_bg_display.h"
#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include <sstream>
#include "font.h"
#include "texts.h"
using namespace std;

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

sub_bg_display::scheme::scheme(bool day)
{
	const string x = day ? "BG_daylight" : "BG_redlight";
	background.reset(new image(get_image_dir() + x + "_background.jpg"));
	direction_ptr.set(x + "_pointer.png", 341, 153, 373, 346);
	for (unsigned i = 0; i < TK_PHASES; ++i) {
		ostringstream osn;
		osn << (i+1) ;
		turn_wheel[i].set(x + "_knob_pos" + osn.str() + ".png", 110, 641);
	}
}

sub_bg_display::sub_bg_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
}



void sub_bg_display::process_input(class game& gm, const SDL_Event& event)
{
	int mx, my, mb;

	if (!myscheme.get()) throw error("sub_bg_display::process_input without scheme!");
	const scheme& s = *myscheme;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = sys().translate_position_x(event);
		my = sys().translate_position_y(event);
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (s.turn_wheel[0].is_mouse_over(mx, my, 128)) {
			turnknobdrag = TK_DIRECTION;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = sys().translate_motion_x(event);
		my = sys().translate_motion_y(event);
		mb = event.motion.state;
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (turnknobdrag != TK_NONE) {
				float& ang = turnknobang[unsigned(turnknobdrag)];
				ang += mx * TK_ANGFAC;
				switch (turnknobdrag) {
				case TK_DIRECTION:
					// bring to 0...360 degree value
					ang = myfmod(ang, 720.0f);
					//sub->set_bg_direction(ang); // fixme: set angle of player
					break;
				default:	// can never happen
					break;
				}
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		mx = sys().translate_position_x(event);
		my = sys().translate_position_y(event);
		turnknobdrag = TK_NONE;
		break;
	default:
		break;
	}
}



void sub_bg_display::display(class game& gm) const
{
  printf("display bg\n");
	sys().prepare_2d_drawing();

	// get hearing device angle from submarine, if it has one

	if (!myscheme.get()) throw error("sub_bg_display::display without scheme!");
	const scheme& s = *myscheme;

	s.background->draw(0, 0);
	s.turn_wheel[unsigned(myfmod(-turnknobang[TK_DIRECTION] * 2.0f, 90.0f)) * TK_PHASES / 90].draw();
	s.direction_ptr.draw(turnknobang[TK_DIRECTION] * 0.5f /* fixme: get angle from player*/);

	ui.draw_infopanel();
	
submarine* mysub = dynamic_cast<submarine*>(gm.get_player());
  std::map<double, sonar_operator::contact> contacts = mysub->get_sonarman().get_contacts();
  
  std::map<double, std::string> notebook_contacts;
  std::ostringstream note,comments;
  
  int posx = 780;
  int posy = 100;
  int i = 0;
  const unsigned linespc = 42;
  for (std::map<double, sonar_operator::contact>::iterator it=contacts.begin(); it!=contacts.end(); ++it)
  {
    note.str("");
    note << texts::get(810+(int)it->second.type);
    double strength = it->second.strength_dB;
		//notebook_contacts.insert(std::pair<double, string>(strength,note.str().c_str()));
		color ink = color(10,10,10);
		ostringstream bearing("");
		bearing << (int)it->first;
		// get the text for the shiptype to write down
		font_jphsl->print_wrapped(posx, posy+ linespc*i, 256-16, 20, texts::get(810+(int)it->second.type), ink);
		font_jphsl->print_wrapped(posx+180, posy+ linespc*i, 50, 20, bearing.str().c_str(), ink);
		
		// fixme: modify this part to give additionnal info provided by the device
		comments.str("");
		printf("signal dB: %d\n",(int)strength);
		// comment: very close, close, distant etc.
		if(strength > 60)
		  comments << texts::get(820);
		else if(strength > 50)
		  comments << texts::get(821);
		else if(strength > 40)
		  comments << texts::get(822);
		else if(strength > 30)
		  comments << texts::get(823);
		else //if(strength > 100)
		  comments << texts::get(823);

		font_jphsl->print_wrapped(posx+20, posy+ linespc*i+20,256-16-20, 20, comments.str().c_str(), ink);
		
    i++;
  }

	sys().unprepare_2d_drawing();
}



void sub_bg_display::enter(bool is_day)
{
	myscheme.reset(new scheme(is_day));
}



void sub_bg_display::leave()
{
	myscheme.reset();
}
