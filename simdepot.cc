/*
 * Copyright (c) 1997 - 2001 Hj. Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "simintr.h"

#include "simconvoi.h"
#include "vehicle/simvehikel.h"
#include "simwin.h"
#include "simware.h"
#include "simhalt.h"
#include "player/simplay.h"
#include "simworld.h"
#include "simdepot.h"
#include "simline.h"
#include "simlinemgmt.h"
#include "simwerkz.h"

#include "gui/depot_frame.h"
#include "gui/messagebox.h"

#include "dataobj/fahrplan.h"
#include "dataobj/loadsave.h"
#include "dataobj/translator.h"

#include "bauer/hausbauer.h"
#include "dings/gebaeude.h"

#include "bauer/vehikelbauer.h"

#include "boden/wege/schiene.h"

#include "besch/haus_besch.h"

#define CREDIT_MESSAGE "That would exceed\nyour credit limit."
#include "utils/cbuffer_t.h"

slist_tpl<depot_t *> depot_t::all_depots;

depot_t::depot_t(karte_t *welt,loadsave_t *file) : gebaeude_t(welt)
{
	rdwr(file);
	if(file->get_version()<88002) {
		set_yoff(0);
	}
	all_depots.append(this);
}



depot_t::depot_t(karte_t *welt, koord3d pos, spieler_t *sp, const haus_tile_besch_t *t) :
    gebaeude_t(welt, pos, sp, t)
{
	all_depots.append(this);
}


depot_t::~depot_t()
{
	destroy_win((long)this);
	all_depots.remove(this);
}


// finds the next/previous depot relative to the current position
depot_t *depot_t::find_depot( koord3d start, const ding_t::typ depot_type, const spieler_t *sp, bool forward)
{
	depot_t *found = NULL;
	koord3d found_pos = forward ? koord3d(welt->get_groesse_x()+1,welt->get_groesse_y()+1,welt->get_grundwasser()) : koord3d(-1,-1,-1);
	long found_hash = forward ? 0x7FFFFFF : -1;
	long start_hash = start.x + (8192*start.y);
	slist_iterator_tpl<depot_t *> iter(all_depots);
	while(iter.next()) {
		depot_t *d = iter.access_current();
		if(d->get_typ()==depot_type  &&  d->get_besitzer()==sp) {
			// ok, the right type of depot
			const koord3d pos = d->get_pos();
			if(pos==start) {
				// ignore the start point
				continue;
			}
			long hash = (pos.x+(8192*pos.y));
			if(forward) {
				if(hash>start_hash  ||  (hash==start_hash  &&  pos.z>start.z)) {
				// found a suitable one
					if(hash<found_hash  ||  (hash==found_hash  &&  pos.z<found_pos.z)) {
						// which is closer ...
						found = d;
						found_pos = pos;
						found_hash = hash;
					}
				}
			}
			else {
				// search to start of the map
				if(hash<start_hash  ||  (hash==start_hash  &&  pos.z<start.z)) {
				// found a suitable one
					if(hash>found_hash  ||  (hash==found_hash  &&  pos.z>found_pos.z)) {
						// which is closer ...
						found = d;
						found_pos = pos;
						found_hash = hash;
					}
				}
			}
		}
	}
	return found;
}

unsigned depot_t::get_max_convoy_length(waytype_t wt)
{
	if (wt==road_wt || wt==water_wt) {
		return 4;
	}
	if (wt==air_wt) {
		return 1;
	}
	return convoi_t::max_rail_vehicle;
}


// again needed for server
void depot_t::call_depot_tool( char tool, convoihandle_t cnv, const char *extra, uint16 livery_scheme_index)
{
	// call depot tool
	werkzeug_t *w = create_tool( WKZ_DEPOT_TOOL | SIMPLE_TOOL );
	cbuffer_t buf;
	buf.printf( "%c,%s,%hu,%hu", tool, get_pos().get_str(), cnv.get_id(), livery_scheme_index );
	if(  extra  ) {
		buf.append( "," );
		buf.append( extra );
	}
	w->set_default_param(buf);
	welt->set_werkzeug( w, get_besitzer() );
	// since init always returns false, it is safe to delete immediately
	delete w;
}



/* this is called on two occasions:
 * first a convoy reaches the depot during its journey
 * second during loading a convoi is stored in a depot => only store it again
 */
void depot_t::convoi_arrived(convoihandle_t acnv, bool fpl_adjust)
{
	if(fpl_adjust) {
		// here a regular convoi arrived

		for(unsigned i=0; i<acnv->get_vehikel_anzahl(); i++) {
			vehikel_t *v = acnv->get_vehikel(i);
			// Hajo: reset vehikel data
			v->loesche_fracht();
			v->set_pos( koord3d::invalid );
			v->set_erstes( i==0 );
			v->set_letztes( i+1==acnv->get_vehikel_anzahl() );
		}
		// Volker: remove depot from schedule
		schedule_t *fpl = acnv->get_schedule();
		for(  int i=0;  i<fpl->get_count();  i++  ) {
			// only if convoi found
			if(fpl->eintrag[i].pos==get_pos()) {
				fpl->set_aktuell( i );
				fpl->remove();
				acnv->set_schedule(fpl);
				break;
			}
		}
	}
	// this part stores the convoi in the depot
	convois.append(acnv);
	depot_frame_t *depot_frame = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
	if(depot_frame) {
		depot_frame->action_triggered(NULL,(long int)0);
	}
	acnv->set_home_depot( get_pos() );
	DBG_MESSAGE("depot_t::convoi_arrived()", "convoi %d, %p entered depot", acnv.get_id(), acnv.get_rep());
}


void depot_t::zeige_info()
{
	create_win(20, 20, new depot_frame_t(this), w_info, (long)this);
}


vehikel_t* depot_t::buy_vehicle(const vehikel_besch_t* info, uint16 livery_scheme_index)
{
	DBG_DEBUG("depot_t::buy_vehicle()", info->get_name());
	vehikel_t* veh = vehikelbauer_t::baue(get_pos(), get_besitzer(), NULL, info, false, livery_scheme_index); //"besitzer" = "owner" (Google)
	DBG_DEBUG("depot_t::buy_vehicle()", "vehiclebauer %p", veh);
	vehicles.append(veh);
	DBG_DEBUG("depot_t::buy_vehicle()", "appended %i vehicle", vehicles.get_count());
	return veh;
}

void depot_t::upgrade_vehicle(convoihandle_t cnv, const vehikel_besch_t* vb)
{
	if(!cnv.is_bound())
	{
		return;
	}

	for(uint16 i = 0; i < cnv->get_vehikel_anzahl(); i ++)
	{
		for(uint8 c = 0; c < cnv->get_vehikel(i)->get_besch()->get_upgrades_count(); c ++)
		{
			if(cnv->get_vehikel(i)->get_besch()->get_upgrades(c) == vb)
			{
				vehikel_t* new_veh = vehikelbauer_t::baue(get_pos(), get_besitzer(), NULL, vb, true, cnv->get_livery_scheme_index()); 
				cnv->upgrade_vehicle(i, new_veh);
				if(cnv->get_vehikel(i)->get_besch()->get_nachfolger_count() == 1 && cnv->get_vehikel(i)->get_besch()->get_leistung() != 0)
				{
					//We need to upgrade tenders, too.	
					vehikel_t* new_veh_2 = vehikelbauer_t::baue(get_pos(), get_besitzer(), NULL, new_veh->get_besch()->get_nachfolger(0), true); 
					cnv->upgrade_vehicle(i + 1, new_veh_2);
					// The above assumes that tenders are free, which they are in Pak128.Britain, the cost being built into the locomotive.
					// The below ought work more accurately, but does not work properly, for some reason.

					/*if(cnv->get_vehikel(i + 1)->get_besch()->get_upgrades_count() >= c && cnv->get_vehikel(i + 1)->get_besch()->get_upgrades_count() > 0)
					{
						cnv->get_vehikel(i + 1)->set_besch(cnv->get_vehikel(i + 1)->get_besch()->get_upgrades(c));
					}	
					else if(cnv->get_vehikel(i + 1)->get_besch()->get_upgrades_count() > 0)
					{
						cnv->get_vehikel(i + 1)->set_besch(cnv->get_vehikel(i + 1)->get_besch()->get_upgrades(0));
					}*/					
				}		
				//Check whether this is a Garrett type vehicle (this is code for the exceptional case where a Garrett is upgraded to another Garrett)
				if(cnv->get_vehikel(0)->get_besch()->get_leistung() == 0 && cnv->get_vehikel(0)->get_besch()->get_zuladung() == 0)
				{
					// Possible Garrett
					const uint8 count = cnv->get_vehikel(0)->get_besch()->get_nachfolger_count();
					if(count > 0 && cnv->get_vehikel(1)->get_besch()->get_leistung() > 0 && cnv->get_vehikel(1)->get_besch()->get_nachfolger_count() > 0)
					{
						// Garrett detected - need to upgrade all three vehicles.
						vehikel_t* new_veh_2 = vehikelbauer_t::baue(get_pos(), get_besitzer(), NULL, new_veh->get_besch()->get_nachfolger(0), true); 
						cnv->upgrade_vehicle(i + 1, new_veh_2);
						vehikel_t* new_veh_3 = vehikelbauer_t::baue(get_pos(), get_besitzer(), NULL, new_veh->get_besch()->get_nachfolger(0), true); 
						cnv->upgrade_vehicle(i + 2, new_veh_3);
					}
				}
				// Only upgrade one at a time.
				// Could add UI feature in future to allow upgrading all at once.
				return;					
			}		
		}				
	}
}


void depot_t::append_vehicle(convoihandle_t &cnv, vehikel_t* veh, bool infront)
{
	/* create  a new convoi, if necessary */
	if (!cnv.is_bound()) {
		cnv = add_convoi();
	}
	veh->set_pos(get_pos());
	cnv->add_vehikel(veh, infront);
	vehicles.remove(veh);
}


void depot_t::remove_vehicle(convoihandle_t cnv, int ipos)
{
	vehikel_t* veh = cnv->remove_vehikel_bei(ipos);
	if (veh) {
		vehicles.append(veh);
	}
}


void depot_t::sell_vehicle(vehikel_t* veh)
{
	vehicles.remove(veh);
	get_besitzer()->buche(veh->calc_restwert(), get_pos().get_2d(), COST_NEW_VEHICLE );
	get_besitzer()->buche(-(sint64)veh->calc_restwert(), COST_ASSETS );
	DBG_MESSAGE("depot_t::sell_vehicle()", "this=%p sells %p", this, veh);
	veh->before_delete();
	delete veh;
}


convoihandle_t depot_t::add_convoi()
{
	convoi_t* new_cnv = new convoi_t(get_besitzer());
	new_cnv->set_home_depot(get_pos());
	convois.append(new_cnv->self);
	depot_frame_t *win = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
	if(  win  ) {
		win->activate_convoi( new_cnv->self );
	}
	return new_cnv->self;
}


convoihandle_t depot_t::copy_convoi(convoihandle_t old_cnv)
{
	if (old_cnv.is_bound()  &&  !convoihandle_t::is_exhausted()) 
	{
		convoihandle_t new_cnv;
		int vehicle_count = old_cnv->get_vehikel_anzahl();
		for (int i = 0; i < vehicle_count; i++) 
		{
			const vehikel_besch_t * info = old_cnv->get_vehikel(i)->get_besch();
			if (info != NULL) 
			{
				// search in depot for an existing vehicle of correct type
				vehikel_t* new_vehicle = get_oldest_vehicle(info);
				if(new_vehicle == NULL)
				{
					// no vehicle of correct type in depot, must buy it:
					//first test affordability.
					sint64 total_price = info->get_preis();
					if(!get_besitzer()->can_afford(total_price))
					{
						create_win( new news_img(CREDIT_MESSAGE), w_time_delete, magic_none);
						if(!new_cnv.is_bound() || new_cnv->get_vehikel_anzahl() == 0)
						{
							return new_cnv;
						}
						break; // ... and what happens with the first few vehicles, if you: return convoihandle_t();
						
					}
					// buy new vehicle
					new_vehicle = vehikelbauer_t::baue(get_pos(), get_besitzer(), NULL, info );
				}
				// append new vehicle
				append_vehicle(new_cnv, new_vehicle, false);
			}
		}
		if (old_cnv->get_line().is_bound()) 
		{
			if(!new_cnv.is_bound())
			{
				new_cnv = add_convoi();
			}
			new_cnv->set_line(old_cnv->get_line());
			new_cnv->get_schedule()->set_aktuell( old_cnv->get_schedule()->get_aktuell() );
		}
		else 
		{
			if (old_cnv->get_schedule() != NULL) 
			{
				if(!new_cnv.is_bound())
				{
					new_cnv = add_convoi();
				}
				new_cnv->set_schedule(old_cnv->get_schedule()->copy());
			}
		}	
		if (new_cnv.is_bound())
		{
			new_cnv->set_name(old_cnv->get_internal_name());

			// make this the current selected convoi
			depot_frame_t *win = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
			if(  win  ) {
				win->activate_convoi( new_cnv );
			}
		}

		return new_cnv;
	}
	return convoihandle_t();
}



bool depot_t::disassemble_convoi(convoihandle_t cnv, bool sell)
{
	if(cnv.is_bound()) {

		if(!sell) {
			// store vehicles in depot
			vehikel_t *v;
			while(  (v=cnv->remove_vehikel_bei(0))!=NULL  ) {
				v->loesche_fracht();
				v->set_erstes(false);
				v->set_letztes(false);
				vehicles.append(v);
			}
		}

		// remove from depot lists
		sint32 icnv = (sint32)convois.index_of( cnv );
		if (convois.remove(cnv)) {
			// actually removed cnv from depot, here icnv>=0
			// make another the current selected convoi
			depot_frame_t *win = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
			if(  win  ) {
				win->activate_convoi( !convois.empty() ? convois.at( min((uint32)icnv, convois.get_count()-1) ) : convoihandle_t() );
			}
		}

		// and remove from welt
		cnv->self_destruct();
		return true;
	}
	return false;
}


bool depot_t::start_convoi(convoihandle_t cnv, bool local_execution)
{
	// close schedule window if not yet closed
	if(cnv.is_bound() &&  cnv->get_schedule()!=NULL) {
		if(!cnv->get_schedule()->ist_abgeschlossen()) {
			// close the schedule window
			destroy_win((long)cnv->get_schedule());
		}
	}

	// convoi not in depot anymore, maybe user double-clicked on start-button
	if(!convois.is_contained(cnv)) {
		return false;
	}

	if (cnv.is_bound() && cnv->get_schedule() && !cnv->get_schedule()->empty()) {
		// if next schedule entry is this depot => advance to next entry
		const koord3d& cur_pos = cnv->get_schedule()->get_current_eintrag().pos;
		if (cur_pos == get_pos()) {
			cnv->get_schedule()->advance();
		}

		// check if convoi is complete
		if(cnv->get_sum_leistung() == 0 || !cnv->pruefe_alle()) {
			if (local_execution) {
				create_win( new news_img("Diese Zusammenstellung kann nicht fahren!\n"), w_time_delete, magic_none);
			}
		}
		else if(  !cnv->front()->calc_route(this->get_pos(), cur_pos, cnv->get_min_top_speed(), cnv->access_route())  ) {
			// no route to go ...
			if(local_execution) {
				static char buf[256];
				sprintf(buf,translator::translate("Vehicle %s can't find a route!"), cnv->get_name());
				create_win( new news_img(buf), w_time_delete, magic_none);
			}
		}
		else {
			// convoi can start now
			welt->sync_add( cnv.get_rep() );
			cnv->start();

			// remove from depot lists
			sint32 icnv = (sint32)convois.index_of( cnv );
			if (convois.remove(cnv)) {
				// actually removed cnv from depot, here icnv>=0
				// make another convoi the current selected one
				depot_frame_t *win = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
				if(  win  ) {
					if (local_execution) {
						// change state of depot window only for local execution
						win->activate_convoi( !convois.empty() ? convois.at( min((uint32)icnv, convois.get_count()-1) ) : convoihandle_t() );
					}
					else {
						win->update_data();
					}
				}
			}

			return true;
		}
	}
	else {
		if (local_execution) {
			create_win( new news_img("Noch kein Fahrzeug\nmit Fahrplan\nvorhanden\n"), w_time_delete, magic_none);
		}

		if(!cnv.is_bound()) {
			dbg->warning("depot_t::start_convoi()","No convoi to start!");
		}
		if(cnv.is_bound() && cnv->get_schedule() == NULL) {
			dbg->warning("depot_t::start_convoi()","No schedule for convoi.");
		}
		if (cnv.is_bound() && cnv->get_schedule() != NULL && !cnv->get_schedule()->ist_abgeschlossen()) {
			dbg->warning("depot_t::start_convoi()","Schedule is incomplete/not finished");
		}
	}
	return false;
}



// attention! this will not be used for railway depots! They will be loaded by hand ...
void depot_t::rdwr(loadsave_t *file)
{
	gebaeude_t::rdwr(file);

	rdwr_vehikel(vehicles, file);
	if (file->get_version() < 81033) {
		// waggons are stored extra, just add them to vehicles
		assert(file->is_loading());
		rdwr_vehikel(vehicles, file);
	}
}



void depot_t::rdwr_vehikel(slist_tpl<vehikel_t *> &list, loadsave_t *file)
{
// read/write vehicles in the depot, which are not part of a convoi.

	sint32 count;

	if(file->is_saving()) {
		count = list.get_count();
		DBG_MESSAGE("depot_t::vehikel_laden()","saving %d vehicles",count);
	}
	file->rdwr_long(count);

	if(file->is_loading()) {

		// no house definition for this => use a normal hut ...
		if(  this->get_tile()==NULL  ) {
			dbg->error( "depot_t::rdwr()", "tile for depot not found!" );
			set_tile( (*hausbauer_t::get_citybuilding_list( gebaeude_t::wohnung ))[0]->get_tile(0) );
		}

		DBG_MESSAGE("depot_t::vehikel_laden()","loading %d vehicles",count);
		for(int i=0; i<count; i++) {
			ding_t::typ typ = (ding_t::typ)file->rd_obj_id();

			vehikel_t *v = NULL;
			const bool first = false;
			const bool last = false;

			switch( typ ) {
				case old_automobil:
				case automobil: v = new automobil_t(welt, file, first, last);    break;
				case old_waggon:
				case waggon:    v = new waggon_t(welt, file, first, last);       break;
				case old_schiff:
				case schiff:    v = new schiff_t(welt, file, first, last);       break;
				case old_aircraft:
				case aircraft: v = new aircraft_t(welt,file, first, last);  break;
				case old_monorailwaggon:
				case monorailwaggon: v = new monorail_waggon_t(welt,file, first, last);  break;
				case maglevwaggon:   v = new maglev_waggon_t(welt,file, first, last);  break;
				case narrowgaugewaggon: v = new narrowgauge_waggon_t(welt,file, first, last);  break;
				default:
					dbg->fatal("depot_t::vehikel_laden()","invalid vehicle type $%X", typ);
			}
			const vehikel_besch_t *besch = v->get_besch();
			if(besch) {
				DBG_MESSAGE("depot_t::vehikel_laden()","loaded %s", besch->get_name());
				list.insert( v );
				// BG, 06.06.2009: fixed maintenance for vehicles in the depot, which are not part of a convoi
				spieler_t::add_maintenance(get_besitzer(), besch->get_fixed_maintenance(get_welt()) / 20, spieler_t::MAINT_VEHICLE);
			}
			else {
				dbg->error("depot_t::vehikel_laden()","vehicle has no besch => ignored");
			}
		}
	}
	else {
		slist_iterator_tpl<vehikel_t *> l_iter ( list);
		while(l_iter.next()) {
			file->wr_obj_id( l_iter.get_current()->get_typ() );
			l_iter.get_current()->rdwr_from_convoi( file );
		}
	}
}

/**
 * @return NULL wenn OK, ansonsten eine Fehlermeldung
 * @author Hj. Malthaner
 */
const char * depot_t::ist_entfernbar(const spieler_t *sp)
{
	if(sp!=get_besitzer()  &&  sp!=welt->get_spieler(1)) {
		return "Das Feld gehoert\neinem anderen Spieler\n";
	}
	if (!vehicles.empty()) {
		return "There are still vehicles\nstored in this depot!\n";
	}
	slist_iterator_tpl<convoihandle_t> iter(convois);

	while(iter.next()) {
		if(iter.get_current()->get_vehikel_anzahl() > 0) {
			return "There are still vehicles\nstored in this depot!\n";
		}
	}
	return NULL;
}

// returns the indest of the old/newest vehicle in a list
//@author: isidoro
vehikel_t* depot_t::find_oldest_newest(const vehikel_besch_t* besch, bool old, vector_tpl<vehikel_t*> *avoid)
{
	vehikel_t* found_veh = NULL;
	slist_iterator_tpl<vehikel_t*> iter(get_vehicle_list());
	while (iter.next()) 
	{
		vehikel_t* veh = iter.get_current();
		if (veh != NULL && veh->get_besch() == besch) 
		{
			// joy of XOR, finally a line where I could use it!
			if (avoid == NULL || (!avoid->is_contained(veh) && (found_veh == NULL ||
					old ^ (found_veh->get_insta_zeit() > veh->get_insta_zeit())))) // Used when replacing to avoid specifying the same vehicle twice
			{
				found_veh = veh;
			}
		}
	}
	return found_veh;
}


slist_tpl<vehikel_besch_t*>* depot_t::get_vehicle_type()
{
	return vehikelbauer_t::get_info(get_wegtyp());
}


vehikel_t* depot_t::get_oldest_vehicle(const vehikel_besch_t* besch)
{
	vehikel_t* oldest_veh = NULL;
	slist_iterator_tpl<vehikel_t*> iter(get_vehicle_list());
	while (iter.next()) {
		vehikel_t* veh = iter.get_current();
		if (veh->get_besch() == besch) {
			if (oldest_veh == NULL ||
					oldest_veh->get_insta_zeit() > veh->get_insta_zeit()) {
				oldest_veh = veh;
			}
		}
	}
	return oldest_veh;
}


/**
 * sets/gets the line that was selected the last time in the depot-dialog
 */
void depot_t::set_selected_line(const linehandle_t sel_line)
{
	selected_line = sel_line;
	depot_frame_t *win = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
	if(  win  ) {
		win->layout(NULL);
		win->update_data();
	}
}


linehandle_t depot_t::get_selected_line()
{
	return selected_line;
}


sint32 depot_t::calc_restwert(const vehikel_besch_t *veh_type)
{
	sint32 wert = 0;

	slist_iterator_tpl<vehikel_t *> iter(get_vehicle_list());
	while(iter.next()) {
		if(iter.get_current()->get_besch() == veh_type) {
			wert += iter.get_current()->calc_restwert();
		}
	}
	return wert;
}


// true if already stored here
bool depot_t::is_contained(const vehikel_besch_t *info)
{
	if(vehicle_count()>0) {
		slist_iterator_tpl<vehikel_t *> iter(get_vehicle_list());
		while(iter.next()) {
			if(iter.get_current()->get_besch()==info) {
				return true;
			}
		}
	}
	return false;
}

/**
 * The player must pay monthly fixed maintenance costs for the vehicles in the depot.
 * This method is called by the world (karte_t) once per month.
 * @author Bernd Gabriel
 * @date 27.06.2009
 */
void depot_t::neuer_monat()
{
	uint32 fixed_maintenance_costs = 0;
	if (vehicle_count() > 0) 
	{
		karte_t *world = get_welt();
		slist_iterator_tpl<vehikel_t *> vehicle_iter(vehicles);
		while (vehicle_iter.next()) {
			fixed_maintenance_costs += vehicle_iter.get_current()->get_besch()->get_fixed_maintenance(world);
		}
	}
	if (fixed_maintenance_costs)
	{
		//spieler->add_maintenance(fixed_maintenance_costs, spieler_t::MAINT_VEHICLE);
		get_besitzer()->buche(-(sint32)welt->calc_adjusted_monthly_figure(fixed_maintenance_costs), COST_VEHICLE_RUN);
	}
	// since vehicles may have become obsolete
	update_all_win();
}

void depot_t::update_win()
{
	depot_frame_t *depot_frame = dynamic_cast<depot_frame_t *>(win_get_magic( (long)this ));
	if(depot_frame) {
		depot_frame->build_vehicle_lists();
	}
}

void depot_t::update_all_win()
{
	slist_iterator_tpl<depot_t *> iter(all_depots);
	while(iter.next()) {
		iter.access_current()->update_win();
	}
}
