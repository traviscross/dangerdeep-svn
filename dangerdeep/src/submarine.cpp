// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "model.h"
#include "game.h"
#include "tokencodes.h"
#include "sensors.h"
#include "date.h"
#include "submarine.h"
#include "submarine_VIIc.h"
#include "submarine_XXI.h"

submarine::submarine() : ship(), dive_speed(0.0f), permanent_dive(false),
	dive_to(0.0f), max_dive_speed(1.0f), dive_acceleration(0.0f), scopeup(false),
	max_depth(150.0f), periscope_depth(12.0f), snorkel_depth(10.0f),
	hassnorkel (false), snorkel_up(false),
	battery_level ( 1.0f ), battery_value_a ( 0.0f ), battery_value_t ( 1.0f ),
	battery_recharge_value_a ( 0.0f ), battery_recharge_value_t ( 1.0f ),
	damageable_parts(nr_of_damageable_parts, unused)
{
	// set all common damageable parts to "no damage"
	for (unsigned i = 0; i < unsigned(outer_stern_tubes); ++i)
		damageable_parts[i] = damage_status(i%5+2); //none; // experimental hack fixme
}
	
bool submarine::parse_attribute(parser& p)
{
	if (ship::parse_attribute(p)) return true;
	switch (p.type()) {
		case TKN_SCOPEUP:
			p.consume();
			p.parse(TKN_ASSIGN);
			scopeup = p.parse_bool();
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_MAXDEPTH:
			p.consume();
			p.parse(TKN_ASSIGN);
			max_depth = p.parse_number();
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_TORPEDOES:
			p.consume();
			p.parse(TKN_SLPARAN);
			for (unsigned i = 0; i < torpedoes.size(); ++i) {
				switch (p.type()) {
					case TKN_TXTNONE: torpedoes[i].status = stored_torpedo::st_empty; break;
					case TKN_T1: torpedoes[i] = stored_torpedo(torpedo::T1); break;
					case TKN_T3: torpedoes[i] = stored_torpedo(torpedo::T3); break;
					case TKN_T5: torpedoes[i] = stored_torpedo(torpedo::T5); break;
					case TKN_T3FAT: torpedoes[i] = stored_torpedo(torpedo::T3FAT); break;
					case TKN_T6LUT: torpedoes[i] = stored_torpedo(torpedo::T6LUT); break;
					case TKN_T11: torpedoes[i] = stored_torpedo(torpedo::T11); break;
					default: p.error("Expected torpedo type");
				}
				p.consume();
				if (p.type() == TKN_SRPARAN) break;
				p.parse(TKN_COMMA);
			}
			p.parse(TKN_SRPARAN);
			break;
		case TKN_SNORKEL:
			p.consume();
			p.parse(TKN_ASSIGN);
			hassnorkel = p.parse_bool();
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_BATTERY:
			p.consume ();
			p.parse ( TKN_ASSIGN );
			battery_level = p.parse_number () / 100.0f;
			p.parse ( TKN_SEMICOLON );
			break;
		default: return false;
	}

	// Activate electric engine if submerged.
	if (is_submerged())
	{
		electric_engine = true;
	}
    
	return true;
}

submarine* submarine::create(submarine::types type_)
{
	switch (type_) {
		case typeVIIc: return new submarine_VIIc();
		case typeXXI: return new submarine_XXI();
	}
	return 0;
}

submarine* submarine::create(parser& p)
{
	p.parse(TKN_SUBMARINE);
	int t = p.type();
	p.consume();
	switch (t) {
		case TKN_TYPEVIIC: return new submarine_VIIc(p);
		case TKN_TYPEXXI: return new submarine_XXI(p);
	}
	return 0;
}

bool submarine::transfer_torpedo(unsigned from, unsigned to, double timeneeded)
{
	if (torpedoes[from].status == stored_torpedo::st_loaded &&
		torpedoes[to].status == stored_torpedo::st_empty) {
		torpedoes[to].type = torpedoes[from].type;
		torpedoes[from].status = stored_torpedo::st_unloading;
		torpedoes[to].status = stored_torpedo::st_reloading;
		torpedoes[from].associated = to;
		torpedoes[to].associated = from;
		torpedoes[from].remaining_time =
			torpedoes[to].remaining_time = timeneeded;
		return true;
	}
	return false;
}

int submarine::find_stored_torpedo(bool usebow)
{
	pair<unsigned, unsigned> indices = (usebow) ? get_bow_storage_indices() : get_stern_storage_indices();
	int tubenr = -1;
	for (unsigned i = indices.first; i < indices.second; ++i) {
		if (torpedoes[i].status == stored_torpedo::st_loaded) {	// loaded
			tubenr = i; break;
		}
	}
	return tubenr;
}

void submarine::simulate(class game& gm, double delta_time)
{
	ship::simulate(gm, delta_time);

	// calculate new depth (fixme this is not physically correct)
	double delta_depth = dive_speed * delta_time;

	// Activate or deactivate electric engines.
	if ((position.z > -SUBMARINE_SUBMERGED_DEPTH) &&
		(position.z+delta_depth < -SUBMARINE_SUBMERGED_DEPTH))
	{
		// Activate electric engine.
		electric_engine = true;
	}
	else if ((position.z < -SUBMARINE_SUBMERGED_DEPTH) &&
		(position.z+delta_depth > -SUBMARINE_SUBMERGED_DEPTH))
	{
		// Activate diesel engine.
		electric_engine = false;
	}

	if (dive_speed != 0) {
		if (permanent_dive) {
			position.z += delta_depth;
		} else {
			double fac = (dive_to - position.z)/delta_depth;
			if (0 <= fac && fac <= 1) {
				position.z = dive_to;
				planes_middle();
			} else {
				position.z += delta_depth;
			}
		}
	}
	if (position.z > 0) {
		position.z = 0;
		dive_speed = 0;
	}

	// fixme: the faster the sub goes, the faster it can dive.

	// fixme: this is simple and not realistic. and the values are just guessed		
//	double water_resistance = -dive_speed * 0.5;
//	dive_speed += delta_time * (2*dive_acceleration + water_resistance);
//	if (dive_speed > max_dive_speed)
//		dive_speed = max_dive_speed;
//	if (dive_speed < -max_dive_speed)
//		dive_speed = -max_dive_speed;
		
	if (-position.z > max_depth)
		kill();
		
	// torpedo transfer
	for (unsigned i = 0; i < torpedoes.size(); ++i) {
		stored_torpedo& st = torpedoes[i];
		if (st.status == stored_torpedo::st_reloading ||
			st.status == stored_torpedo::st_unloading) { // reloading/unloading
			st.remaining_time -= delta_time;
			if (st.remaining_time <= 0) {
				if (st.status == stored_torpedo::st_reloading) {	// reloading
					st.status = stored_torpedo::st_loaded;	// loading
					torpedoes[st.associated].status = stored_torpedo::st_empty;	// empty
				} else {		// unloading
					st.status = stored_torpedo::st_empty;	// empty
					torpedoes[st.associated].status = stored_torpedo::st_loaded;	// loaded
				}
			}
		}
	}

	// automatic reloading if desired	
	if (true /*automatic_reloading*/) {
		pair<unsigned, unsigned> bow_tube_indices = get_bow_tube_indices();
		pair<unsigned, unsigned> stern_tube_indices = get_stern_tube_indices();
		for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
			if (torpedoes[i].status == 0) {
				int reload = find_stored_torpedo(true);		// bow
				if (reload >= 0) {
					transfer_torpedo(reload, i);
				}
			}
		}
		for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
			if (torpedoes[i].status == 0) {
				int reload = find_stored_torpedo(false);	// stern
				if (reload >= 0) {
					transfer_torpedo(reload, i);
				}
			}
		}
	}
}

double submarine::get_max_speed(void) const
{
	double ms;

	if ( is_electric_engine() )
	{
		ms = max_submerged_speed;
	}
	else
	{
		ms = ship::get_max_speed ();

		// When submarine is submerged and snorkel is used the maximum
		// diesel speed is halved.
		if ( has_snorkel() && is_submerged () && snorkel_up )
			ms *= 0.5f;
	}

	return ms;
}

float submarine::surface_visibility(const vector2& watcher) const
{
	double depth = get_depth();
	float dive_factor = 0.0f;

	if ( depth >= 0.0f && depth < 10.0f )
	{
		dive_factor = 0.1f * ( 10.0f - depth ) * vis_cross_section_factor *
			get_profile_factor ( watcher );
	}

	// Some modifiers when submarine is submerged.
	if ( depth >= 10.0f && depth <= periscope_depth )
	{
		double diverse_modifiers = 0.0f;

		// Periscope.
		if ( is_scope_up () )
		{
			// The visibility of the periscope also depends on the speed its moves
			// through the water. A fast moving periscope with water splashed is
			// much farther visible than a still standing one.
			diverse_modifiers += CROSS_SECTION_VIS_PERIS;
		}

		if ( is_snorkel_up () )
		{
			// A snorkel is much larger than a periscope.
			diverse_modifiers += 3.0f * CROSS_SECTION_VIS_PERIS;
		}

		dive_factor += diverse_modifiers * ( 0.5f + 0.5f * speed / max_speed );
	}

	return dive_factor;
}

float submarine::sonar_visibility ( const vector2& watcher ) const
{
	double depth = get_depth();
	float diveFactor = 0.0f;

	if ( depth > 10.0f )
	{
		diveFactor = 1.0f;
	}
	else if ( (depth > SUBMARINE_SUBMERGED_DEPTH ) && ( depth < 10.0f ) )
	{
		// Submarine becomes visible for active sonar system while
		// diving process.
		diveFactor = 0.125f * (depth - SUBMARINE_SUBMERGED_DEPTH);
	}

	diveFactor *= sonar_cross_section_factor * get_profile_factor ( watcher );

	return diveFactor;
}

void submarine::planes_up(double amount)
{
//	dive_acceleration = -1;
	dive_speed = max_dive_speed;
	permanent_dive = true;
}

void submarine::planes_down(double amount)
{
//	dive_acceleration = 1;
	dive_speed = -max_dive_speed;
	permanent_dive = true;
}

void submarine::planes_middle(void)
{
//	dive_acceleration = 0;
	dive_speed = 0;
	permanent_dive = false;
	dive_to = position.z;
}

void submarine::dive_to_depth(unsigned meters)
{
	dive_to = -int(meters);
	permanent_dive = false;
	dive_speed = (dive_to < position.z) ? -max_dive_speed : max_dive_speed;
}

bool submarine::fire_torpedo(class game& gm, int tubenr, sea_object* target,
	const angle& manual_lead_angle)
{
	pair<unsigned, unsigned> bow_tube_indices = get_bow_tube_indices();
	pair<unsigned, unsigned> stern_tube_indices = get_stern_tube_indices();

	bool usebowtubes = true;
	if (tubenr < 0) {
		if (target != 0) {
			angle a = angle(target->get_pos().xy() - get_pos().xy())
				- get_heading() + manual_lead_angle;
			if (a.ui_abs_value180() > 90)
				usebowtubes = false;
		}
	} else {
		if (tubenr >= bow_tube_indices.first && tubenr < bow_tube_indices.second) {
			tubenr -= bow_tube_indices.first;
			usebowtubes = true;
		} else if (tubenr >= stern_tube_indices.first && tubenr < stern_tube_indices.second) {
			tubenr -= stern_tube_indices.first;
			usebowtubes = false;
		} else {
			return false;	// illegal tube nr
		}
	}

	unsigned torpnr = 0xffff;	// some high illegal value
	if (tubenr < 0) {
		if (usebowtubes) {
			for (unsigned i = bow_tube_indices.first; i < bow_tube_indices.second; ++i) {
				if (torpedoes[i].status == stored_torpedo::st_loaded) {
					torpnr = i;
					break;
				}
			}
		} else {
			for (unsigned i = stern_tube_indices.first; i < stern_tube_indices.second; ++i) {
				if (torpedoes[i].status == stored_torpedo::st_loaded) {
					torpnr = i;
					break;
				}
			}
		}
	} else {
		unsigned d = (usebowtubes) ? bow_tube_indices.second - bow_tube_indices.first :
			stern_tube_indices.second - stern_tube_indices.first;
		if (tubenr >= 0 && tubenr < d)
		{
			unsigned i = tubenr + ((usebowtubes) ? bow_tube_indices.first : stern_tube_indices.first);

			if ( torpedoes[i].status == stored_torpedo::st_loaded )
				torpnr = i;
		}
	}
	if (torpnr == 0xffff)
		return false;
		
	torpedo* t = new torpedo(this, torpedoes[torpnr].type, usebowtubes);
	if (target) {
		if (t->adjust_head_to(target, usebowtubes, manual_lead_angle)) {
			gm.spawn_torpedo(t);
		} else {
			// gyro angle invalid
			delete t;
			return false;
		}
	} else {
		gm.spawn_torpedo(t);
	}
	torpedoes[torpnr].type = torpedo::none;
	torpedoes[torpnr].status = stored_torpedo::st_empty;
	return true;
}

double submarine::get_noise_factor () const
{
	double noisefac = 1.0f;

	// Get engine noise factor.
	noisefac = sea_object::get_noise_factor ();

	// Noise level modification because of diesel/electric engine.
	if ( is_electric_engine () )
	{
		// This is an empirical value.
		noisefac *= 0.007f;
	}
	else
	{
		// When a submarine uses its snorkel its maximum diesel speed is
		// reduced by 50%. This reduces the noise level returned by method
		// sea_object::get_noise_factor and must be corrected here by
		// multiply the actual noise factor with 2.
		if ( has_snorkel() && is_submerged () && snorkel_up )
			noisefac *= 2.0f;
	}

	return noisefac;
}

void submarine::calculate_fuel_factor ( double delta_time )
{
	if ( electric_engine )
	{
		if ( battery_level >= 0.0f && battery_level <= 1.0f )
			battery_level -= delta_time * get_battery_consumption_rate ();
	}
	else
	{
		ship::calculate_fuel_factor ( delta_time );

		// Recharge battery.
		if ( battery_level >= 0.0f && battery_level <= 1.0f )
			battery_level += delta_time * get_battery_recharge_rate ();
	}
}

bool submarine::set_snorkel_up ( bool snorkel_up )
{
	// Snorkel can be toggled only when it is available 
	// and the submarine is at least at snorkel depth.
	if ( has_snorkel() && get_depth () <= snorkel_depth )
	{
		this->snorkel_up = snorkel_up;

		// Activate diesel or electric engines if snorkel is up or down.
		if ( snorkel_up )
			electric_engine = false;
		else
			electric_engine = true;

		return true;
	}

	return false;
}
