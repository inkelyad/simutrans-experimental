/*
 * Spieleinstellungen
 *
 * Hj. Malthaner
 *
 * April 2000
 */

#include "einstellungen.h"
#include "umgebung.h"
#include "../simconst.h"
#include "../simtools.h"
#include "../simtypes.h"
#include "../simdebug.h"
#include "../simworld.h"
#include "../bauer/wegbauer.h"
#include "../besch/weg_besch.h"
#include "../utils/simstring.h"
#include "../utils/float32e8_t.h"
#include "../vehicle/simvehikel.h"
#include "../player/simplay.h"
#include "loadsave.h"
#include "tabfile.h"
#include "translator.h"
#include "../besch/intro_dates.h"

#include "../tpl/minivec_tpl.h"


#define NEVER 0xFFFFU


settings_t::settings_t() :
	filename(""),
	heightfield("")
{
	groesse_x = 256;
	groesse_y = 256;

	nummer = 33;

	/* new setting since version 0.85.01
	 * @author prissi
	 */
	land_industry_chains = 4;
	tourist_attractions = 16;

	anzahl_staedte = 16;
	mittlere_einwohnerzahl = 1600;

	station_coverage_size = 2;

	verkehr_level = 5;

	show_pax = true;

	// default climate zones
	set_default_climates( );
	winter_snowline = 7;	// not mediterran
	grundwasser = -2*Z_TILE_STEP;            //25-Nov-01        Markus Weber    Added

	max_mountain_height = 160;                  //can be 0-160.0  01-Dec-01        Markus Weber    Added
	map_roughness = 0.6;                        //can be 0-1      01-Dec-01        Markus Weber    Added

	river_number = 16;
	min_river_length = 16;
	max_river_length = 256;

	// forest setting ...
	forest_base_size = 36; 	// Base forest size - minimal size of forest - map independent
	forest_map_size_divisor = 38;	// Map size divisor - smaller it is the larger are individual forests
	forest_count_divisor = 16;	// Forest count divisor - smaller it is, the more forest are generated
	forest_inverse_spare_tree_density = 5;	// Determins how often are spare trees going to be planted (works inversly)
	max_no_of_trees_on_square = 3;	// Number of trees on square 2 - minimal usable, 3 good, 5 very nice looking
	tree_climates = 0;	// bit set, if this climate is to be covered with trees entirely
	no_tree_climates = 0;	// bit set, if this climate is to be void of random trees
	no_trees = false;	// if set, no trees at all, may be useful for low end engines

	// some settigns more
	allow_player_change = true;
	use_timeline = 2;
	starting_year = 1930;
	starting_month = 0;
	bits_per_month = 20;

	beginner_mode = false;
	beginner_price_factor = 1500;

	rotation = 0;

	origin_x = origin_y = 0;

	// passenger manipulation factor (=16 about old value)
	passenger_factor = 10;

	electric_promille = 1000;

	// town growth factors
	passenger_multiplier = 40;
	mail_multiplier = 20;
	goods_multiplier = 20;
	electricity_multiplier = 20;

	// Also there are size dependen factors (0 causes crash !)
	growthfactor_small = 400;
	growthfactor_medium = 200;
	growthfactor_large = 100;

	industry_increase = 2000;
	city_isolation_factor = 1;

	factory_worker_percentage = 33;
	tourist_percentage = 16;

	factory_worker_radius = 77;
	// try to have at least a single town connected to a factory
	factory_worker_minimum_towns = 1;
	// not more than four towns should supply to a factory
	factory_worker_maximum_towns = 4;

	factory_arrival_periods = 4;

	factory_enforce_demand = true;

#ifdef OTTD_LIKE
	/* prissi: crossconnect all factories (like OTTD and similar games) */
	crossconnect_factories=true;
	crossconnect_factor=100;
#else
	/* prissi: crossconnect a certain number */
	crossconnect_factories=false;
	crossconnect_factor=33;
#endif

	/* minimum spacing between two factories */
	factory_spacing = 6;

	/* prissi: do not distribute goods to overflowing factories */
	just_in_time = true;

	fussgaenger = true;
	stadtauto_duration = 36;	// three years

	// to keep names consistent
	numbered_stations = false;

	num_city_roads = 0;
	num_intercity_roads = 0;

	max_route_steps = 1000000;
	max_transfers = 7;
	max_hops = 300;

	// Eighteen hours - the control of real journey times
	// should be the journey time tolerance feature, not
	// the maximum waiting: discarding should be a last
	// resort, not an integral part of the journey time
	// measurement mechanics.
	// 19440 / 18 = 1080 (or 18 hours).
	passenger_max_wait = 19440;

	// 4 is faster; 2 is more accurate.
	// Not recommended to use anything other than 2 or 4
	// @author: jamespetts
	max_rerouting_interval_months = 2;

	/* multiplier for steps on diagonal:
	 * 1024: TT-like, faktor 2, vehicle will be too long and too fast
	 * 724: correct one, faktor sqrt(2)
	 */
	pak_diagonal_multiplier = 724;

	strcpy( language_code_names, "en" );

	// default AIs active
	for(  int i=0;  i<MAX_PLAYER_COUNT;  i++  ) {
		if(  i<2  ) {
			automaten[i] = true;
			spieler_type[i] = spieler_t::HUMAN;
		}
		else if(  i==3  ) {
			automaten[i] = true;
			spieler_type[i] = spieler_t::AI_PASSENGER;
		}
		else if(  i==6  ) {
			automaten[i] = true;
			spieler_type[i] = spieler_t::AI_GOODS;
		}
		else {
			automaten[i] = false;
			spieler_type[i] = spieler_t::EMPTY;
		}
		// undefined colors
		default_player_color[i][0] = 255;
		default_player_color[i][1] = 255;
	}
	default_player_color_random = false;

	/* the big cost section */
	freeplay = false;
	starting_money = 20000000;
	for(  int i=0; i<10; i++  ) {
		startingmoneyperyear[i].year = 0;
		startingmoneyperyear[i].money = 0;
		startingmoneyperyear[i].interpol = 0;
	}

	maint_building = 5000;	// normal buildings

	// stop buildings
	cst_multiply_dock=-50000;
	cst_multiply_station=-60000;
	cst_multiply_roadstop=-40000;
	cst_multiply_airterminal=-300000;
	cst_multiply_post=-30000;
	cst_multiply_headquarter=-100000;
	cst_depot_rail=-100000;
	cst_depot_road=-130000;
	cst_depot_ship=-250000;
	cst_depot_air=-500000;
	// alter landscape
	cst_buy_land=-10000;
	cst_alter_land=-100000;
	cst_set_slope=-250000;
	cst_found_city=-500000000;
	cst_multiply_found_industry=-2000000;
	cst_remove_tree=-10000;
	cst_multiply_remove_haus=-100000;
	cst_multiply_remove_field=-500000;
	// cost for transformers
	cst_transformer=-250000;
	cst_maintain_transformer=-2000;

	// costs for the way searcher
	way_count_straight=1;
	way_count_curve=2;
	way_count_double_curve=6;
	way_count_90_curve=15;
	way_count_slope=10;
	way_count_tunnel=8;
	way_max_bridge_len=15;
	way_count_leaving_road=25;

	// defualt: joined capacities
	seperate_halt_capacities = false;

	// Cornering settings
	// @author: jamespetts
	max_corner_limit[waytype_t(road_wt)] = 200;
	min_corner_limit[waytype_t(road_wt)] = 30;
	max_corner_adjustment_factor[waytype_t(road_wt)] = 75;
	min_corner_adjustment_factor[waytype_t(road_wt)] = 97;
	min_direction_steps[waytype_t(road_wt)] = 3;
	max_direction_steps[waytype_t(road_wt)] = 6;
	curve_friction_factor[waytype_t(road_wt)] = 0;

	max_corner_limit[waytype_t(track_wt)] = 425;
	min_corner_limit[waytype_t(track_wt)] = 45;
	max_corner_adjustment_factor[waytype_t(track_wt)] = 50;
	min_corner_adjustment_factor[waytype_t(track_wt)] = 95;
	min_direction_steps[waytype_t(track_wt)] = 4;
	max_direction_steps[waytype_t(track_wt)] = 14;
	curve_friction_factor[waytype_t(track_wt)] = 0;

	max_corner_limit[waytype_t(tram_wt)] = 250;
	min_corner_limit[waytype_t(tram_wt)] = 30;
	max_corner_adjustment_factor[waytype_t(tram_wt)] = 60;
	min_corner_adjustment_factor[waytype_t(tram_wt)] = 90;	
	min_direction_steps[waytype_t(tram_wt)] = 3;
	max_direction_steps[waytype_t(tram_wt)] = 10;
	curve_friction_factor[waytype_t(tram_wt)] = 0;

	max_corner_limit[waytype_t(monorail_wt)] = 425;
	min_corner_limit[waytype_t(monorail_wt)] = 75;
	max_corner_adjustment_factor[waytype_t(monorail_wt)] = 50;
	min_corner_adjustment_factor[waytype_t(monorail_wt)] = 85;	
	min_direction_steps[waytype_t(monorail_wt)] = 5;
	max_direction_steps[waytype_t(monorail_wt)] = 16;
	curve_friction_factor[waytype_t(monorail_wt)] =0;

	max_corner_limit[waytype_t(maglev_wt)] = 500;
	min_corner_limit[waytype_t(maglev_wt)] = 50;
	max_corner_adjustment_factor[waytype_t(maglev_wt)] = 40;
	min_corner_adjustment_factor[waytype_t(maglev_wt)] = 80;
	min_direction_steps[waytype_t(maglev_wt)] = 4;
	max_direction_steps[waytype_t(maglev_wt)] = 16;
	curve_friction_factor[waytype_t(maglev_wt)] = 0;

	max_corner_limit[waytype_t(narrowgauge_wt)] = 250;
	min_corner_limit[waytype_t(narrowgauge_wt)] = 30;
	max_corner_adjustment_factor[waytype_t(narrowgauge_wt)] = 67;
	min_corner_adjustment_factor[waytype_t(narrowgauge_wt)] = 92;
	min_direction_steps[waytype_t(narrowgauge_wt)] = 3;
	max_direction_steps[waytype_t(narrowgauge_wt)] = 8;
	curve_friction_factor[waytype_t(narrowgauge_wt)] = 0;

	// Revenue calibration settings
	// @author: jamespetts
	min_bonus_max_distance = 4;
	max_bonus_min_distance = 256;
	median_bonus_distance = 0;
	max_bonus_multiplier_percent = 300;
	meters_per_tile = 250;
	tolerable_comfort_short = 15;
	tolerable_comfort_median_short = 60;
	tolerable_comfort_median_median = 100;
	tolerable_comfort_median_long = 160;
	tolerable_comfort_long = 220;
	tolerable_comfort_short_minutes = 2;
	tolerable_comfort_median_short_minutes = 30;
	tolerable_comfort_median_median_minutes = 120;
	tolerable_comfort_median_long_minutes = 300;
	tolerable_comfort_long_minutes = 720;
	max_luxury_bonus_differential = 75;
	max_luxury_bonus_percent = 50;
	max_discomfort_penalty_differential = 200;
	max_discomfort_penalty_percent = 95;

	catering_min_minutes = 60;
	catering_level1_minutes = 90;
	catering_level1_max_revenue = 150;
	catering_level2_minutes = 120;
	catering_level2_max_revenue = 250;
	catering_level3_minutes = 150;
	catering_level3_max_revenue = 350;
	catering_level4_minutes = 240;
	catering_level4_max_revenue = 400;
	catering_level5_minutes = 300;
	catering_level5_max_revenue = 475;

	tpo_min_minutes = 120;
	tpo_revenue = 300;

	// Obsolete vehicles running costs adjustment
	obsolete_running_cost_increase_percent = 400; //Running costs will be this % of normal costs after vehicle has been obsolete
	obsolete_running_cost_increase_phase_years = 20; //for this number of years.

	// Passenger destination ranges

	local_passengers_min_distance = 0;
	local_passengers_max_distance = 16;
	midrange_passengers_min_distance = 0;
	midrange_passengers_max_distance = 96;
	longdistance_passengers_min_distance = 0;
	longdistance_passengers_max_distance = 4096;

	// Passenger routing settings
	passenger_routing_packet_size = 7;
	max_alternative_destinations = 3;

	always_prefer_car_percent = 10;
	base_car_preference_percent = 90;
	congestion_density_factor = 12;

	//@author: jamespetts
	// Passenger routing settings
	passenger_routing_packet_size = 7;

	//@author: jamespetts
	// Factory retirement settings
	factory_max_years_obsolete = 30;

	//@author: jamespetts
	// Insolvency and debt settings
	interest_rate_percent = 10;
	allow_bankruptsy  = 0;
	allow_purhcases_when_insolvent  = 0;

	// Reversing settings
	// @author: jamespetts
	unit_reverse_time = 0;
	hauled_reverse_time = 0;
	turntable_reverse_time = 0;

	// Global power factor
	// @author: jamespetts
	global_power_factor_percent = 100;

	avoid_overcrowding = false;

	// How and whether weight limits are enforced.
	// @author: jamespetts
	enforce_weight_limits = 1;

	speed_bonus_multiplier_percent = 100;

	allow_buying_obsolete_vehicles = true;

	// default: load also private extensions of the pak file
	with_private_paks = true;

	// The default is a selective refresh.
	default_path_option = 2;

	// The defaults for journey time tolerance.
	// Applies to passengers only.
	// NOTE: The *maximum* numbers need to be 
	// added to the minimum numbers to produce
	// the true maximum.
	// @author: jamespetts

	min_local_tolerance = 45 * 10; // 3/4 of an hour.
	max_local_tolerance = 60 * 10; // One hour
	//max_local_tolerance = 15; // One hour
	min_midrange_tolerance = 60 * 10;
	max_midrange_tolerance = 180 * 10; //: Three hours
	//max_midrange_tolerance = 120;
	min_longdistance_tolerance = 180 * 10;
	max_longdistance_tolerance = 330 * 10; // Five and a half hours
	//max_longdistance_tolerance = 150;

	max_walking_distance = 4;

	used_vehicle_reduction = 0;

	// some network thing to keep client in sync
	random_counter = 0;	// will be set when actually saving
	frames_per_second = 20;
	frames_per_step = 4;

	quick_city_growth = false;
	assume_everywhere_connected_by_road=false;

	city_threshold_size = 1000;
	capital_threshold_size = 10000;
	
	for(uint8 i = 0; i < 17; i ++)
	{
		if(i != road_wt)
		{
			default_increase_maintenance_after_years[i] = 30;
		}
		else
		{
			default_increase_maintenance_after_years[i] = 15;
		}
	}
	server_frames_ahead = 4;

	spacing_shift_mode = SPACING_SHIFT_PER_STOP;
	spacing_shift_divisor = 24*60;
}

settings_t::~settings_t()
{
	/*ITERATE(livery_schemes, i)
	{
		delete livery_schemes[i];
	}*/
	//livery_schemes = NULL;
}


void settings_t::set_default_climates()
{
	static sint16 borders[MAX_CLIMATES] = { 0, 0, 0, 3, 6, 8, 10, 10 };
	memcpy( climate_borders, borders, sizeof(sint16)*MAX_CLIMATES );
}



void settings_t::rdwr(loadsave_t *file)
{
	xml_tag_t e( file, "einstellungen_t" );

	if(file->get_version() < 86000) {
		uint32 dummy;

		file->rdwr_long(groesse_x );
		groesse_y = groesse_x;

		file->rdwr_long(nummer );

		// to be compatible with previous savegames
		dummy = 0;
		file->rdwr_long(dummy );	//dummy!
		land_industry_chains = 6;
		tourist_attractions = 12;

		// now towns
		mittlere_einwohnerzahl = 1600;
		dummy =  anzahl_staedte;
		file->rdwr_long(dummy );
		dummy &= 127;
		if(dummy>63) {
			dbg->warning("settings_t::rdwr()", "This game was saved with too many cities! (%i of maximum 63). Simutrans may crash!", dummy);
		}
		anzahl_staedte = dummy;

		// rest
		file->rdwr_long(dummy );	// scroll ingnored
		file->rdwr_long(verkehr_level );
		file->rdwr_long(show_pax );
		dummy = grundwasser;
		file->rdwr_long(dummy );
		grundwasser = (sint16)(dummy/16)*Z_TILE_STEP;
		file->rdwr_double(max_mountain_height );
		file->rdwr_double(map_roughness );

		station_coverage_size = 3;
		beginner_mode = false;
		rotation = 0;
	}
	else {
		// newer versions
		file->rdwr_long(groesse_x );
		file->rdwr_long(nummer );

		// industries
		file->rdwr_long(land_industry_chains );
		if(file->get_version()<99018) {
			uint32 dummy;	// was city chains
			file->rdwr_long(dummy );
		}
		else {
			file->rdwr_long( electric_promille );
		}
		file->rdwr_long(tourist_attractions );

		// now towns
		file->rdwr_long(mittlere_einwohnerzahl );
		file->rdwr_long(anzahl_staedte );

		// rest
		if(file->get_version() < 101000) {
			uint32 dummy;	// was scroll dir
			file->rdwr_long(dummy );
		}
		file->rdwr_long(verkehr_level );
		file->rdwr_long(show_pax );
		sint32 dummy = grundwasser/Z_TILE_STEP;
		file->rdwr_long(dummy );
		if(file->get_version() < 99005) {
			grundwasser = (sint16)(dummy/16)*Z_TILE_STEP;
		}
		else {
			grundwasser = (sint16)dummy*Z_TILE_STEP;
		}
		file->rdwr_double(max_mountain_height );
		file->rdwr_double(map_roughness );

		if(file->get_version() >= 86003) {
			dummy = station_coverage_size;
			file->rdwr_long(dummy );
			station_coverage_size = (uint16)dummy;
		}

		if(file->get_version() >= 86006) {
			// handle also size on y direction
			file->rdwr_long(groesse_y );
		}
		else {
			groesse_y = groesse_x;
		}

		if(file->get_version() >= 86011) {
			// some more settings
			file->rdwr_byte(allow_player_change );
			file->rdwr_byte(use_timeline );
			file->rdwr_short(starting_year );
		}
		else {
			allow_player_change = 1;
			use_timeline = 1;
			starting_year = 1930;
		}

		if(file->get_version()>=88005) {
			file->rdwr_short(bits_per_month );
		}
		else {
			bits_per_month = 18;
		}

		if(file->get_version()>=89003) {
			file->rdwr_bool(beginner_mode );
		}
		else {
			beginner_mode = false;
		}
		if(file->get_version()>=89004) {
			file->rdwr_bool(just_in_time );
		}
		// rotation of the map with respect to the original value
		if(file->get_version()>=99015) {
			file->rdwr_byte(rotation );
		}
		else {
			rotation = 0;
		}

		// clear the name when loading ...
		if(file->is_loading()) {
			filename = "";
		}

		// climate corders
		if(file->get_version()>=91000) {
			for(  int i=0;  i<8;  i++ ) {
				file->rdwr_short(climate_borders[i] );
			}
			file->rdwr_short(winter_snowline );
		}

		// since vehicle will need realignment afterwards!
		if(file->get_version()<=99018) {
			vehikel_basis_t::set_diagonal_multiplier( pak_diagonal_multiplier, 1024 );
		}
		else {
			uint16 old_multiplier = pak_diagonal_multiplier;
			file->rdwr_short(old_multiplier );
			vehikel_basis_t::set_diagonal_multiplier( pak_diagonal_multiplier, old_multiplier );
			// since vehicle will need realignment afterwards!
		}

		if(file->get_version()>=101000) {
			// game mechanics
			file->rdwr_short(origin_x );
			file->rdwr_short(origin_y );

			file->rdwr_long(passenger_factor );

			// town grow stuff
			if(file->get_version()>102001) {
				file->rdwr_long(passenger_multiplier );
				file->rdwr_long(mail_multiplier );
				file->rdwr_long(goods_multiplier );
				file->rdwr_long(electricity_multiplier );
				file->rdwr_long(growthfactor_small );
				file->rdwr_long(growthfactor_medium );
				file->rdwr_long(growthfactor_large );
				file->rdwr_short(factory_worker_percentage );
				file->rdwr_short(tourist_percentage );
				file->rdwr_short(factory_worker_radius );
			}

			file->rdwr_long(electric_promille );

			file->rdwr_short(factory_spacing );
			file->rdwr_bool(crossconnect_factories );
			file->rdwr_short(crossconnect_factor );

			file->rdwr_bool(fussgaenger );
			file->rdwr_long(stadtauto_duration );

			file->rdwr_bool( numbered_stations);
			if(  file->get_version()<=102002 || (file->get_experimental_version() < 8 && file->get_experimental_version() != 0))
			{
				if(  file->is_loading()  ) 
				{
					num_city_roads = 1;
					city_roads[0].intro = 0;
					city_roads[0].retire = 0;
					// intercity roads were not saved in old savegames
					num_intercity_roads = 0;
				}
				file->rdwr_str(city_roads[0].name, lengthof(city_roads[0].name) );
			}
			else 
			{
				// several roads ...
				file->rdwr_short(num_city_roads );
				if(  num_city_roads>=10  ) {
					dbg->fatal("settings_t::rdwr()", "Too many (%i) city roads!", num_city_roads);
				}
				for(  int i=0;  i<num_city_roads;  i++  ) {
					file->rdwr_str(city_roads[i].name, lengthof(city_roads[i].name) );
					file->rdwr_short(city_roads[i].intro );
					file->rdwr_short(city_roads[i].retire );
				}
				// several intercity roads ...
				file->rdwr_short(num_intercity_roads );
				if(  num_intercity_roads>=10  ) {
					dbg->fatal("settings_t::rdwr()", "Too many (%i) intercity roads!", num_intercity_roads);
				}
				for(  int i=0;  i<num_intercity_roads;  i++  ) {
					file->rdwr_str(intercity_roads[i].name, lengthof(intercity_roads[i].name) );
					file->rdwr_short(intercity_roads[i].intro );
					file->rdwr_short(intercity_roads[i].retire );
				}
			}
			file->rdwr_long(max_route_steps );
			file->rdwr_long(max_transfers );
			file->rdwr_long(max_hops );

			file->rdwr_long(beginner_price_factor );

			// name of stops
			file->rdwr_str(language_code_names, lengthof(language_code_names) );

			// restore AI state
			//char password[16]; // unused
			for(  int i=0;  i<15;  i++  ) {
				file->rdwr_bool( automaten[i]);
				file->rdwr_byte( spieler_type[i]);
				if(  file->get_version()<=102002 || file->get_experimental_version() == 7) {
					char dummy[17];
					dummy[0] = 0;
					file->rdwr_str(dummy, lengthof(dummy));
				}
			}

			// cost section ...
			file->rdwr_bool( freeplay);
			if(  file->get_version()>102002 && file->get_experimental_version() != 7 ) {
				file->rdwr_longlong( starting_money);
				// these must be saved, since new player will get different amounts eventually
				for(  int i=0;  i<10;  i++  ) {
					file->rdwr_short(startingmoneyperyear[i].year );
					file->rdwr_longlong(startingmoneyperyear[i].money );
					file->rdwr_bool(startingmoneyperyear[i].interpol );
				}
			}
			else {
				// compatibility code
				sint64 save_starting_money = starting_money;
				if(  file->is_saving()  ) {
					if(save_starting_money==0) {
						save_starting_money = get_starting_money(starting_year );
					}
					if(save_starting_money==0) {
						save_starting_money = umgebung_t::default_einstellungen.get_starting_money(starting_year );
					}
					if(save_starting_money==0) {
						save_starting_money = 20000000;
					}
				}
				file->rdwr_longlong(save_starting_money );
				if(file->is_loading()) {
					if(save_starting_money==0) {
						save_starting_money = umgebung_t::default_einstellungen.get_starting_money(starting_year );
					}
					if(save_starting_money==0) {
						save_starting_money = 20000000;
					}
					starting_money = save_starting_money;
				}
			}
			file->rdwr_long(maint_building );

			file->rdwr_longlong(cst_multiply_dock );
			file->rdwr_longlong(cst_multiply_station );
			file->rdwr_longlong(cst_multiply_roadstop );
			file->rdwr_longlong(cst_multiply_airterminal );
			file->rdwr_longlong(cst_multiply_post );
			file->rdwr_longlong(cst_multiply_headquarter );
			file->rdwr_longlong(cst_depot_rail );
			file->rdwr_longlong(cst_depot_road );
			file->rdwr_longlong(cst_depot_ship );
			file->rdwr_longlong(cst_depot_air );
			if(  file->get_version()<=102001  ) {
				sint64 dummy64 = 100000;
				file->rdwr_longlong(dummy64 );
				file->rdwr_longlong(dummy64 );
				file->rdwr_longlong(dummy64 );
			}
			// alter landscape
			file->rdwr_longlong(cst_buy_land );
			file->rdwr_longlong(cst_alter_land );
			file->rdwr_longlong(cst_set_slope );
			file->rdwr_longlong(cst_found_city );
			file->rdwr_longlong(cst_multiply_found_industry );
			file->rdwr_longlong(cst_remove_tree );
			file->rdwr_longlong(cst_multiply_remove_haus );
			file->rdwr_longlong(cst_multiply_remove_field );
			// cost for transformers
			file->rdwr_longlong(cst_transformer );
			file->rdwr_longlong(cst_maintain_transformer );
			// wayfinder
			file->rdwr_long(way_count_straight );
			file->rdwr_long(way_count_curve );
			file->rdwr_long(way_count_double_curve );
			file->rdwr_long(way_count_90_curve );
			file->rdwr_long(way_count_slope );
			file->rdwr_long(way_count_tunnel );
			file->rdwr_long(way_max_bridge_len );
			file->rdwr_long(way_count_leaving_road );
		}
		else {
			// name of stops
			set_name_language_iso( umgebung_t::language_iso );

			// default AIs active
			for(  int i=0;  i<MAX_PLAYER_COUNT;  i++  ) {
				if(  i<2  ) {
					spieler_type[i] = spieler_t::HUMAN;
				}
				else if(  i==3  ) {
					spieler_type[i] = spieler_t::AI_PASSENGER;
				}
				else if(  i<8  ) {
					spieler_type[i] = spieler_t::AI_GOODS;
				}
				else {
					spieler_type[i] = spieler_t::EMPTY;
				}
				automaten[i] = false;
			}
		}

		if(file->get_version()>101000) {
			file->rdwr_bool( seperate_halt_capacities);
			if(file->get_experimental_version() < 2)
			{
				// Was pay for total distance.
				// Now depracated.
				uint8 dummy;
				file->rdwr_byte( dummy);
			}

			file->rdwr_short(starting_month );

			file->rdwr_short( river_number );
			file->rdwr_short( min_river_length );
			file->rdwr_short( max_river_length );
		}

		if(file->get_version()>102000) {
			file->rdwr_bool(avoid_overcrowding);
		}

		if(file->get_version()>102001) 
		{
			bool dummy;
			file->rdwr_bool(dummy);
			file->rdwr_bool(with_private_paks);
		}

		if(file->get_version()>=102003) {
			// network stuff
			random_counter = get_random_seed( );
			file->rdwr_long( random_counter );
			if(  !umgebung_t::networkmode  ||  umgebung_t::server  ) {
				frames_per_second = clamp(umgebung_t::fps,5,100 );	// update it on the server to the current setting
				frames_per_step = umgebung_t::network_frames_per_step;
			}
			file->rdwr_long( frames_per_second );
			file->rdwr_long( frames_per_step );
			if(  !umgebung_t::networkmode  ||  umgebung_t::server  ) {
				frames_per_second = umgebung_t::fps;	// update it on the server to the current setting
				frames_per_step = umgebung_t::network_frames_per_step;
			}
			file->rdwr_bool( allow_buying_obsolete_vehicles);
			if(file->get_experimental_version() >= 8 || file->get_experimental_version() == 0)
			{
				file->rdwr_long( factory_worker_minimum_towns);
				file->rdwr_long( factory_worker_maximum_towns);
			}
			if(file->get_experimental_version() >= 9 || file->get_experimental_version() == 0)
			{
				// forest stuff
				file->rdwr_byte( forest_base_size);
				file->rdwr_byte( forest_map_size_divisor);
				file->rdwr_byte( forest_count_divisor);
				file->rdwr_short( forest_inverse_spare_tree_density);
				file->rdwr_byte( max_no_of_trees_on_square);
				file->rdwr_short( tree_climates);
				file->rdwr_short( no_tree_climates);
				file->rdwr_bool( no_trees);
				if(file->get_experimental_version() < 9)
				{
					sint32 dummy = 0;
					file->rdwr_long(dummy); // Was "minimum city distance"
				}
				file->rdwr_long( industry_increase );
			}
			if(file->get_experimental_version() >= 9)
			{
				file->rdwr_long( city_isolation_factor );
			}
		}

		if(file->get_experimental_version() >= 1)
		{
			file->rdwr_short(min_bonus_max_distance);
			file->rdwr_short(max_bonus_min_distance);
			if(file->get_experimental_version() == 1)
			{
				uint16 dummy;
				file->rdwr_short(dummy);
			}
			else
			{
				file->rdwr_short(median_bonus_distance);
				file->rdwr_short(max_bonus_multiplier_percent);
				uint16 distance_per_tile_integer = meters_per_tile / 10;
				file->rdwr_short(distance_per_tile_integer);
				if(file->get_experimental_version() < 5 && file->get_experimental_version() >= 1)
				{
					// In earlier versions, the default was set to a higher level. This
					// is a problem when the new journey time tolerance features is used.
					if(file->is_loading())
					{
						distance_per_tile_integer = (distance_per_tile_integer * 8) / 10;
					}
					else
					{
						distance_per_tile_integer = (distance_per_tile_integer * 10) / 8;
					}
				}		
				meters_per_tile = distance_per_tile_integer * 10;
				
				file->rdwr_byte(tolerable_comfort_short);
				file->rdwr_byte(tolerable_comfort_median_short);
				file->rdwr_byte(tolerable_comfort_median_median);
				file->rdwr_byte(tolerable_comfort_median_long);
				file->rdwr_byte(tolerable_comfort_long);
				file->rdwr_short(tolerable_comfort_short_minutes);
				file->rdwr_short(tolerable_comfort_median_short_minutes);
				file->rdwr_short(tolerable_comfort_median_median_minutes);
				file->rdwr_short(tolerable_comfort_median_long_minutes);
				file->rdwr_short(tolerable_comfort_long_minutes);
				file->rdwr_byte(max_luxury_bonus_differential);
				file->rdwr_byte(max_discomfort_penalty_differential);
				file->rdwr_short(max_discomfort_penalty_percent);
				file->rdwr_short(max_luxury_bonus_percent);

				file->rdwr_short(catering_min_minutes);
				file->rdwr_short(catering_level1_minutes);
				file->rdwr_short(catering_level1_max_revenue);
				file->rdwr_short(catering_level2_minutes);
				file->rdwr_short(catering_level2_max_revenue);
				file->rdwr_short(catering_level3_minutes);
				file->rdwr_short(catering_level3_max_revenue);
				file->rdwr_short(catering_level4_minutes);
				file->rdwr_short(catering_level4_max_revenue);
				file->rdwr_short(catering_level5_minutes);
				file->rdwr_short(catering_level5_max_revenue);

				file->rdwr_short(tpo_min_minutes);
				file->rdwr_short(tpo_revenue);
			}

			if(file->get_experimental_version() < 6)
			{
				float32e8_t km_per_tile(meters_per_tile, 1000);
				min_bonus_max_distance /= km_per_tile;
				max_bonus_min_distance /= km_per_tile;
				// Scale the costs to match the scale factor.
				cst_multiply_dock *= km_per_tile;
				cst_multiply_station *= km_per_tile;
				cst_multiply_roadstop *= km_per_tile;
				cst_multiply_airterminal *= km_per_tile;
				cst_multiply_post *= km_per_tile;
				maint_building *= km_per_tile;
				cst_buy_land *= km_per_tile;
				cst_remove_tree *= km_per_tile;
			}

			file->rdwr_short(obsolete_running_cost_increase_percent);
			file->rdwr_short(obsolete_running_cost_increase_phase_years);

			if(file->get_experimental_version() >= 9)
			{
				file->rdwr_long(local_passengers_min_distance);
				file->rdwr_long(local_passengers_max_distance);
				file->rdwr_long(midrange_passengers_min_distance);
				file->rdwr_long(midrange_passengers_max_distance);
				file->rdwr_long(longdistance_passengers_min_distance);
				file->rdwr_long(longdistance_passengers_max_distance);
			}
			else
			{
				uint16 old_local_passengers_min_distance = (uint16) local_passengers_min_distance;
				file->rdwr_short(old_local_passengers_min_distance);
				local_passengers_min_distance = (uint32)old_local_passengers_min_distance;

				uint16 old_local_passengers_max_distance = (uint16)local_passengers_max_distance;
				file->rdwr_short(old_local_passengers_max_distance);
				local_passengers_max_distance = (uint32)old_local_passengers_max_distance;

				uint16 old_midrange_passengers_min_distance = (uint16)midrange_passengers_min_distance;	
				file->rdwr_short(old_midrange_passengers_min_distance);
				midrange_passengers_min_distance = (uint32)old_midrange_passengers_min_distance;

				uint16 old_midrange_passengers_max_distance = (uint16)midrange_passengers_max_distance;
				file->rdwr_short(old_midrange_passengers_max_distance);
				midrange_passengers_max_distance = (uint32)old_midrange_passengers_max_distance;

				uint16 old_longdistance_passengers_min_distance = (uint16)longdistance_passengers_min_distance;
				file->rdwr_short(old_longdistance_passengers_min_distance);
				longdistance_passengers_min_distance = (uint32)old_longdistance_passengers_min_distance;

				uint16 old_longdistance_passengers_max_distance = (uint16)longdistance_passengers_max_distance;
				file->rdwr_short(old_longdistance_passengers_max_distance);
				longdistance_passengers_max_distance = (uint32)old_longdistance_passengers_max_distance;
				if(longdistance_passengers_max_distance == 0)
				{
					// Often, with high numbers specified, this value would be set incorrectly to zero. Correct this.
					longdistance_passengers_max_distance = 65535;
				}
			}

			file->rdwr_byte(passenger_routing_packet_size);
			file->rdwr_byte(max_alternative_destinations);
			file->rdwr_byte(passenger_routing_local_chance);
			file->rdwr_byte(passenger_routing_midrange_chance);
			file->rdwr_byte(base_car_preference_percent);
			file->rdwr_byte(always_prefer_car_percent);
			file->rdwr_byte(congestion_density_factor);

			if(file->get_experimental_version() < 4)
			{
				if(passenger_routing_packet_size < 1)
				{
					passenger_routing_packet_size = 7;
				}
				if(passenger_routing_local_chance < 1 || passenger_routing_local_chance > 99)
				{
					passenger_routing_local_chance = 33;
				}
				if(passenger_routing_midrange_chance < 1 || passenger_routing_midrange_chance > 99)
				{
					passenger_routing_midrange_chance = 33;
				}
			}

			waytype_t wt = road_wt;
			for(int n = 1; n < 7; n ++)
			{
				switch(n)
				{
				case 1:
					wt = road_wt;
					break;
				case 2:
					wt = track_wt;
					break;
				case 3:
					wt = tram_wt;
					break;
				case 4:
					wt = monorail_wt;
					break;
				case 5:
					wt = maglev_wt;
					break;
				case 6:
					wt = narrowgauge_wt;
					break;
				default:
					dbg->fatal("einstellungen_t::rdwr", "Invalid waytype");
				}

				file->rdwr_long(max_corner_limit[waytype_t(wt)]);
				file->rdwr_long(min_corner_limit[waytype_t(wt)]);
				if(file->get_experimental_version() < 10)
				{
					double tmp = (double)max_corner_adjustment_factor[waytype_t(wt)] / 100.0;
					file->rdwr_double(tmp);
					max_corner_adjustment_factor[waytype_t(wt)] = tmp * 100.0;
					tmp = (double)min_corner_adjustment_factor[waytype_t(wt)] / 100.0;
					file->rdwr_double(tmp);
					min_corner_adjustment_factor[waytype_t(wt)] = tmp * 100.0;
				}
				else
				{
					file->rdwr_short(max_corner_adjustment_factor[waytype_t(wt)]);
					file->rdwr_short(min_corner_adjustment_factor[waytype_t(wt)]);
				}
				file->rdwr_byte(min_direction_steps[waytype_t(wt)]);
				file->rdwr_byte(max_direction_steps[waytype_t(wt)]);
				file->rdwr_byte(curve_friction_factor[waytype_t(wt)]);
			}

			file->rdwr_short(factory_max_years_obsolete);

			file->rdwr_byte(interest_rate_percent);
			file->rdwr_bool(allow_bankruptsy);
			file->rdwr_bool(allow_purhcases_when_insolvent);

			file->rdwr_short(unit_reverse_time);
			file->rdwr_short(hauled_reverse_time);
			file->rdwr_short(turntable_reverse_time);

		}

		if(file->get_experimental_version() >= 2)
		{
			file->rdwr_short(global_power_factor_percent);
			if(file->get_experimental_version() <= 7)
			{
				uint16 old_passenger_max_wait;
				file->rdwr_short(old_passenger_max_wait);
				passenger_max_wait = (uint32)old_passenger_max_wait;
			}
			else
			{
				file->rdwr_long(passenger_max_wait);
			}
			file->rdwr_byte(max_rerouting_interval_months);
		}

		if(file->get_experimental_version() >= 3)
		{
			if(file->get_experimental_version() < 7)
			{
				// Was city weight factor. Now replaced by a more
				// sophisticated customisable city growth from Standard.
				uint16 dummy;
				file->rdwr_short(dummy);
			}
			file->rdwr_byte(enforce_weight_limits);
			file->rdwr_short(speed_bonus_multiplier_percent);
		}
		else
		{
			// For older saved games, enforcing weight limits strictly is likely to lead
			// to problems, so, whilst strictly enforced weight limits should be allowed
			// for new games and games saved with this feature enabled, it should not be
			// allowed for older saved games.
			enforce_weight_limits = enforce_weight_limits == 0 ? 0 : 1;
		}
		
		if(file->get_experimental_version() >= 4)
		{
			file->rdwr_byte(default_path_option);
		}

		if(file->get_experimental_version() >= 5)
		{
			file->rdwr_short(min_local_tolerance);
			file->rdwr_short(max_local_tolerance);
			file->rdwr_short(min_midrange_tolerance);
			file->rdwr_short(max_midrange_tolerance);
			file->rdwr_short(min_longdistance_tolerance);
			file->rdwr_short(max_longdistance_tolerance);
		}

		if(  file->get_version()>=110000  ) {
			if(  !umgebung_t::networkmode  ||  umgebung_t::server  ) {
				server_frames_ahead = umgebung_t::server_frames_ahead;
			}
			file->rdwr_long( server_frames_ahead );
			if(  !umgebung_t::networkmode  ||  umgebung_t::server  ) {
				server_frames_ahead = umgebung_t::server_frames_ahead;
			}
			file->rdwr_short( used_vehicle_reduction );
		}
		
		if(file->get_experimental_version() >= 8)
		{
			file->rdwr_short(max_walking_distance);
			file->rdwr_bool(quick_city_growth);
			file->rdwr_bool(assume_everywhere_connected_by_road);
			for(uint8 i = 0; i < 17; i ++)
			{
				file->rdwr_short(default_increase_maintenance_after_years[i]);
			}
			file->rdwr_long(capital_threshold_size);
			file->rdwr_long(city_threshold_size);
		}
		
		if(  file->get_version()>=110001  ) {
			file->rdwr_bool( default_player_color_random );
			for(  int i=0;  i<MAX_PLAYER_COUNT;  i++  ) {
				file->rdwr_byte( default_player_color[i][0] );
				file->rdwr_byte( default_player_color[i][1] );
			}
		}
		else if(  file->is_loading()  ) {
			default_player_color_random = false;
			for(  int i=0;  i<MAX_PLAYER_COUNT;  i++  ) {
				// default colors for player ...
				default_player_color[i][0] = 255;
				default_player_color[i][1] = 255;
			}
		}

		if(  file->get_version()>=110005  ) {
			file->rdwr_short(factory_arrival_periods);
			file->rdwr_bool(factory_enforce_demand);
		}

		if(  file->get_version()>=110007  ) 
		{
			if(file->get_experimental_version() == 0 )
			{
				uint16 city_short_range_percentage = passenger_routing_local_chance;
				uint16 city_medium_range_percentage = passenger_routing_midrange_chance;
				uint32 city_short_range_radius = local_passengers_max_distance;
				uint32 city_medium_range_radius = midrange_passengers_max_distance;

				file->rdwr_short(city_short_range_percentage);
				file->rdwr_short(city_medium_range_percentage);
				file->rdwr_long(city_short_range_radius);
				file->rdwr_long(city_medium_range_radius);

				passenger_routing_local_chance = city_short_range_percentage;
				passenger_routing_midrange_chance = city_medium_range_percentage;
				local_passengers_max_distance = city_short_range_radius;
				midrange_passengers_max_distance = city_medium_range_radius;
			}
		}

		if (file->get_experimental_version() >= 9 && file->get_version() >= 110006) 
		{
			file->rdwr_byte(spacing_shift_mode);
			file->rdwr_short(spacing_shift_divisor);

			uint16 livery_schemes_count = 0;
			if(file->is_loading())
			{
				ITERATE(livery_schemes, i)
				{
					delete livery_schemes[i];
				}
				livery_schemes.clear();
			}
			if(file->is_saving())
			{
				livery_schemes_count = livery_schemes.get_count();
			}
			
			file->rdwr_short(livery_schemes_count);

			if(strcmp(file->get_pak_extension(), "settings only"))
			{
				for(int i = 0; i < livery_schemes_count; i ++)
				{
					if(file->is_saving())
					{
						livery_schemes[i]->rdwr(file);
					}
					else
					{
						livery_scheme_t* scheme = new livery_scheme_t("default", DEFAULT_RETIRE_DATE);
						scheme->rdwr(file);
						livery_schemes.append(scheme);
					}
				}
			}
		}
	}
}




// read the settings from this file
void settings_t::parse_simuconf(tabfile_t& simuconf, sint16& disp_width, sint16& disp_height, sint16 &fullscreen, std::string& objfilename)
{
	tabfileobj_t contents;

	simuconf.read(contents );

	// This needs to be first as other settings are based on this.
	// @author: jamespetts
	uint16 distance_per_tile_integer = meters_per_tile / 10;
	meters_per_tile = contents.get_int("distance_per_tile", distance_per_tile_integer) * 10;
	float32e8_t distance_per_tile(meters_per_tile, 1000);

	umgebung_t::water_animation = contents.get_int("water_animation_ms", umgebung_t::water_animation);
	umgebung_t::ground_object_probability = contents.get_int("random_grounds_probability", umgebung_t::ground_object_probability);
	umgebung_t::moving_object_probability = contents.get_int("random_wildlife_probability", umgebung_t::moving_object_probability);
	umgebung_t::drive_on_left = contents.get_int("drive_left", umgebung_t::drive_on_left );

	umgebung_t::verkehrsteilnehmer_info = contents.get_int("pedes_and_car_info", umgebung_t::verkehrsteilnehmer_info) != 0;
	umgebung_t::tree_info = contents.get_int("tree_info", umgebung_t::tree_info) != 0;
	umgebung_t::ground_info = contents.get_int("ground_info", umgebung_t::ground_info) != 0;
	umgebung_t::townhall_info = contents.get_int("townhall_info", umgebung_t::townhall_info) != 0;
	umgebung_t::single_info = contents.get_int("only_single_info", umgebung_t::single_info );

	umgebung_t::window_snap_distance = contents.get_int("window_snap_distance", umgebung_t::window_snap_distance );
	umgebung_t::window_buttons_right = contents.get_int("window_buttons_right", umgebung_t::window_buttons_right );
	umgebung_t::left_to_right_graphs = contents.get_int("left_to_right_graphs", umgebung_t::left_to_right_graphs );
	umgebung_t::window_frame_active = contents.get_int("window_frame_active", umgebung_t::window_frame_active );
	umgebung_t::front_window_bar_color = contents.get_int("front_window_bar_color", umgebung_t::front_window_bar_color );
	umgebung_t::front_window_text_color = contents.get_int("front_window_text_color", umgebung_t::front_window_text_color );
	umgebung_t::bottom_window_bar_color = contents.get_int("bottom_window_bar_color", umgebung_t::bottom_window_bar_color );
	umgebung_t::bottom_window_text_color = contents.get_int("bottom_window_text_color", umgebung_t::bottom_window_text_color );

	umgebung_t::show_tooltips = contents.get_int("show_tooltips", umgebung_t::show_tooltips );
	umgebung_t::tooltip_color = contents.get_int("tooltip_background_color", umgebung_t::tooltip_color );
	umgebung_t::tooltip_textcolor = contents.get_int("tooltip_text_color", umgebung_t::tooltip_textcolor );
	umgebung_t::tooltip_delay = contents.get_int("tooltip_delay", umgebung_t::tooltip_delay );
	umgebung_t::tooltip_duration = contents.get_int("tooltip_duration", umgebung_t::tooltip_duration );
	umgebung_t::toolbar_max_width = contents.get_int("toolbar_max_width", umgebung_t::toolbar_max_width );
	umgebung_t::toolbar_max_height = contents.get_int("toolbar_max_height", umgebung_t::toolbar_max_height );
	umgebung_t::cursor_overlay_color = contents.get_int("cursor_overlay_color", umgebung_t::cursor_overlay_color );
	umgebung_t::add_player_name_to_message = contents.get_int("add_player_name_to_message", umgebung_t::add_player_name_to_message );

	// display stuff
	umgebung_t::show_names = contents.get_int("show_names", umgebung_t::show_names );
	umgebung_t::show_month = contents.get_int("show_month", umgebung_t::show_month );
	umgebung_t::max_acceleration = contents.get_int("fast_forward", umgebung_t::max_acceleration );

	// network stuff
	umgebung_t::server_frames_ahead = contents.get_int("server_frames_ahead", umgebung_t::server_frames_ahead );
	umgebung_t::additional_client_frames_behind = contents.get_int("additional_client_frames_behind", umgebung_t::additional_client_frames_behind);
	umgebung_t::network_frames_per_step = contents.get_int("server_frames_per_step", umgebung_t::network_frames_per_step );
	umgebung_t::server_sync_steps_between_checks = contents.get_int("server_frames_between_checks", umgebung_t::server_sync_steps_between_checks );

	umgebung_t::announce_server = contents.get_int("announce_server", umgebung_t::announce_server );
	umgebung_t::announce_server = contents.get_int("server_announce", umgebung_t::announce_server );
	umgebung_t::announce_server_intervall = contents.get_int("server_announce_intervall", umgebung_t::announce_server );
	if(  *contents.get("server_name")  ) {
		umgebung_t::server_name = ltrim(contents.get("server_name"));
	}
	if(  *contents.get("server_comment")  ) {
		umgebung_t::server_comment = ltrim(contents.get("server_comment"));
	}
	if(  *contents.get("server_admin_pw")  ) {
		umgebung_t::server_admin_pw = ltrim(contents.get("server_admin_pw"));
	}

	// up to ten rivers are possible
	for(  int i = 0;  i<10;  i++  ) {
		char name[32];
		sprintf( name, "river_type[%i]", i );
		const char *test = ltrim(contents.get(name) );
		if(*test) {
			const int add_river = i<umgebung_t::river_types ? i : umgebung_t::river_types;
			free( (void *)(umgebung_t::river_type[add_river]) );
			umgebung_t::river_type[add_river] = NULL;
			umgebung_t::river_type[add_river] = strdup( test );
			if(  add_river==umgebung_t::river_types  ) {
				umgebung_t::river_types++;
			}
		}
	}

	// old syntax for single city road
	const char *str = ltrim(contents.get("city_road_type") );
	if(  str[0]  ) {
		num_city_roads = 1;
		tstrncpy(city_roads[0].name, str, lengthof(city_roads[0].name) );
		rtrim( city_roads[0].name );
		// default her: always available
		city_roads[0].intro = 1;
		city_roads[0].retire = NEVER;
	}

	// new: up to ten city_roads are possible
	if(  *contents.get("city_road[0]")  ) {
		// renew them always when a table is encountered ...
		num_city_roads = 0;
		for(  int i = 0;  i<10;  i++  ) {
			char name[256];
			sprintf( name, "city_road[%i]", i );
			// format is "city_road[%i]=name_of_road,using from (year), using to (year)"
			const char *test = ltrim(contents.get(name) );
			if(*test) {
				const char *p = test;
				while(*p  &&  *p!=','  ) {
					p++;
				}
				tstrncpy( city_roads[num_city_roads].name, test, (unsigned)(p-test)+1 );
				// default her: intro/retire=0 -> set later to intro/retire of way-besch
				city_roads[num_city_roads].intro = 0;
				city_roads[num_city_roads].retire = 0;
				if(  *p==','  ) {
					++p;
					city_roads[num_city_roads].intro = atoi(p)*12;
					while(*p  &&  *p!=','  ) {
						p++;
					}
				}
				if(  *p==','  ) {
					city_roads[num_city_roads].retire = atoi(p+1)*12;
				}
				num_city_roads ++;
			}
		}
	}
	if (num_city_roads == 0) {
		// take fallback value: "city_road"
		tstrncpy(city_roads[0].name, "city_road", lengthof(city_roads[0].name) );
		// default her: always available
		city_roads[0].intro = 1;
		city_roads[0].retire = NEVER;
		num_city_roads = 1;
	}

	// intercity roads
	// old syntax for single intercity road
	str = ltrim(contents.get("intercity_road_type") );
	if(  str[0]  ) {
		num_intercity_roads = 1;
		tstrncpy(intercity_roads[0].name, str, lengthof(intercity_roads[0].name) );
		rtrim( intercity_roads[0].name );
		// default her: always available
		intercity_roads[0].intro = 1;
		intercity_roads[0].retire = NEVER;
	}

	// new: up to ten intercity_roads are possible
	if(  *contents.get("intercity_road[0]")  ) {
		// renew them always when a table is encountered ...
		num_intercity_roads = 0;
		for(  int i = 0;  i<10;  i++  ) {
			char name[256];
			sprintf( name, "intercity_road[%i]", i );
			// format is "intercity_road[%i]=name_of_road,using from (year), using to (year)"
			const char *test = ltrim(contents.get(name) );
			if(*test) {
				const char *p = test;
				while(*p  &&  *p!=','  ) {
					p++;
				}
				tstrncpy( intercity_roads[num_intercity_roads].name, test, (unsigned)(p-test)+1 );
				// default her: intro/retire=0 -> set later to intro/retire of way-besch
				intercity_roads[num_intercity_roads].intro = 0;
				intercity_roads[num_intercity_roads].retire = 0;
				if(  *p==','  ) {
					++p;
					intercity_roads[num_intercity_roads].intro = atoi(p)*12;
					while(*p  &&  *p!=','  ) {
						p++;
					}
				}
				if(  *p==','  ) {
					intercity_roads[num_intercity_roads].retire = atoi(p+1)*12;
				}
				num_intercity_roads ++;
			}
		}
	}
	if (num_intercity_roads == 0) {
		// take fallback value: "asphalt_road"
		tstrncpy(intercity_roads[0].name, "asphalt_road", lengthof(intercity_roads[0].name) );
		// default her: always available
		intercity_roads[0].intro = 1;
		intercity_roads[0].retire = NEVER;
		num_intercity_roads = 1;
	}


	umgebung_t::autosave = (contents.get_int("autosave", umgebung_t::autosave) );
	umgebung_t::fps = contents.get_int("frames_per_second",umgebung_t::fps );

	// routing stuff
	uint16 city_short_range_percentage = passenger_routing_local_chance;
	uint16 city_medium_range_percentage = passenger_routing_midrange_chance;
	uint32 city_short_range_radius = local_passengers_max_distance;
	uint32 city_medium_range_radius = midrange_passengers_max_distance;

	max_route_steps = contents.get_int("max_route_steps", max_route_steps );
	max_hops = contents.get_int("max_hops", max_hops );
	max_transfers = contents.get_int("max_transfers", max_transfers );
	industry_increase = contents.get_int("industry_increase_every", industry_increase );
	city_isolation_factor = contents.get_int("city_isolation_factor", city_isolation_factor );
	passenger_factor = contents.get_int("passenger_factor", passenger_factor ); /* this can manipulate the passenger generation */
	factory_worker_percentage = contents.get_int("factory_worker_percentage", factory_worker_percentage );
	factory_worker_radius = contents.get_int("factory_worker_radius", factory_worker_radius );
	factory_worker_minimum_towns = contents.get_int("factory_worker_minimum_towns", factory_worker_minimum_towns );
	factory_worker_maximum_towns = contents.get_int("factory_worker_maximum_towns", factory_worker_maximum_towns );
	factory_arrival_periods = clamp( contents.get_int("factory_arrival_periods", factory_arrival_periods), 1, 16 );
	factory_enforce_demand = contents.get_int("factory_enforce_demand", factory_enforce_demand) != 0;
	tourist_percentage = contents.get_int("tourist_percentage", tourist_percentage );
	city_short_range_percentage = contents.get_int("city_short_range_percentage", city_short_range_percentage);
	city_medium_range_percentage = contents.get_int("city_medium_range_percentage", city_medium_range_percentage);
	city_short_range_radius = contents.get_int("city_short_range_radius", city_short_range_radius);
	city_medium_range_radius = contents.get_int("city_medium_range_radius", city_medium_range_radius);
	seperate_halt_capacities = contents.get_int("seperate_halt_capacities", seperate_halt_capacities ) != 0;
	avoid_overcrowding = contents.get_int("avoid_overcrowding", avoid_overcrowding )!=0;
	passenger_max_wait = contents.get_int("passenger_max_wait", passenger_max_wait); 
	max_rerouting_interval_months = contents.get_int("max_rerouting_interval_months", max_rerouting_interval_months);

	// city stuff
	passenger_multiplier = contents.get_int("passenger_multiplier", passenger_multiplier );
	mail_multiplier = contents.get_int("mail_multiplier", mail_multiplier );
	goods_multiplier = contents.get_int("goods_multiplier", goods_multiplier );
	electricity_multiplier = contents.get_int("electricity_multiplier", electricity_multiplier );

	growthfactor_small = contents.get_int("growthfactor_villages", growthfactor_small );
	growthfactor_medium = contents.get_int("growthfactor_cities", growthfactor_medium );
	growthfactor_large = contents.get_int("growthfactor_capitals", growthfactor_large );

	fussgaenger = contents.get_int("random_pedestrians", fussgaenger ) != 0;
	show_pax = contents.get_int("stop_pedestrians", show_pax ) != 0;
	verkehr_level = contents.get_int("citycar_level", verkehr_level );	// ten normal years
	stadtauto_duration = contents.get_int("default_citycar_life", stadtauto_duration );	// ten normal years
	allow_buying_obsolete_vehicles = contents.get_int("allow_buying_obsolete_vehicles", allow_buying_obsolete_vehicles );
	used_vehicle_reduction  = clamp( contents.get_int("used_vehicle_reduction", used_vehicle_reduction ), 0, 1000 );

	// starting money
	starting_money = contents.get_int64("starting_money", starting_money );
	/* up to ten blocks year, money, interpolation={0,1} are possible:
	 * starting_money[i]=y,m,int
	 * y .. year
	 * m .. money (in 1/100 Cr)
	 * int .. interpolation: 0 - no interpolation, !=0 linear interpolated
	 * (m) is the starting money for player start after (y), if (i)!=0, the starting money
	 * is linearly interpolated between (y) and the next greater year given in another entry.
	 * starting money for given year is:
	 */
	int j=0;
	for(  int i = 0;  i<10;  i++  ) {
		char name[32];
		sprintf( name, "starting_money[%i]", i );
		sint64 *test = contents.get_sint64s(name );
		if ((test[0]>1) && (test[0]<=3)) {
			// insert sorted by years
			int k=0;
			for (k=0; k<i; k++) {
				if (startingmoneyperyear[k].year > test[1]) {
					for (int l=j; l>=k; l--)
						memcpy( &startingmoneyperyear[l+1], &startingmoneyperyear[l], sizeof(yearmoney) );
					break;
				}
			}
			startingmoneyperyear[k].year = (sint16)test[1];
			startingmoneyperyear[k].money = test[2];
			if (test[0]==3) {
				startingmoneyperyear[k].interpol = test[3]!=0;
			}
			else {
				startingmoneyperyear[k].interpol = false;
			}
//			printf("smpy[%d] year=%d money=%lld\n",k,startingmoneyperyear[k].year,startingmoneyperyear[k].money);
			j++;
		}
		else {
			// invalid entry
		}
		delete [] test;
	}
	// at least one found => use this now!
	if(  j>0  &&  startingmoneyperyear[0].money>0  ) {
		starting_money = 0;
		// fill remaining entries
		for(  int i=j+1; i<10; i++  ) {
			startingmoneyperyear[i].year = 0;
			startingmoneyperyear[i].money = 0;
			startingmoneyperyear[i].interpol = 0;
		}
	}

	// player colors
	default_player_color_random = contents.get_int("random_player_colors", default_player_color_random ) != 0;
	for(  int i = 0;  i<MAX_PLAYER_COUNT;  i++  ) {
		char name[32];
		sprintf( name, "player_color[%i]", i );
		const char *command = contents.get(name);
		int c1, c2;
		if(  sscanf( command, "%i,%i", &c1, &c2 )==2  ) {
			default_player_color[i][0] = c1;
			default_player_color[i][1] = c2;
		}
	}

	maint_building = contents.get_int("maintenance_building", maint_building);
	const sint32 inverse_distance_per_tile = 1 / distance_per_tile;
	maint_building /= inverse_distance_per_tile;

	numbered_stations = contents.get_int("numbered_stations", numbered_stations );
	station_coverage_size = contents.get_int("station_coverage", station_coverage_size );

	// time stuff
	bits_per_month = contents.get_int("bits_per_month", bits_per_month );
	use_timeline = contents.get_int("use_timeline", use_timeline );
	starting_year = contents.get_int("starting_year", starting_year );
	starting_month = contents.get_int("starting_month", starting_month+1)-1;

	river_number = contents.get_int("river_number", river_number );
	min_river_length = contents.get_int("river_min_length", min_river_length );
	max_river_length = contents.get_int("river_max_length", max_river_length );

	// forest stuff (now part of simuconf.tab)
	forest_base_size = contents.get_int("forest_base_size", forest_base_size );
	forest_map_size_divisor = contents.get_int("forest_map_size_divisor", forest_map_size_divisor );
	forest_count_divisor = contents.get_int("forest_count_divisor", forest_count_divisor );
	forest_inverse_spare_tree_density = contents.get_int("forest_inverse_spare_tree_density", forest_inverse_spare_tree_density );
	max_no_of_trees_on_square = contents.get_int("max_no_of_trees_on_square", max_no_of_trees_on_square );
	tree_climates = contents.get_int("tree_climates", tree_climates );
	no_tree_climates = contents.get_int("no_tree_climates", no_tree_climates );
	no_trees	= contents.get_int("no_trees", no_trees );

	pak_diagonal_multiplier = contents.get_int("diagonal_multiplier", pak_diagonal_multiplier );

	factory_spacing = contents.get_int("factory_spacing", factory_spacing );
	crossconnect_factories = contents.get_int("crossconnect_factories", crossconnect_factories ) != 0;
	crossconnect_factor = contents.get_int("crossconnect_factories_percentage", crossconnect_factor );
	electric_promille = contents.get_int("electric_promille", electric_promille );

	just_in_time = contents.get_int("just_in_time", just_in_time) != 0;
	beginner_price_factor = contents.get_int("beginner_price_factor", beginner_price_factor ); /* this manipulates the good prices in beginner mode */
	beginner_mode = contents.get_int("first_beginner", beginner_mode ); /* start in beginner mode */

	/* now the cost section */
	cst_multiply_dock = (contents.get_int64("cost_multiply_dock", cst_multiply_dock/(-100) ) * -100) * distance_per_tile;
	cst_multiply_station = (contents.get_int64("cost_multiply_station", cst_multiply_station/(-100) ) * -100) * distance_per_tile;
	cst_multiply_roadstop = (contents.get_int64("cost_multiply_roadstop", cst_multiply_roadstop/(-100) ) * -100) * distance_per_tile;
	cst_multiply_airterminal = (contents.get_int64("cost_multiply_airterminal", cst_multiply_airterminal/(-100) ) * -100) * distance_per_tile;
	cst_multiply_post = (contents.get_int64("cost_multiply_post", cst_multiply_post/(-100) ) * -100) * distance_per_tile;
	cst_multiply_headquarter = contents.get_int64("cost_multiply_headquarter", cst_multiply_headquarter/(-100) ) * -100;
	cst_depot_air = contents.get_int64("cost_depot_air", cst_depot_air/(-100) ) * -100;
	cst_depot_rail = contents.get_int64("cost_depot_rail", cst_depot_rail/(-100) ) * -100;
	cst_depot_road = contents.get_int64("cost_depot_road", cst_depot_road/(-100) ) * -100;
	cst_depot_ship = contents.get_int64("cost_depot_ship", cst_depot_ship/(-100) ) * -100;

	// alter landscape
	cst_buy_land = (contents.get_int64("cost_buy_land", cst_buy_land/(-100) ) * -100) * distance_per_tile;
	cst_alter_land = contents.get_int64("cost_alter_land", cst_alter_land/(-100) ) * -100;
	cst_set_slope = contents.get_int64("cost_set_slope", cst_set_slope/(-100) ) * -100;
	cst_found_city = contents.get_int64("cost_found_city", cst_found_city/(-100) ) * -100;
	cst_multiply_found_industry = contents.get_int64("cost_multiply_found_industry", cst_multiply_found_industry/(-100) ) * -100;
	cst_remove_tree = (contents.get_int64("cost_remove_tree", cst_remove_tree/(-100) ) * -100) * distance_per_tile;
	cst_multiply_remove_haus = (contents.get_int64("cost_multiply_remove_haus", cst_multiply_remove_haus/(-100) ) * -100) * distance_per_tile;
	cst_multiply_remove_field = contents.get_int64("cost_multiply_remove_field", cst_multiply_remove_field/(-100) ) * -100;
	// powerlines
	cst_transformer = contents.get_int64("cost_transformer", cst_transformer/(-100) ) * -100;
	cst_maintain_transformer = contents.get_int64("cost_maintain_transformer", cst_maintain_transformer/(-100) ) * -100;

	/* now the way builder */
	way_count_straight = contents.get_int("way_straight", way_count_straight );
	way_count_curve = contents.get_int("way_curve", way_count_curve );
	way_count_double_curve = contents.get_int("way_double_curve", way_count_double_curve );
	way_count_90_curve = contents.get_int("way_90_curve", way_count_90_curve );
	way_count_slope = contents.get_int("way_slope", way_count_slope );
	way_count_tunnel = contents.get_int("way_tunnel", way_count_tunnel );
	way_max_bridge_len = contents.get_int("way_max_bridge_len", way_max_bridge_len );
	way_count_leaving_road = contents.get_int("way_leaving_road", way_count_leaving_road );

	// Revenue calibration settings
	// @author: jamespetts
	min_bonus_max_distance = contents.get_int("min_bonus_max_distance", min_bonus_max_distance) / distance_per_tile;
	max_bonus_min_distance = contents.get_int("max_bonus_min_distance", max_bonus_min_distance) / distance_per_tile;
	median_bonus_distance = contents.get_int("median_bonus_distance", median_bonus_distance) / distance_per_tile;
	max_bonus_multiplier_percent = contents.get_int("max_bonus_multiplier_percent", max_bonus_multiplier_percent);
	tolerable_comfort_short = contents.get_int("tolerable_comfort_short", tolerable_comfort_short);
	tolerable_comfort_long = contents.get_int("tolerable_comfort_long", tolerable_comfort_long);
	tolerable_comfort_short_minutes = contents.get_int("tolerable_comfort_short_minutes", tolerable_comfort_short_minutes);
	tolerable_comfort_long_minutes = contents.get_int("tolerable_comfort_long_minutes", tolerable_comfort_long_minutes);
	tolerable_comfort_median_short = contents.get_int("tolerable_comfort_median_short", tolerable_comfort_median_short);
	tolerable_comfort_median_median = contents.get_int("tolerable_comfort_median_median", tolerable_comfort_median_median);
	tolerable_comfort_median_long = contents.get_int("tolerable_comfort_median_long", tolerable_comfort_median_long);
	tolerable_comfort_median_short_minutes = contents.get_int("tolerable_comfort_median_short_minutes", tolerable_comfort_median_short_minutes);
	tolerable_comfort_median_median_minutes = contents.get_int("tolerable_comfort_median_median_minutes", tolerable_comfort_median_median_minutes);
	tolerable_comfort_median_long_minutes = contents.get_int("tolerable_comfort_median_long_minutes", tolerable_comfort_median_long_minutes);
	max_luxury_bonus_differential = contents.get_int("max_luxury_bonus_differential", max_luxury_bonus_differential);
	max_discomfort_penalty_differential = contents.get_int("max_discomfort_penalty_differential", max_discomfort_penalty_differential);
	max_luxury_bonus_percent = contents.get_int("max_luxury_bonus_percent", max_luxury_bonus_percent);
	max_discomfort_penalty_percent = contents.get_int("max_discomfort_penalty_percent", max_discomfort_penalty_percent);

	catering_min_minutes = contents.get_int("catering_min_minutes", catering_min_minutes);
	catering_level1_minutes = contents.get_int("catering_level1_minutes", catering_level1_minutes);
	catering_level1_max_revenue = contents.get_int("catering_level1_max_revenue", catering_level1_max_revenue);
	catering_level2_minutes = contents.get_int("catering_level2_minutes", catering_level2_minutes);
	catering_level2_max_revenue = contents.get_int("catering_level2_max_revenue", catering_level2_max_revenue);
	catering_level3_minutes = contents.get_int("catering_level3_minutes", catering_level3_minutes);
	catering_level3_max_revenue = contents.get_int("catering_level3_max_revenue", catering_level3_max_revenue);
	catering_level4_minutes = contents.get_int("catering_level4_minutes", catering_level4_minutes);
	catering_level4_max_revenue = contents.get_int("catering_level4_max_revenue", catering_level4_max_revenue);
	catering_level5_minutes = contents.get_int("catering_level5_minutes", catering_level5_minutes);
	catering_level5_max_revenue = contents.get_int("catering_level5_max_revenue", catering_level5_max_revenue);

	tpo_min_minutes = contents.get_int("tpo_min_minutes", tpo_min_minutes);
	tpo_revenue = contents.get_int("tpo_revenue", tpo_revenue);

	// Obsolete vehicles' running cost increase
	obsolete_running_cost_increase_percent = contents.get_int("obsolete_running_cost_increase_percent", obsolete_running_cost_increase_percent);
	obsolete_running_cost_increase_phase_years = contents.get_int("obsolete_running_cost_increase_phase_years", obsolete_running_cost_increase_phase_years);

	// Passenger destination ranges
	uint32 city_short_range_radius_km = city_short_range_radius * distance_per_tile;
	uint32 city_medium_range_radius_km = city_medium_range_radius * distance_per_tile;
	local_passengers_min_distance = contents.get_int("local_passengers_min_distance", local_passengers_min_distance) / distance_per_tile;
	local_passengers_max_distance = contents.get_int("local_passengers_max_distance", city_short_range_radius_km) / distance_per_tile;
	midrange_passengers_min_distance = contents.get_int("midrange_passengers_min_distance", midrange_passengers_min_distance) / distance_per_tile;
	midrange_passengers_max_distance = contents.get_int("midrange_passengers_max_distance", city_medium_range_radius_km) / distance_per_tile;
	longdistance_passengers_min_distance = contents.get_int("longdistance_passengers_min_distance", longdistance_passengers_min_distance) / distance_per_tile;
	longdistance_passengers_max_distance = contents.get_int("longdistance_passengers_max_distance", longdistance_passengers_max_distance) / distance_per_tile;
	
	// Passenger routing settings
	passenger_routing_packet_size = contents.get_int("passenger_routing_packet_size", passenger_routing_packet_size);
	if(passenger_routing_packet_size < 1)
	{
		passenger_routing_packet_size = 7;
	}
	max_alternative_destinations = contents.get_int("max_alternative_destinations", max_alternative_destinations);
	passenger_routing_local_chance  = contents.get_int("passenger_routing_local_chance", city_short_range_percentage);
	if(passenger_routing_local_chance < 1 || passenger_routing_local_chance > 99)
	{
		passenger_routing_local_chance = 33;
	}
	passenger_routing_midrange_chance = contents.get_int("passenger_routing_midrange_chance", city_medium_range_percentage);
	if(passenger_routing_midrange_chance < 1 || passenger_routing_midrange_chance > 99)
	{
		passenger_routing_midrange_chance = 33;
	}
	base_car_preference_percent = contents.get_int("base_car_preference_percent", base_car_preference_percent);
	always_prefer_car_percent = contents.get_int("always_prefer_car_percent", always_prefer_car_percent);
	congestion_density_factor = contents.get_int("congestion_density_factor", congestion_density_factor);

	// Cornering settings
	max_corner_limit[waytype_t(road_wt)] = contents.get_int("max_corner_limit_road", max_corner_limit[waytype_t(road_wt)]);
	min_corner_limit[waytype_t(road_wt)] = contents.get_int("min_corner_limit_road", min_corner_limit[waytype_t(road_wt)]);
	max_corner_adjustment_factor[waytype_t(road_wt)] = contents.get_int("max_corner_adjustment_factor_road", max_corner_adjustment_factor[waytype_t(road_wt)]);
	min_corner_adjustment_factor[waytype_t(road_wt)] = contents.get_int("min_corner_adjustment_factor_road", min_corner_adjustment_factor[waytype_t(road_wt)]);
	min_direction_steps[waytype_t(road_wt)] = contents.get_int("min_direction_steps_road", min_direction_steps[waytype_t(road_wt)]);
	max_direction_steps[waytype_t(road_wt)] = contents.get_int("max_direction_steps_road", max_direction_steps[waytype_t(road_wt)]);
	curve_friction_factor[waytype_t(road_wt)] = contents.get_int("curve_friction_factor_road", curve_friction_factor[waytype_t(road_wt)]);

	max_corner_limit[waytype_t(track_wt)] = contents.get_int("max_corner_limit_track", max_corner_limit[waytype_t(track_wt)]);
	min_corner_limit[waytype_t(track_wt)] = contents.get_int("min_corner_limit_track", min_corner_limit[waytype_t(track_wt)]);
	max_corner_adjustment_factor[waytype_t(track_wt)] = contents.get_int("max_corner_adjustment_factor_track", max_corner_adjustment_factor[waytype_t(track_wt)]);
	min_corner_adjustment_factor[waytype_t(track_wt)] = contents.get_int("min_corner_adjustment_factor_track", min_corner_adjustment_factor[waytype_t(track_wt)]);
	min_direction_steps[waytype_t(track_wt)] = contents.get_int("min_direction_steps_track", min_direction_steps[waytype_t(track_wt)]);
	max_direction_steps[waytype_t(track_wt)] = contents.get_int("max_direction_steps_track", max_direction_steps[waytype_t(track_wt)]);
	curve_friction_factor[waytype_t(track_wt)] = contents.get_int("curve_friction_factor_track", curve_friction_factor[waytype_t(track_wt)]);

	max_corner_limit[waytype_t(tram_wt)] = contents.get_int("max_corner_limit_tram", max_corner_limit[waytype_t(tram_wt)]);
	min_corner_limit[waytype_t(tram_wt)] = contents.get_int("min_corner_limit_tram", min_corner_limit[waytype_t(tram_wt)]);
	max_corner_adjustment_factor[waytype_t(tram_wt)] = contents.get_int("max_corner_adjustment_factor_tram", max_corner_adjustment_factor[waytype_t(tram_wt)]);
	min_corner_adjustment_factor[waytype_t(tram_wt)] = contents.get_int("min_corner_adjustment_factor_tram", min_corner_adjustment_factor[waytype_t(tram_wt)]);
	min_direction_steps[waytype_t(tram_wt)] = contents.get_int("min_direction_steps_tram", min_direction_steps[waytype_t(tram_wt)]);
	max_direction_steps[waytype_t(tram_wt)] = contents.get_int("max_direction_steps_tram", max_direction_steps[waytype_t(tram_wt)]);
	curve_friction_factor[waytype_t(tram_wt)] = contents.get_int("curve_friction_factor_tram", curve_friction_factor[waytype_t(tram_wt)]);

	max_corner_limit[waytype_t(monorail_wt)] = contents.get_int("max_corner_limit_monorail", max_corner_limit[waytype_t(monorail_wt)]);
	min_corner_limit[waytype_t(monorail_wt)] = contents.get_int("min_corner_limit_monorail", min_corner_limit[waytype_t(monorail_wt)]);
	max_corner_adjustment_factor[waytype_t(monorail_wt)] = contents.get_int("max_corner_adjustment_factor_monorail", max_corner_adjustment_factor[waytype_t(monorail_wt)]);
	min_corner_adjustment_factor[waytype_t(monorail_wt)] = contents.get_int("min_corner_adjustment_factor_monorail", min_corner_adjustment_factor[waytype_t(monorail_wt)]);
	min_direction_steps[waytype_t(monorail_wt)] = contents.get_int("min_direction_steps_monorail", min_direction_steps[waytype_t(monorail_wt)]);
	max_direction_steps[waytype_t(monorail_wt)] = contents.get_int("max_direction_steps_monorail", max_direction_steps[waytype_t(monorail_wt)]);
	curve_friction_factor[waytype_t(monorail_wt)] = contents.get_int("curve_friction_factor_monorail", curve_friction_factor[waytype_t(monorail_wt)]);

	max_corner_limit[waytype_t(maglev_wt)] = contents.get_int("max_corner_limit_maglev", max_corner_limit[waytype_t(maglev_wt)]);
	min_corner_limit[waytype_t(maglev_wt)] = contents.get_int("min_corner_limit_maglev", min_corner_limit[waytype_t(maglev_wt)]);
	max_corner_adjustment_factor[waytype_t(maglev_wt)] = contents.get_int("max_corner_adjustment_factor_maglev", max_corner_adjustment_factor[waytype_t(maglev_wt)]);
	min_corner_adjustment_factor[waytype_t(maglev_wt)] = contents.get_int("min_corner_adjustment_factor_maglev", min_corner_adjustment_factor[waytype_t(maglev_wt)]);
	min_direction_steps[waytype_t(maglev_wt)] = contents.get_int("min_direction_steps_maglev", min_direction_steps[waytype_t(maglev_wt)] );
	max_direction_steps[waytype_t(maglev_wt)] = contents.get_int("max_direction_steps_maglev", max_direction_steps[waytype_t(maglev_wt)]);
	curve_friction_factor[waytype_t(maglev_wt)] = contents.get_int("curve_friction_factor_maglev", curve_friction_factor[waytype_t(maglev_wt)]);

	max_corner_limit[waytype_t(narrowgauge_wt)] = contents.get_int("max_corner_limit_narrowgauge", max_corner_limit[waytype_t(narrowgauge_wt)]);
	min_corner_limit[waytype_t(narrowgauge_wt)] = contents.get_int("min_corner_limit_narrowgauge", min_corner_limit[waytype_t(narrowgauge_wt)]);
	max_corner_adjustment_factor[waytype_t(narrowgauge_wt)]  =  contents.get_int("max_corner_adjustment_factor_narrowgauge", max_corner_adjustment_factor[waytype_t(narrowgauge_wt)]);
	min_corner_adjustment_factor[waytype_t(narrowgauge_wt)] = contents.get_int("min_corner_adjustment_factor_narrowgauge", min_corner_adjustment_factor[waytype_t(narrowgauge_wt)]);
	min_direction_steps[waytype_t(narrowgauge_wt)] = contents.get_int("min_direction_steps_narrowgauge", min_direction_steps[waytype_t(narrowgauge_wt)]);
	max_direction_steps[waytype_t(narrowgauge_wt)] = contents.get_int("max_direction_steps_narrowgauge", max_direction_steps[waytype_t(narrowgauge_wt)]);
	curve_friction_factor[waytype_t(narrowgauge_wt)] = contents.get_int("curve_friction_factor_narrowgauge", curve_friction_factor[waytype_t(narrowgauge_wt)]);

	// Factory settings
	factory_max_years_obsolete = contents.get_int("max_years_obsolete", factory_max_years_obsolete);

	// @author: jamespetts
	// Insolvency and debt settings
	interest_rate_percent = contents.get_int("interest_rate_percent", interest_rate_percent);
	allow_bankruptsy = contents.get_int("allow_bankruptsy", allow_bankruptsy);
	allow_purhcases_when_insolvent = contents.get_int("allow_purhcases_when_insolvent", allow_purhcases_when_insolvent);

	// Reversing settings
	// @author: jamespetts
	unit_reverse_time = contents.get_int("unit_reverse_time", unit_reverse_time);
	hauled_reverse_time = contents.get_int("hauled_reverse_time", hauled_reverse_time);
	turntable_reverse_time = contents.get_int("turntable_reverse_time", turntable_reverse_time);

	// Global power factor
	// @author: jamespetts
	global_power_factor_percent = contents.get_int("global_power_factor_percent", global_power_factor_percent);

	// How and whether weight limits are enforced.
	// @author: jamespetts
	enforce_weight_limits = contents.get_int("enforce_weight_limits", enforce_weight_limits);

	speed_bonus_multiplier_percent = contents.get_int("speed_bonus_multiplier_percent", speed_bonus_multiplier_percent);

	bool path_searching_approach = contents.get_int("path_searching_approach", default_path_option == 2);
	default_path_option = path_searching_approach ? 2 : 1;

	// Multiply by 10 because journey times are measured in tenths of minutes.
	//@author: jamespetts
	const uint16 min_local_tolerance_minutes = contents.get_int("min_local_tolerance", (min_local_tolerance / 10));
	min_local_tolerance = min_local_tolerance_minutes * 10;
	const uint16 max_local_tolerance_minutes = contents.get_int("max_local_tolerance", (max_local_tolerance / 10));
	max_local_tolerance = max_local_tolerance_minutes * 10;
	const uint16 min_midrange_tolerance_minutes = contents.get_int("min_midrange_tolerance", (min_midrange_tolerance/ 10));
	min_midrange_tolerance = min_midrange_tolerance_minutes * 10;
	const uint16 max_midrange_tolerance_minutes = contents.get_int("max_midrange_tolerance", (max_midrange_tolerance / 10));
	max_midrange_tolerance = max_midrange_tolerance_minutes * 10;
	const uint16 min_longdistance_tolerance_minutes = contents.get_int("min_longdistance_tolerance", (min_longdistance_tolerance / 10));
	min_longdistance_tolerance = min_longdistance_tolerance_minutes * 10;
	const uint16 max_longdistance_tolerance_minutes = contents.get_int("max_longdistance_tolerance", (max_longdistance_tolerance / 10));
	max_longdistance_tolerance = max_longdistance_tolerance_minutes * 10;

	const uint16 max_walking_distance_m = contents.get_int("max_walking_distance_km_tenth", (max_walking_distance * meters_per_tile) / 100) * 100;
	max_walking_distance = ((uint32)max_walking_distance_m * distance_per_tile) / 100;

	quick_city_growth = (bool)(contents.get_int("quick_city_growth", quick_city_growth));

	assume_everywhere_connected_by_road = (bool)(contents.get_int("assume_everywhere_connected_by_road", assume_everywhere_connected_by_road));

	for(uint8 i = road_wt; i <= air_wt; i ++)
	{
		std::string buf;
		switch(i)
		{
		case road_wt:
			buf = "default_increase_maintenance_after_years_road";
			break;
		case track_wt:
			buf = "default_increase_maintenance_after_years_rail";
			break;
		case water_wt:
			buf = "default_increase_maintenance_after_years_water";
			break;
		case monorail_wt:
			buf = "default_increase_maintenance_after_years_monorail";
			break;
		case maglev_wt:
			buf = "default_increase_maintenance_after_years_maglev";
			break;
		case tram_wt:
			buf = "default_increase_maintenance_after_years_tram";
			break;
		case narrowgauge_wt:
			buf = "default_increase_maintenance_after_years_narrowgauge";
			break;
		case air_wt:
			buf = "default_increase_maintenance_after_years_air";
			break;
		default:
			buf = "default_increase_maintenance_after_years_other";
		}
		default_increase_maintenance_after_years[i] = contents.get_int(buf.c_str(), default_increase_maintenance_after_years[i]);
	}

	city_threshold_size  = contents.get_int("city_threshold_size", city_threshold_size);
	capital_threshold_size  = contents.get_int("capital_threshold_size", capital_threshold_size);
	spacing_shift_mode = contents.get_int("spacing_shift_mode", spacing_shift_mode);
	spacing_shift_divisor = contents.get_int("spacing_shift_divisor", spacing_shift_divisor);


	for(int i = 0; i < 65336; i ++)
	{
		char name[128] ;
		sprintf( name, "livery_scheme[%i]", i );
		const char* scheme_name = ltrim(contents.get(name));
		if(scheme_name[0] == '\0')
		{
			break;
		}
		
		sprintf( name, "retire_year[%i]", i );
		uint16 retire = contents.get_int(name, DEFAULT_RETIRE_DATE) * 12;

		sprintf( name, "retire_month[%i]", i );
		retire += contents.get_int(name, 1) - 1;

		livery_scheme_t* scheme = new livery_scheme_t(scheme_name, retire);

		bool has_liveries = false;
		for(int j = 0; j < 65536; j ++)
		{
			char livery[128];
			sprintf(livery, "livery[%i][%i]", i, j);
			const char* liv_name = ltrim(contents.get(livery));
			if(liv_name[0] == '\0')
			{
				break;
			}

			has_liveries = true;
			sprintf(livery, "intro_year[%i][%i]", i, j);
			uint16 intro = contents.get_int(livery, DEFAULT_INTRO_DATE) * 12;

			sprintf(livery, "intro_month[%i][%i]", i, j);
			intro += contents.get_int("intro_month", 1) - 1;

			scheme->add_livery(liv_name, intro);
			livery_t liv = {liv_name, intro};
		}
		if(has_liveries)
		{
			livery_schemes.append(scheme);
		}
		else
		{
			delete scheme;
		}
	}

	/*
	 * Selection of savegame format through inifile
	 */
	str = contents.get("saveformat" );
	while (*str == ' ') str++;
	if (strcmp(str, "binary") == 0) {
		loadsave_t::set_savemode(loadsave_t::binary );
	} else if(strcmp(str, "zipped") == 0) {
		loadsave_t::set_savemode(loadsave_t::zipped );
	} else if(strcmp(str, "xml") == 0) {
		loadsave_t::set_savemode(loadsave_t::xml );
	} else if(strcmp(str, "xml_zipped") == 0) {
		loadsave_t::set_savemode(loadsave_t::xml_zipped );
	} else if(strcmp(str, "bzip2") == 0) {
		loadsave_t::set_savemode(loadsave_t::bzip2 );
	} else if(strcmp(str, "xml_bzip2") == 0) {
		loadsave_t::set_savemode(loadsave_t::xml_bzip2 );
	}

	/*
	 * Default resolution
	 */
	disp_width = contents.get_int("display_width", disp_width );
	disp_height = contents.get_int("display_height", disp_height );
	fullscreen = contents.get_int("fullscreen", fullscreen );

	with_private_paks = contents.get_int("with_private_paks", with_private_paks)!=0;

	// Default pak file path
	objfilename = ltrim(contents.get_string("pak_file_path", ""));

	printf("Reading simuconf.tab successful!\n" );

	simuconf.close( );
}


int settings_t::get_name_language_id() const
{
	int lang = -1;
	if(  umgebung_t::networkmode  ) {
		lang = translator::get_language( language_code_names );
	}
	if(  lang == -1  ) {
		lang = translator::get_language();
	}
	return lang;
}


sint64 settings_t::get_starting_money(sint16 const year) const
{
	if(  starting_money>0  ) {
		return starting_money;
	}

	// search entry with startingmoneyperyear[i].year > year
	int i;
	for(  i=0;  i<10;  i++  ) {
		if(startingmoneyperyear[i].year!=0) {
			if (startingmoneyperyear[i].year>year) {
				break;
			}
		}
		else {
			// year is behind the latest given date
			return startingmoneyperyear[i>0 ? i-1 : 0].money;
		}
	}
	if (i==0) {
		// too early: use first entry
		return startingmoneyperyear[0].money;
	}
	else {
		// now: startingmoneyperyear[i-1].year <= year < startingmoneyperyear[i].year
		if (startingmoneyperyear[i-1].interpol) {
			// linear interpolation
			return startingmoneyperyear[i-1].money +
				(startingmoneyperyear[i].money-startingmoneyperyear[i-1].money)
				* (year-startingmoneyperyear[i-1].year) /(startingmoneyperyear[i].year-startingmoneyperyear[i-1].year );
		}
		else {
			// no interpolation
			return startingmoneyperyear[i-1].money;
		}

	}
}


/**
 * returns newest way-besch for road_timeline_t arrays
 * @param road_timeline_t must be an array with at least num_roads elements, no range checks!
 */
static const weg_besch_t *get_timeline_road_type( uint16 year, uint16 num_roads, road_timeline_t* roads)
{
	const weg_besch_t *besch = NULL;
	const weg_besch_t *test;
	for(  int i=0;  i<num_roads;  i++  ) {
		test = wegbauer_t::get_besch( roads[i].name, 0 );
		if(  test  ) {
			// return first available for no timeline
			if(  year==0  ) {
				return test;
			}
			if(  roads[i].intro==0  ) {
				// fill in real intro date
				roads[i].intro = test->get_intro_year_month( );
			}
			if(  roads[i].retire==0  ) {
				// fill in real retire date
				roads[i].retire = test->get_retire_year_month( );
				if(  roads[i].retire==0  ) {
					roads[i].retire = NEVER;
				}
			}
			// find newest available ...
			if(  year>=roads[i].intro  &&  year<roads[i].retire  ) {
				if(  besch==0  ||  besch->get_intro_year_month()<test->get_intro_year_month()  ) {
					besch = test;
				}
			}
		}
	}
	return besch;
}


weg_besch_t const* settings_t::get_city_road_type(uint16 const year)
{
	return get_timeline_road_type(year, num_city_roads, city_roads );
}


weg_besch_t const* settings_t::get_intercity_road_type(uint16 const year)
{
	return get_timeline_road_type(year, num_intercity_roads, intercity_roads );
}


void settings_t::copy_city_road(settings_t const& other)
{
	num_city_roads = other.num_city_roads;
	for(  int i=0;  i<10;  i++  ) {
		city_roads[i] = other.city_roads[i];
	}
}


// returns default player colors for new players
void settings_t::set_default_player_color(spieler_t* const sp) const
{
	COLOR_VAL color1 = default_player_color[sp->get_player_nr()][0];
	if(  color1 == 255  ) {
		if(  default_player_color_random  ) {
			// build a vector with all colors
			minivec_tpl<uint8>all_colors1(28);
			for(  uint8 i=0;  i<28;  i++  ) {
				all_colors1.append(i);
			}
			// remove all used colors
			for(  uint8 i=0;  i<MAX_PLAYER_COUNT;  i++  ) {
				spieler_t *test_sp = sp->get_welt()->get_spieler(i);
				if(  test_sp  &&  sp!=test_sp  ) {
					uint8 rem = 1<<(sp->get_player_color1()/8);
					if(  all_colors1.is_contained(rem)  ) {
						all_colors1.remove( rem );
					}
				}
				else if(  default_player_color[i][0]!=255  ) {
					uint8 rem = default_player_color[i][0];
					if(  all_colors1.is_contained(rem)  ) {
						all_colors1.remove( rem );
					}
				}
			}
			// now choose a random empty color
			color1 = pick_any(all_colors1);
		}
		else {
			color1 = sp->get_player_nr();
		}
	}

	COLOR_VAL color2 = default_player_color[sp->get_player_nr()][1];
	if(  color2 == 255  ) {
		if(  default_player_color_random  ) {
			// build a vector with all colors
			minivec_tpl<uint8>all_colors2(28);
			for(  uint8 i=0;  i<28;  i++  ) {
				all_colors2.append(i);
			}
			// remove color1
			all_colors2.remove( color1/8 );
			// remove all used colors
			for(  uint8 i=0;  i<MAX_PLAYER_COUNT;  i++  ) {
				spieler_t *test_sp = sp->get_welt()->get_spieler(i);
				if(  test_sp  &&  sp!=test_sp  ) {
					uint8 rem = 1<<(sp->get_player_color2()/8);
					if(  all_colors2.is_contained(rem)  ) {
						all_colors2.remove( rem );
					}
				}
				else if(  default_player_color[i][1]!=255  ) {
					uint8 rem = default_player_color[i][1];
					if(  all_colors2.is_contained(rem)  ) {
						all_colors2.remove( rem );
					}
				}
			}
			// now choose a random empty color
			color2 = pick_any(all_colors2);
		}
		else {
			color2 = sp->get_player_nr() + 3;
		}
	}

	sp->set_player_color( color1*8, color2*8 );
}
 
