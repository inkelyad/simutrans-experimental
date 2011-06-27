#include <string>
#include "../../utils/simstring.h"
#include "../../dataobj/tabfile.h"
#include "../haus_besch.h"
#include "../vehikel_besch.h"
#include "obj_pak_exception.h"
#include "obj_node.h"
#include "text_writer.h"
#include "imagelist2d_writer.h"
#include "get_waytype.h"
#include "get_climate.h"
#include "building_writer.h"
#include "skin_writer.h"

using std::string;

/**
 * Calculate numeric engine type from engine type string
 */
static uint8 get_engine_type(const char* engine_type, tabfileobj_t& obj)
{
	uint8 uv8 = vehikel_besch_t::diesel;

	if (!STRICMP(engine_type, "diesel")) {
		uv8 = vehikel_besch_t::diesel;
	} else if (!STRICMP(engine_type, "electric")) {
		uv8 = vehikel_besch_t::electric;
	} else if (!STRICMP(engine_type, "steam")) {
		uv8 = vehikel_besch_t::steam;
	} else if (!STRICMP(engine_type, "bio")) {
		uv8 = vehikel_besch_t::bio;
	} else if (!STRICMP(engine_type, "sail")) {
		uv8 = vehikel_besch_t::sail;
	} else if (!STRICMP(engine_type, "fuel_cell")) {
		uv8 = vehikel_besch_t::fuel_cell;
	} else if (!STRICMP(engine_type, "hydrogene")) {
		uv8 = vehikel_besch_t::hydrogene;
	} else if (!STRICMP(engine_type, "battery")) {
		uv8 = vehikel_besch_t::battery;
	} else if (!STRICMP(engine_type, "unknown")) {
		uv8 = vehikel_besch_t::unknown;
	}

	// printf("Engine type %s -> %d\n", engine_type, uv8);

	return uv8;
}

void tile_writer_t::write_obj(FILE* fp, obj_node_t& parent, int index, int seasons,
	slist_tpl<slist_tpl<slist_tpl<string> > >& backkeys,
	slist_tpl<slist_tpl<slist_tpl<string> > >& frontkeys
)
{
	obj_node_t node(this, 7, &parent);

	uint8 phasen = 0;
	for (int i = 0; i < seasons; i++) {
		slist_iterator_tpl<slist_tpl<string> > iter(backkeys.at(i));
		while (iter.next()) {
			if (iter.get_current().get_count() > phasen) {
				phasen = iter.get_current().get_count();
			}
		}
		iter = slist_iterator_tpl<slist_tpl<string> >(frontkeys.at(i));
		while (iter.next()) {
			if (iter.get_current().get_count() > phasen) {
				phasen = iter.get_current().get_count();
			}
		}
	}

	for (int i = 0; i < seasons; i++) {
		imagelist2d_writer_t::instance()->write_obj(fp, node, backkeys.at(i));
		imagelist2d_writer_t::instance()->write_obj(fp, node, frontkeys.at(i));
	}

	// Hajo: temp vars of appropriate size
	uint16 v16;

	// Set version data
	v16 = 0x8002;

	// Write version data
	node.write_uint16(fp, v16, 0);

	v16 = phasen;
	node.write_uint16(fp, v16, 2);

	v16 = index;
	node.write_uint16(fp, v16, 4);

	uint8 uv8 = seasons;
	node.write_uint8(fp, uv8, 6);

	node.write(fp);
}


void building_writer_t::write_obj(FILE* fp, obj_node_t& parent, tabfileobj_t& obj)
{
	// Hajo: take care, hardocded size of node on disc here!
	obj_node_t node(this, 38, &parent);

	write_head(fp, node, obj);

	koord groesse(1, 1);
	uint8 layouts = 0;

	int* ints = obj.get_ints("dims");

	switch (ints[0]) {
		default:
		case 3: layouts   = ints[3]; /* FALLTHROUGH */
		case 2: groesse.y = ints[2]; /* FALLTHROUGH */
		case 1: groesse.x = ints[1]; /* FALLTHROUGH */
		case 0: break;
	}
	if (layouts == 0) {
		layouts = groesse.x == groesse.y ? 1 : 2;
	}
	delete [] ints;

	gebaeude_t::typ            gtyp             = gebaeude_t::unbekannt;
	haus_besch_t::utyp         utype            = haus_besch_t::unbekannt;
	uint16                     extra_data       = 0;
	climate_bits               allowed_climates = all_but_water_climate; // all but water
	uint8                      enables          = 0;
	uint16                     level            = obj.get_int("level", 1) - 1;
	haus_besch_t::flag_t const flags            =
		(obj.get_int("noinfo",         0) > 0 ? haus_besch_t::FLAG_KEINE_INFO  : haus_besch_t::FLAG_NULL) |
		(obj.get_int("noconstruction", 0) > 0 ? haus_besch_t::FLAG_KEINE_GRUBE : haus_besch_t::FLAG_NULL) |
		(obj.get_int("needs_ground",   0) > 0 ? haus_besch_t::FLAG_NEED_GROUND : haus_besch_t::FLAG_NULL);
	uint16               const animation_time   = obj.get_int("animation_time", 300);

	// get the allowed area for this building
	const char* climate_str = obj.get("climates");
	if (climate_str && strlen(climate_str) > 4) {
		allowed_climates = get_climate_bits(climate_str);
	}

	const char* type_name = obj.get("type");
	if (!STRICMP(type_name, "res")) {
		gtyp = gebaeude_t::wohnung;
	} else if (!STRICMP(type_name, "com")) {
		gtyp = gebaeude_t::gewerbe;
	} else if (!STRICMP(type_name, "ind")) {
		gtyp = gebaeude_t::industrie;
	} else if (!STRICMP(type_name, "cur")) {
		extra_data = obj.get_int("build_time", 0);
		level      = obj.get_int("passengers",  level);
		utype      = extra_data == 0 ? haus_besch_t::attraction_land : haus_besch_t::attraction_city;
	} else if (!STRICMP(type_name, "mon")) {
		utype = haus_besch_t::denkmal;
		level = obj.get_int("passengers",  level);
	} else if (!STRICMP(type_name, "tow")) {
		level      = obj.get_int("passengers",  level);
		extra_data = obj.get_int("build_time", 0);
		utype = haus_besch_t::rathaus;
	} else if (!STRICMP(type_name, "hq")) {
		level      = obj.get_int("passengers",  level);
		extra_data = obj.get_int("hq_level", 0);
		utype = haus_besch_t::firmensitz;
	} else if (!STRICMP(type_name, "habour")  ||  !STRICMP(type_name, "harbour")) {
		utype      = haus_besch_t::hafen;
		extra_data = water_wt;
	} else if (!STRICMP(type_name, "fac")) {
		utype    = haus_besch_t::fabrik;
		enables |= 4;
	} else if (!STRICMP(type_name, "stop")) {
		utype      = haus_besch_t::generic_stop;
		extra_data = get_waytype(obj.get("waytype"));
	} else if (!STRICMP(type_name, "extension")) {
		utype = haus_besch_t::generic_extension;
		const char *wt = obj.get("waytype");
		if(wt  &&  *wt>' ') {
			// not waytype => just a generic exten that fits all
			extra_data = get_waytype(wt);
		}
	} else if (!STRICMP(type_name, "depot")) {
		utype      = haus_besch_t::depot;
		extra_data = get_waytype(obj.get("waytype"));
	} else if (!STRICMP(type_name, "any") || *type_name == '\0') {
		// for instance "MonorailGround"
		utype = haus_besch_t::weitere;
	} else if (
		!STRICMP(type_name, "station")  ||
		!STRICMP(type_name, "railstop")  ||
		!STRICMP(type_name, "monorailstop")  ||
		!STRICMP(type_name, "busstop")  ||
		!STRICMP(type_name, "carstop")  ||
		!STRICMP(type_name, "airport")  ||
		!STRICMP(type_name, "wharf")
	) {
		dbg->fatal("building_writer_t::write_obj()","%s is obsolete type for %s; use stop/extension and waytype!", type_name, obj.get("name") );
	} else if (!STRICMP(type_name, "hall")  ||  !STRICMP(type_name, "post")  ||  !STRICMP(type_name, "shed")  ) {
		dbg->fatal("building_writer_t::write_obj()","%s is obsolete type for %s; use extension and waytype!", type_name, obj.get("name") );
	} else {
		dbg->fatal( "building_writer_t::write_obj()","%s is obsolete type for %s", type_name, obj.get("name") );
	}

	// is is an station extension building?
	if (obj.get_int("extension_building", 0) > 0) {
		dbg->fatal("building_writer_t::write_obj()","extension_building is obsolete keyword for %s; use stop/extension and waytype!", obj.get("name") );
	}

	if (obj.get_int("enables_pax", 0) > 0) {
		enables |= 1;
	}
	if (obj.get_int("enables_post", 0) > 0) {
		enables |= 2;
	}
	if (utype == haus_besch_t::fabrik || obj.get_int("enables_ware", 0) > 0) {
		enables |= 4;
	}

	// some station thing ...
	if (enables) {
		++level;
	}

	// Hajo: read chance - default is 100% chance to be built
	uint8 const chance = obj.get_int("chance", 100);

	// prissi: timeline for buildings
	uint16 const intro_date =
		obj.get_int("intro_year", DEFAULT_INTRO_DATE) * 12 +
		obj.get_int("intro_month", 1) - 1;

	uint16 const obsolete_date =
		obj.get_int("retire_year", DEFAULT_RETIRE_DATE) * 12 +
		obj.get_int("retire_month", 1) - 1;

	// @author: jamespetts
	// Station-specific capacity and price information.
	// Stands in place of the "level" setting, but uses "level" data by default.

	sint32 station_capacity = obj.get_int("station_capacity", level * 32);
	sint32 station_maintenance = obj.get_int("station_maintenance", 2147483647); //NOTE: Default cannot be set because it depends on a world factor. Must detect this number and put in default if it is found.
	sint32 station_price = obj.get_int("station_price", 2147483647);
	uint16 compatibility_group = obj.get_int("compatibility_group", 0);

	// Encode the depot traction types.
	if(utype == haus_besch_t::depot)
	{
		// HACK: Use "enables" (only used for a stop type) to encode traction types
		enables = 255; // Default - all types enabled.
		
		uint8 traction_type_count = 0;
		uint8 traction_type = 1;
		bool engaged = false;
		string engine_type;
		do {
			char buf[256];

			sprintf(buf, "traction_type[%d]", traction_type_count);

			engine_type = obj.get(buf);
			if (engine_type.length() > 0) 
			{
				if(!engaged)
				{
					// If the user specifies anything, the
					// default of 255 must be cleared.
					engaged = true;
					enables = 0;
				}
				traction_type = get_engine_type(engine_type.c_str(), obj);
				const uint8 shifter = 1 << traction_type;
				enables |= shifter;
				traction_type_count++;
			}
		} while (engine_type.length() > 0);			
	}

	// scan for most number of seasons
	int seasons = 1;
	for (int l = 0; l < layouts; l++) { // each layout
		int const h = l & 1 ? groesse.x : groesse.y;
		int const w = l & 1 ? groesse.y : groesse.x;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) { // each tile
				for (int pos = 0; pos < 2; pos++) {
					for (int season = seasons; season < 12; season++) {
						char buf[40];
						sprintf(buf, "%simage[%d][%d][%d][%d][%d][%d]", pos ? "back" : "front", l, y, x, 0, 0, season);
						string str = obj.get(buf);
						if (str.size() != 0) {
							seasons = season + 1;
						} else {
							break;
						}
					}
				}
			}
		}
	}

	int tile_index = 0;
	for (int l = 0; l < layouts; l++) { // each layout
		int const h = l & 1 ? groesse.x : groesse.y;
		int const w = l & 1 ? groesse.y : groesse.x;
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) { // each tile
				slist_tpl<slist_tpl<slist_tpl<string> > > backkeys;
				slist_tpl<slist_tpl<slist_tpl<string> > > frontkeys;

				for (int season = 0; season < seasons; season++) {
					backkeys.append(slist_tpl<slist_tpl<string> >());
					frontkeys.append(slist_tpl<slist_tpl<string> >());

					for (int pos = 0; pos < 2; pos++) {
						slist_tpl<slist_tpl<slist_tpl<string> > >& keys = pos ? backkeys : frontkeys;

						for (unsigned int h = 0; ; h++) { // each height
							for (int phase = 0; ; phase++) { // each animation
								char buf[40];

								sprintf(buf, "%simage[%d][%d][%d][%d][%d][%d]", pos ? "back" : "front", l, y, x, h, phase, season);
								string str = obj.get(buf);

								// if no string check to see whether using format without seasons parameter
								if (str.size() == 0 && seasons == 1) {
									sprintf(buf, "%simage[%d][%d][%d][%d][%d]", pos ? "back" : "front", l, y, x, h, phase);
									str = obj.get(buf);
								}
								if (str.size() == 0) {
#if 0
									printf("Not found: %s\n", buf);
									fflush(NULL);
#endif
									break;
								} else {
									// no higher front images
									if (h > 0 && pos == 0) {
										printf("WARNING: frontimage height MUST be one tile only!\n");
										fflush(NULL);
										break;
									}
								}
								if (phase == 0) {
									keys.at(season).append(slist_tpl<string>());
								}
								keys.at(season).at(h).append(str);
							}
							if (keys.at(season).get_count() <= h) {
								break;
							}
						}
					}
				}
				tile_writer_t::instance()->write_obj(fp, node, tile_index++, seasons, backkeys, frontkeys);
			}
		}
	}

	uint16 version = 0x8005;
	
	// This is the overlay flag for Simutrans-Experimental
	// This sets the *second* highest bit to 1. 
	version |= EXP_VER;

	// Finally, this is the experimental version number. This is *added*
	// to the standard version number, to be subtracted again when read.
	// Start at 0x100 and increment in hundreds (hex).
	// 0x200 - Depot traction types.
	// 0x300 - Compatibility group
	version += 0x300;
	
	// Hajo: write version data

	node.write_uint16(fp, version,				0);

	// Hajo: write besch data
	node.write_uint8 (fp, gtyp,					2);
	node.write_uint8 (fp, utype,				3);
	node.write_uint16(fp, level,				4);
	node.write_uint32(fp, extra_data,			6);
	node.write_uint16(fp, groesse.x,			10);
	node.write_uint16(fp, groesse.y,			12);
	node.write_uint8 (fp, layouts,				14);
	node.write_uint16(fp, allowed_climates,		15);
	node.write_uint8 (fp, enables,				17);
	node.write_uint8 (fp, flags,				18);
	node.write_uint8 (fp, chance,				19);
	node.write_uint16(fp, intro_date,			20);
	node.write_uint16(fp, obsolete_date,		22);
	node.write_uint16(fp, animation_time,		24);
	node.write_uint16(fp, station_capacity,		26);
	node.write_sint32(fp, station_maintenance,	28);
	node.write_sint32(fp, station_price,		32);
	node.write_uint16(fp, compatibility_group,	36);

	// probably add some icons, if defined
	slist_tpl<string> cursorkeys;

	string c = string(obj.get("cursor"));
	string i = string(obj.get("icon"));
	cursorkeys.append(c);
	cursorkeys.append(i);
	if (c.size() > 0 || i.size() > 0) {
		cursorskin_writer_t::instance()->write_obj(fp, node, obj, cursorkeys);
	}
	node.write(fp);
}
