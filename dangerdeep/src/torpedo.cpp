// torpedoes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "torpedo.h"
#include "global_data.h"
#include "model.h"
#include "system.h"
#include "game.h"

torpedo::torpedo(sea_object* parent_, unsigned type_, bool usebowtubes)
{
	sea_object::init();
	parent = parent_;
	type = type_;
	position = parent->get_pos();
	heading = parent->get_heading();
	if (!usebowtubes) heading += angle(180);
	vector2 dp = heading.direction() * (parent->get_length()/2 + 3.5);
	position.x += dp.x;
	position.y += dp.y;
	head_to = heading;
	turn_rate = 1;	// most submarine simulations seem to ignore this
			// launching a torpedo will cause it to run in target direction
			// immidiately instead of turning there from the sub's heading
	length = 7;
	width = 1;
	run_length = 0;
	switch (type_) {
		case T3FAT:		// G7e FAT
			speed = kts2ms(30);
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 7500;
			break;
		case T3:	// G7e electric torpedo (T2/T3)
			speed = kts2ms(30);
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 7500;
		case T1:
		case T5:
		case T6LUT:
		case T11:		
		default:	// G7a steam torpedo
			speed = kts2ms(30);
			max_speed = kts2ms(30);
			max_rev_speed = 0;
			max_run_length = 12500;
	};
	throttle = aheadfull;
	system::sys()->add_console("torpedo created");
}

void torpedo::simulate(game& gm, double delta_time)
{
	if (parent != 0 && parent->is_dead()) parent = 0;
	sea_object::simulate(gm, delta_time);
	if (is_defunct() || is_dead()) return;
	if (type == T3FAT) {	// FAT, test hack
		if (run_length > 1600) {
			rudder_right(0.5);
		}
	}
	run_length += speed * delta_time;
	if (run_length > max_run_length) {
		kill();
		return;
	}

	// check for collisions with other subs or ships
	if (run_length > 10) {	// avoid collision with parent after initial creation
		bool runlengthfailure = (run_length < 250);
		bool failure = false;	// calculate additional probability of torpedo failure
		if (gm.check_torpedo_hit(this, runlengthfailure, failure))
			kill();
	}
}

void torpedo::display(void) const
{
	torpedo_g7->display();
}

pair<angle, bool> torpedo::lead_angle(double target_speed, angle angle_on_the_bow) const
{
	double sla = target_speed*angle_on_the_bow.sin()/speed;
	if (fabs(sla) >= 1.0) return make_pair(angle(), false);
	return make_pair(angle::from_rad(asin(sla)), true);
}

double torpedo::expected_run_time(angle lead_angle,
	angle angle_on_the_bow, double target_range) const
{
	angle ang = angle(180) - angle_on_the_bow - lead_angle;
	return (angle_on_the_bow.sin() * target_range) / (speed * ang.sin());
}

//#include <sstream>
bool torpedo::adjust_head_to(const sea_object* target, bool usebowtubes)
{
	if (parent == 0) return false;
	pair<angle, double> br = parent->bearing_and_range_to(target);
	angle ab = parent->estimate_angle_on_the_bow(br.first, target->get_heading());
	pair<angle, bool> la = lead_angle(target->get_speed(), ab);
	if (la.second) {
		angle gyro_angle = br.first - parent->get_heading() + la.first;
		angle headto = parent->get_heading() + gyro_angle;
//ostringstream os;
//	os << "lead angle " << la.first.value() << " gyro angle " << gyro_angle.value() << ", trp head to " << headto.value()
//	<< ", expected run time " << expected_run_time(la.first, ab, br.second);
//	system::sys()->add_console(os.str());
		double fga = fabs(gyro_angle.value_pm180());
		if (usebowtubes) {
			if (fga <= 90) {
				head_to_ang(headto, gyro_angle.value_pm180() < 0);
				return true;
			}
		} else {	// stern tubes
			if (fga >= 90) {
				head_to_ang(headto, gyro_angle.value_pm180() > 0);
				return true;
			}
		}
	}
	return false;
}
