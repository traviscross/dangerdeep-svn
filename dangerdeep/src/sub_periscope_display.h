// user display: submarine's periscope
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_PERISCOPE_DISPLAY_H
#define SUB_PERISCOPE_DISPLAY_H

#include "freeview_display.h"

class sub_periscope_display : public freeview_display
{
	void pre_display(class game& gm) const;
	projection_data get_projection_data(class game& gm) const;
	void post_display(class game& gm) const;

	class image* periscopeimg;

	bool zoomed;	// use 1,5x (false) or 6x (true) zoom

public:
	sub_periscope_display(class user_interface& ui_);
	virtual ~sub_periscope_display();

	//overload for zoom key handling ('y') and TDC input
	//virtual void process_input(class game& gm, const SDL_Event& event);
};

#endif
