/*
 * Dialog window for defining a schedule
 *
 * Hj. Malthaner
 *
 * Juli 2000
 */

#include "../simconvoi.h"
#include "../simline.h"
#include "../simlinemgmt.h"
#include "../simcolor.h"
#include "../simwin.h"
#include "../simhalt.h"
#include "../simskin.h"
#include "../simworld.h"
#include "../simmenu.h"
#include "../simgraph.h"
#include "../simtools.h"

#include "../utils/simstring.h"
#include "../utils/cbuffer_t.h"

#include "../boden/grund.h"

#include "../dataobj/umgebung.h"
#include "../dataobj/fahrplan.h"
#include "../dataobj/loadsave.h"
#include "../dataobj/translator.h"

#include "../player/simplay.h"
#include "../vehicle/simvehikel.h"

#include "../tpl/vector_tpl.h"

#include "fahrplan_gui.h"
#include "line_item.h"
#include "components/list_button.h"
#include "components/gui_button.h"
#include "karte.h"

char fahrplan_gui_t::no_line[128];	// contains the current translation of "<no line>"
karte_t *fahrplan_gui_t::welt = NULL;


/**
 * Fills buf with description of schedule's i'th entry.
 *
 * @author Hj. Malthaner
 */
void fahrplan_gui_t::gimme_stop_name(cbuffer_t & buf, karte_t *welt, const spieler_t *sp, const linieneintrag_t &entry )
{
	halthandle_t halt = haltestelle_t::get_halt(welt, entry.pos, sp);
	if(halt.is_bound()) {
		if (entry.ladegrad != 0) {
			buf.printf("%d%% %s (%s)", entry.ladegrad, halt->get_name(), entry.pos.get_str() );
		}
		else {
			buf.printf("%s (%s)",
				halt->get_name(),
				entry.pos.get_str() );
		}
	}
	else {
		const grund_t* gr = welt->lookup(entry.pos);
		if(gr==NULL) {
			buf.printf("%s (%s)", translator::translate("Invalid coordinate"), entry.pos.get_str() );
		}
		else if(gr->get_depot() != NULL) {
			buf.printf("%s (%s)", translator::translate("Depot"), entry.pos.get_str() );
		}
		else {
			buf.printf("%s (%s)", translator::translate("Wegpunkt"), entry.pos.get_str() );
		}
	}
}


/**
 * Fills buf with description of schedule's i'th entry.
 * short version, without loading level and position ...
 * @author Hj. Malthaner
 */
void fahrplan_gui_t::gimme_short_stop_name(cbuffer_t &buf, karte_t *welt, const spieler_t *sp, const schedule_t *fpl, int i, int max_chars)
{
	if(i<0  ||  fpl==NULL  ||  i>=fpl->get_count()) {
		dbg->warning("void fahrplan_gui_t::gimme_short_stop_name()","tried to receive unused entry %i in schedule %p.",i,fpl);
		return;
	}
	const linieneintrag_t& entry = fpl->eintrag[i];
	const char *p;
	halthandle_t halt = haltestelle_t::get_halt(welt, entry.pos, sp);
	if(halt.is_bound()) {
		p = halt->get_name();
	}
	else {
		const grund_t* gr = welt->lookup(entry.pos);
		if(gr==NULL) {
			p = translator::translate("Invalid coordinate");
		}
		else if(gr->get_depot() != NULL) {
			p = translator::translate("Depot");
		}
		else {
			p = translator::translate("Wegpunkt");
		}
	}
	// finally append
	if(strlen(p)>(unsigned)max_chars) {
		buf.printf("%.*s...", max_chars - 3, p);
	}
	else {
		buf.append(p);
	}
}



karte_t *fahrplan_gui_stats_t::welt = NULL;
cbuffer_t fahrplan_gui_stats_t::buf;

void fahrplan_gui_stats_t::zeichnen(koord offset)
{
	if(fpl) {
		sint16 width = get_groesse().x-16;

		for (int i = 0; i < fpl->get_count(); i++) {

			buf.clear();
			buf.printf( "%i) ", i+1 );
			fahrplan_gui_t::gimme_stop_name( buf, welt, sp, fpl->eintrag[i] );
			sint16 w = display_proportional_clip(offset.x + 4 + 10, offset.y + i * (LINESPACE + 1), buf, ALIGN_LEFT, COL_BLACK, true);
			if(  w>width  ) {
				width = w;
			}

			// the goto button (right arrow)
			display_color_img( i!=fpl->get_aktuell() ? button_t::arrow_right_normal : button_t::arrow_right_pushed,
				offset.x + 2, offset.y + i * (LINESPACE + 1), 0, false, true);
		}
		set_groesse( koord(width+16, fpl->get_count() * (LINESPACE + 1) ) );
	}
}



fahrplan_gui_t::~fahrplan_gui_t()
{
	if(  sp  ) {
		update_werkzeug( false );
		// hide schedule on minimap (may not current, but for safe)
		reliefkarte_t::get_karte()->set_current_fpl(NULL, 0); // (*fpl,player_nr)
	}
	delete fpl;

}



fahrplan_gui_t::fahrplan_gui_t(schedule_t* fpl_, spieler_t* sp_, convoihandle_t cnv_) :
	gui_frame_t("Fahrplan", sp_),
	lb_line("Serves Line:"),
	lb_wait("month wait time"),
	lb_waitlevel(NULL, COL_WHITE, gui_label_t::right),
	lb_waitlevel_as_clock(NULL, COL_BLACK, gui_label_t::centered),
	lb_load("Full load"),
	lb_spacing("Spacing cnv/month, shift"),
	lb_spacing_as_clock(NULL, COL_BLACK, gui_label_t::centered),
	lb_spacing_shift_as_clock(NULL, COL_BLACK, gui_label_t::centered),
	stats(sp_->get_welt(),sp_),
	scrolly(&stats),
	old_fpl(fpl_),
	sp(sp_),
	cnv(cnv_)
{
	welt = sp->get_welt();
	old_fpl->eingabe_beginnen();
	fpl = old_fpl->copy();
	stats.set_fahrplan(fpl);
	if(!cnv.is_bound()) {
		old_line = new_line = linehandle_t();
		show_line_selector(false);
	}
	else {
		old_line = new_line = cnv_->get_line();
	}
	old_line_count = 0;
	strcpy(no_line, translator::translate("<no line>"));

	sint16 ypos = 0;
	if (cnv.is_bound()) {
		// things, only relevant to convois, like creating/selecting lines
		lb_line.set_pos(koord(10, ypos+2));
		add_komponente(&lb_line);

		bt_promote_to_line.init(button_t::roundbox, "promote to line", koord( BUTTON3_X, ypos ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
		bt_promote_to_line.set_tooltip("Create a new line based on this schedule");
		bt_promote_to_line.add_listener(this);
		add_komponente(&bt_promote_to_line);

		ypos += BUTTON_HEIGHT+1;

		line_selector.set_pos(koord(2, ypos));
		line_selector.set_groesse(koord(BUTTON4_X-2, BUTTON_HEIGHT));
		line_selector.set_max_size(koord(BUTTON4_X-2, 13*LINESPACE+TITLEBAR_HEIGHT-1));
		line_selector.set_highlight_color(sp->get_player_color1() + 1);
		line_selector.clear_elements();

		sp->simlinemgmt.sort_lines();
		init_line_selector();
		line_selector.add_listener(this);
		add_komponente(&line_selector);

		ypos += BUTTON_HEIGHT+3;
	}

	// loading level and return tickets
	lb_load.set_pos( koord( 10, ypos+2 ) );
	add_komponente(&lb_load);

	numimp_load.set_pos( koord( BUTTON_WIDTH*2-65, ypos+2 ) );
	numimp_load.set_groesse( koord( 60, BUTTON_HEIGHT ) );
	numimp_load.set_value( fpl->get_current_eintrag().ladegrad );
	numimp_load.set_limits( 0, 100 );
	numimp_load.set_increment_mode( gui_numberinput_t::PROGRESS );
	numimp_load.add_listener(this);
	add_komponente(&numimp_load);

	bt_bidirectional.init(button_t::square_automatic, "Alternate directions", koord( BUTTON_WIDTH*2, ypos ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
	bt_bidirectional.set_tooltip("When adding convoys to the line, every second convoy will follow it in the reverse direction.");
	bt_bidirectional.pressed = fpl->is_bidirectional();
	bt_bidirectional.add_listener(this);
	add_komponente(&bt_bidirectional);

	ypos += BUTTON_HEIGHT;

	// waiting in parts per month
	lb_wait.set_pos( koord( 10, ypos+2 ) );
	add_komponente(&lb_wait);

	bt_wait_prev.set_pos( koord( BUTTON_WIDTH*2-65, ypos+2 ) );
	bt_wait_prev.set_typ(button_t::arrowleft);
	bt_wait_prev.add_listener(this);
	add_komponente(&bt_wait_prev);

	if(  fpl->get_current_eintrag().waiting_time_shift==0  ) {
		strcpy( str_parts_month, translator::translate("off") );
		strcpy( str_parts_month_as_clock, translator::translate("off") );

	}
	else {
		sprintf( str_parts_month, "1/%d",  1<<(16-fpl->get_current_eintrag().waiting_time_shift) );
		sint32 ticks_waiting = welt->ticks_per_world_month >> (16-fpl->get_current_eintrag().waiting_time_shift);
		win_sprintf_ticks(str_parts_month_as_clock, sizeof(str_parts_month_as_clock), ticks_waiting + 1);
	}

	lb_waitlevel.set_text_pointer( str_parts_month );
	lb_waitlevel.set_pos( koord( BUTTON_WIDTH*2-20, ypos+2 ) );
	add_komponente(&lb_waitlevel);

	bt_wait_next.set_pos( koord( BUTTON_WIDTH*2-15, ypos+2 ) );
	bt_wait_next.set_typ(button_t::arrowright);
	bt_wait_next.add_listener(this);
	add_komponente(&bt_wait_next);

	bt_mirror.init(button_t::square_automatic, "return ticket", koord( BUTTON_WIDTH*2, ypos ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
	bt_mirror.set_tooltip("Vehicles make a round trip between the schedule endpoints, visiting all stops in reverse after reaching the end.");
	bt_mirror.pressed = fpl->is_mirrored();
	bt_mirror.add_listener(this);
	add_komponente(&bt_mirror);

	ypos += BUTTON_HEIGHT;

	lb_waitlevel_as_clock.set_text_pointer (str_parts_month_as_clock);
	lb_waitlevel_as_clock.set_pos( koord( BUTTON_WIDTH*2-35, ypos+2 ) );
	add_komponente(&lb_waitlevel_as_clock);

	ypos += BUTTON_HEIGHT;

	// Spacing
	if ( !cnv.is_bound() ) {
		lb_spacing.set_pos( koord( 10, ypos+2 ) );
		add_komponente(&lb_spacing);
		//numimp_spacing.set_pos( koord( BUTTON_WIDTH*2-65, ypos+2 ) );
		numimp_spacing.set_pos( koord( BUTTON_WIDTH*2, ypos+2 ) );
		numimp_spacing.set_groesse( koord( 60, BUTTON_HEIGHT ) );
		numimp_spacing.set_value( fpl->get_spacing() );
		numimp_spacing.set_limits( 0, 999 );
		numimp_spacing.set_increment_mode( 1 );
		numimp_spacing.add_listener(this);
		add_komponente(&numimp_spacing);

		// Spacing shift
		int spacing_shift_mode = welt->get_settings().get_spacing_shift_mode();
		if ( spacing_shift_mode > settings_t::SPACING_SHIFT_DISABLED) {
			numimp_spacing_shift.set_pos( koord( BUTTON_WIDTH*2 + 65, ypos+2 ) );
			numimp_spacing_shift.set_groesse( koord( 60, BUTTON_HEIGHT ) );
			numimp_spacing_shift.set_value( fpl->get_current_eintrag().spacing_shift  );
			numimp_spacing_shift.set_limits( 0,welt->get_settings().get_spacing_shift_divisor() );
			numimp_spacing_shift.set_increment_mode( 1 );
			numimp_spacing_shift.add_listener(this);
			add_komponente(&numimp_spacing_shift);
		}

		ypos += BUTTON_HEIGHT;

		if (spacing_shift_mode > settings_t::SPACING_SHIFT_PER_LINE) {
			//Same spacing button
			bt_same_spacing_shift.init(button_t::square_automatic, "Use same shift for all stops.", koord( 10 , ypos+2 ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
			bt_same_spacing_shift.set_tooltip("Use one spacing shift value for all stops in schedule.");
			bt_same_spacing_shift.pressed = fpl->is_same_spacing_shift();
			bt_same_spacing_shift.add_listener(this);
			add_komponente(&bt_same_spacing_shift);
		}

		lb_spacing_as_clock.set_pos(koord( BUTTON_WIDTH*2 + 30 , ypos+2 ) );
		lb_spacing_as_clock.set_text_pointer(str_spacing_as_clock);
		add_komponente(&lb_spacing_as_clock);

		if (spacing_shift_mode > settings_t::SPACING_SHIFT_PER_LINE) {
			lb_spacing_shift_as_clock.set_pos(koord( BUTTON_WIDTH*2 + 95, ypos+2 ) );
			lb_spacing_shift_as_clock.set_text_pointer(str_spacing_shift_as_clock);
			add_komponente(&lb_spacing_shift_as_clock);
		}

		ypos += BUTTON_HEIGHT;
	}

	bt_add.init(button_t::roundbox_state, "Add Stop", koord(BUTTON1_X, ypos ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
	bt_add.set_tooltip("Appends stops at the end of the schedule");
	bt_add.add_listener(this);
	bt_add.pressed = true;
	add_komponente(&bt_add);

	bt_insert.init(button_t::roundbox_state, "Ins Stop", koord(BUTTON2_X, ypos ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
	bt_insert.set_tooltip("Insert stop before the current stop");
	bt_insert.add_listener(this);
	bt_insert.pressed = false;
	add_komponente(&bt_insert);

	bt_remove.init(button_t::roundbox_state, "Del Stop", koord(BUTTON3_X, ypos ), koord(BUTTON_WIDTH,BUTTON_HEIGHT) );
	bt_remove.set_tooltip("Delete the current stop");
	bt_remove.add_listener(this);
	bt_remove.pressed = false;
	add_komponente(&bt_remove);

	ypos += BUTTON_HEIGHT+2;

	scrolly.set_pos( koord( 0, ypos ) );
	scrolly.set_show_scroll_x(true);
	scrolly.set_scroll_amount_y(LINESPACE+1);
	add_komponente(&scrolly);

	mode = adding;
	update_selection();

	set_fenstergroesse( koord(BUTTON4_X + 35, ypos+BUTTON_HEIGHT+min(15,fpl->get_count())*(LINESPACE + 1)+TITLEBAR_HEIGHT) );
	set_min_windowsize( koord(BUTTON4_X + 35, ypos+BUTTON_HEIGHT+3*(LINESPACE + 1)+TITLEBAR_HEIGHT) );

	set_resizemode(diagonal_resize);
	resize( koord(0,0) );

	// set this schedule as current to show on minimap if possible
	reliefkarte_t::get_karte()->set_current_fpl(fpl, sp->get_player_nr()); // (*fpl,player_nr)
}


void fahrplan_gui_t::update_werkzeug(bool set)
{
	if(!set  ||  mode==removing  ||  mode==undefined_mode) {
		// reset tools, if still selected ...
		if(welt->get_werkzeug(sp->get_player_nr())==werkzeug_t::general_tool[WKZ_FAHRPLAN_ADD]) {
			if(werkzeug_t::general_tool[WKZ_FAHRPLAN_ADD]->get_default_param()==(const char *)fpl) {
				welt->set_werkzeug( werkzeug_t::general_tool[WKZ_ABFRAGE], sp );
			}
		}
		else if(welt->get_werkzeug(sp->get_player_nr())==werkzeug_t::general_tool[WKZ_FAHRPLAN_INS]) {
			if(werkzeug_t::general_tool[WKZ_FAHRPLAN_INS]->get_default_param()==(const char *)fpl) {
				welt->set_werkzeug( werkzeug_t::general_tool[WKZ_ABFRAGE], sp );
			}
		}
	}
	else {
		//  .. or set them again
		if(mode==adding) {
			werkzeug_t::general_tool[WKZ_FAHRPLAN_ADD]->set_default_param((const char *)fpl);
			sp->get_welt()->set_werkzeug( werkzeug_t::general_tool[WKZ_FAHRPLAN_ADD], sp );
		}
		else if(mode==inserting) {
			werkzeug_t::general_tool[WKZ_FAHRPLAN_INS]->set_default_param((const char *)fpl);
			sp->get_welt()->set_werkzeug( werkzeug_t::general_tool[WKZ_FAHRPLAN_INS], sp );
		}
	}
}


void fahrplan_gui_t::update_selection()
{
	// update load
	lb_load.set_color( COL_GREY3 );
	lb_wait.set_color( COL_GREY3 );
	lb_spacing.set_color( COL_GREY3 );
	lb_spacing_as_clock.set_color( COL_GREY3 );
	sprintf(str_spacing_as_clock, "%s", translator::translate("off") );
	lb_spacing_shift.set_color( COL_GREY3 );
	lb_spacing_shift_as_clock.set_color( COL_GREY3 );
	sprintf(str_spacing_shift_as_clock, "%s", translator::translate("off") );

	if (!fpl->empty()) {
		fpl->set_aktuell( min(fpl->get_count()-1,fpl->get_aktuell()) );
		const uint8 aktuell = fpl->get_aktuell();
		if(  haltestelle_t::get_halt(sp->get_welt(), fpl->eintrag[aktuell].pos, sp).is_bound()  ) {
			lb_load.set_color( COL_BLACK );
			numimp_load.set_value( fpl->eintrag[aktuell].ladegrad );
			numimp_spacing_shift.set_value(fpl->eintrag[aktuell].spacing_shift);
			if(  fpl->eintrag[aktuell].ladegrad>0  ) {
				lb_wait.set_color( COL_BLACK );
				lb_spacing.set_color( COL_BLACK );
				if (fpl->get_spacing() ) {
					lb_spacing_shift.set_color( COL_BLACK );
					lb_spacing_as_clock.set_color( COL_BLACK );
					lb_spacing_shift_as_clock.set_color( COL_BLACK );

					win_sprintf_ticks(str_spacing_as_clock, sizeof(str_spacing_as_clock), welt->ticks_per_world_month/fpl->get_spacing());
					win_sprintf_ticks(str_spacing_shift_as_clock, sizeof(str_spacing_as_clock),
							fpl->eintrag[aktuell].spacing_shift * welt->ticks_per_world_month/welt->get_settings().get_spacing_shift_divisor()+1
							);
				}
			}
			if(  fpl->eintrag[aktuell].ladegrad>0  &&  fpl->eintrag[aktuell].waiting_time_shift>0  ) {
				sprintf( str_parts_month, "1/%d",  1<<(16-fpl->eintrag[aktuell].waiting_time_shift) );
				sint32 ticks_waiting = welt->ticks_per_world_month >> (16-fpl->get_current_eintrag().waiting_time_shift);
				win_sprintf_ticks(str_parts_month_as_clock, sizeof(str_parts_month_as_clock), ticks_waiting + 1);
			}
			else {
				strcpy( str_parts_month, translator::translate("off") );
				strcpy( str_parts_month_as_clock, translator::translate("off") );
			}
		}
		else {
			strcpy( str_ladegrad, "0%" );
			strcpy( str_parts_month, translator::translate("off") );
		}
	}
}

/**
 * Mausklicks werden hiermit an die GUI-Komponenten
 * gemeldet
 */
bool fahrplan_gui_t::infowin_event(const event_t *ev)
{
	if ( (ev)->ev_class == EVENT_CLICK  &&  !((ev)->ev_code==MOUSE_WHEELUP  ||  (ev)->ev_code==MOUSE_WHEELDOWN)  &&  !line_selector.getroffen(ev->cx, ev->cy-16))  {//  &&  !scrolly.getroffen(ev->cx, ev->cy+16)) {

		// close combo box; we must do it ourselves, since the box does not receive outside events ...
		line_selector.close_box();

		if(ev->my>=scrolly.get_pos().y+16) {
			// we are now in the multiline region ...
			const int line = ( ev->my - scrolly.get_pos().y + scrolly.get_scroll_y() - 16)/(LINESPACE+1);

			if(line >= 0 && line < fpl->get_count()) {
				if(IS_RIGHTCLICK(ev)  ||  ev->mx<16) {
					// just center on it
					sp->get_welt()->change_world_position( fpl->eintrag[line].pos );
				}
				else if(ev->mx<scrolly.get_groesse().x-11) {
					fpl->set_aktuell( line );
					if(mode == removing) {
						fpl->remove();
						action_triggered( &bt_add, value_t() );
					}
					update_selection();
				}
			}
		}
	}
	else if(ev->ev_class == INFOWIN  &&  ev->ev_code == WIN_CLOSE  &&  fpl!=NULL  ) {

		update_werkzeug( false );
		fpl->cleanup();
		fpl->eingabe_abschliessen();
		// now apply the changes
		if(cnv.is_bound()) {
			// do not send changes if the convoi is about to be deleted
			if(  cnv->get_state() != convoi_t::SELF_DESTRUCT  ) {
				// if a line is selected
				if(  new_line.is_bound()  ) {
					// if the selected line is different to the convoi's line, apply it
					if(new_line!=cnv->get_line()) {
						char id[16];
						sprintf( id, "%i,%i", new_line.get_id(), fpl->get_aktuell() );
						cnv->call_convoi_tool( 'l', id );
					}
					else {
						cbuffer_t buf;
						fpl->sprintf_schedule( buf );
						cnv->call_convoi_tool( 'g', buf );
					}
				}
				else {
					cbuffer_t buf;
					fpl->sprintf_schedule( buf );
					cnv->call_convoi_tool( 'g', buf );
				}
			}
		}
	}
	else if(ev->ev_class == INFOWIN  &&  (ev->ev_code == WIN_TOP  ||  ev->ev_code == WIN_OPEN)  &&  fpl!=NULL  ) {
		// just to be sure, renew the tools ...
		update_werkzeug( true );
	}

	return gui_frame_t::infowin_event(ev);
}


bool fahrplan_gui_t::action_triggered( gui_action_creator_t *komp, value_t p)
{
DBG_MESSAGE("fahrplan_gui_t::action_triggered()","komp=%p combo=%p",komp,&line_selector);

	if(komp == &bt_add) {
		mode = adding;
		bt_add.pressed = true;
		bt_insert.pressed = false;
		bt_remove.pressed = false;
		update_werkzeug( true );

	} else if(komp == &bt_insert) {
		mode = inserting;
		bt_add.pressed = false;
		bt_insert.pressed = true;
		bt_remove.pressed = false;
		update_werkzeug( true );

	} else if(komp == &bt_remove) {
		mode = removing;
		bt_add.pressed = false;
		bt_insert.pressed = false;
		bt_remove.pressed = true;
		update_werkzeug( false );

	} else if(komp == &numimp_load) {
		if (!fpl->empty()) {
			fpl->eintrag[fpl->get_aktuell()].ladegrad = p.i;
			update_selection();
		}
	} else if(komp == &bt_wait_prev) {
		if (!fpl->empty()) {
			sint8& wait = fpl->eintrag[fpl->get_aktuell()].waiting_time_shift;
			if(wait>7) {
				wait --;
			}
			else if(  wait>0  ) {
				wait = 0;
			}
			else {
				wait = 16;
			}
			update_selection();
		}
	} else if(komp == &bt_wait_next) {
		if (!fpl->empty()) {
			sint8& wait = fpl->eintrag[fpl->get_aktuell()].waiting_time_shift;
			if(wait==0) {
				wait = 7;
			}
			else if(wait<16) {
				wait ++;
			}
			else {
				wait = 0;
			}
			update_selection();
		}
	/*} else if (komp == &bt_return) {
		fpl->add_return_way();*/
	} else if (komp == &numimp_spacing) {
		fpl->set_spacing(p.i);
		update_selection();
	} else if(komp == &numimp_spacing_shift) {
		if (!fpl->empty()) {
			if ( fpl->is_same_spacing_shift() ) {
			    for(  uint8 i=0;  i<fpl->eintrag.get_count();  i++  ) {
					fpl->eintrag[i].spacing_shift = p.i;
				}
			} else {
				fpl->eintrag[fpl->get_aktuell()].spacing_shift = p.i;
			}
			update_selection();
		}
	} else if (komp == &bt_mirror) {
		fpl->set_mirrored(bt_mirror.pressed);
	} else if (komp == &bt_bidirectional) {
		fpl->set_bidirectional(bt_bidirectional.pressed);
	} else if (komp == &bt_same_spacing_shift) {
		fpl->set_same_spacing_shift(bt_same_spacing_shift.pressed);
		if ( fpl->is_same_spacing_shift() ) {
		    for(  uint8 i=0;  i<fpl->eintrag.get_count();  i++  ) {
				fpl->eintrag[i].spacing_shift = fpl->eintrag[fpl->get_aktuell()].spacing_shift;
			}
		}
	} else if (komp == &line_selector) {
		int selection = p.i;
//DBG_MESSAGE("fahrplan_gui_t::action_triggered()","line selection=%i",selection);
		if(  (uint32)(selection-1)<(uint32)line_selector.count_elements()  ) {
			new_line = lines[selection - 1];
			fpl->copy_from( new_line->get_schedule() );
			fpl->eingabe_beginnen();
		}
		else {
			// remove line
			new_line = linehandle_t();
			line_selector.set_selection( 0 );
		}
	} else if (komp == &bt_promote_to_line) {
		// update line schedule via tool!
		werkzeug_t *w = create_tool( WKZ_LINE_TOOL | SIMPLE_TOOL );
		cbuffer_t buf;
		buf.printf( "c,0,%i,%ld,", (int)fpl->get_type(), (long)old_fpl );
		fpl->sprintf_schedule( buf );
		w->set_default_param(buf);
		sp->get_welt()->set_werkzeug( w, sp );
		// since init always returns false, it is save to delete immediately
		delete w;
	}
	// recheck lines
	if (cnv.is_bound()) {
		// unequal to line => remove from line ...
		if(  new_line.is_bound()  &&  !fpl->matches(sp->get_welt(),new_line->get_schedule())  ) {
			new_line = linehandle_t();
			line_selector.set_selection(0);
		}
		// only assing old line, when new_line is not equal
		if(  !new_line.is_bound()  &&  old_line.is_bound()  &&   fpl->matches(sp->get_welt(),old_line->get_schedule())  ) {
			new_line = old_line;
			init_line_selector();
		}
	}
	return true;
}


void fahrplan_gui_t::init_line_selector()
{
	line_selector.clear_elements();
	line_selector.append_element( new gui_scrolled_list_t::const_text_scrollitem_t( no_line, COL_BLACK ) );
	int selection = -1;
	sp->simlinemgmt.sort_lines();	// to take care of renaming ...
	sp->simlinemgmt.get_lines(fpl->get_type(), &lines);

	// keep assignment with identical schedules
	if(  new_line.is_bound()  &&  !fpl->matches( sp->get_welt(), new_line->get_schedule() )  ) {
		if(  old_line.is_bound()  &&  fpl->matches( sp->get_welt(), old_line->get_schedule() )  ) {
			new_line = old_line;
		}
		else {
			new_line = linehandle_t();
		}
	}

	for (vector_tpl<linehandle_t>::const_iterator i = lines.begin(), end = lines.end(); i != end; i++) {
		linehandle_t line = *i;
		line_selector.append_element( new line_scrollitem_t(line) );
		if(  !new_line.is_bound()  ) {
			if(  fpl->matches( sp->get_welt(), line->get_schedule() )  ) {
				selection = line_selector.count_elements() - 1;
				new_line = line;
			}
		}
		else if(  line==new_line  ) {
			selection = line_selector.count_elements() - 1;
		}
	}
	line_selector.set_selection( selection );
	old_line_count = sp->simlinemgmt.get_line_count();
	last_schedule_count = fpl->get_count();
}



void fahrplan_gui_t::zeichnen(koord pos, koord gr)
{
	if(  sp->simlinemgmt.get_line_count()!=old_line_count  ||  last_schedule_count!=fpl->get_count()  ) {
		// lines added or deleted
		init_line_selector();
		last_schedule_count = fpl->get_count();
	}

	// after loading in network games, the schedule might still being updated
	if(  cnv.is_bound()  &&  cnv->get_state()==convoi_t::FAHRPLANEINGABE  &&  fpl->ist_abgeschlossen()  ) {
		assert( convoi_t::FAHRPLANEINGABE==1 ); // convoi_t::FAHRPLANEINGABE is 1
		fpl->eingabe_beginnen();
		cnv->call_convoi_tool( 's', "1" );
	}

	gui_frame_t::zeichnen(pos,gr);
}



/**
 * Set window size and adjust component sizes and/or positions accordingly
 * @author Hj. Malthaner
 * @date   16-Oct-2003
 */
void fahrplan_gui_t::set_fenstergroesse(koord groesse)
{
	gui_frame_t::set_fenstergroesse(groesse);

	groesse = get_fenstergroesse()-koord(0,16+1);
	scrolly.set_groesse(groesse-koord(0,scrolly.get_pos().y));

	line_selector.set_max_size(koord(BUTTON4_X-2, groesse.y-line_selector.get_pos().y -16-1));
}


void fahrplan_gui_t::map_rotate90( sint16 y_size)
{
	fpl->rotate90(y_size);
}


fahrplan_gui_t::fahrplan_gui_t(karte_t *welt):
	gui_frame_t("Fahrplan", NULL),
	lb_line("Serves Line:"),
	lb_wait("month wait time"),
	lb_waitlevel(NULL, COL_WHITE, gui_label_t::right),
	lb_load("Full load"),
	stats(welt,NULL),
	scrolly(&stats),
	fpl(NULL),
	old_fpl(NULL),
	sp(NULL),
	cnv()
{
	// just a dummy
	this->welt = welt;
}


void fahrplan_gui_t::rdwr(loadsave_t *file)
{
	// this handles only schedules of bound convois
	// lines are handled by line_management_gui_t
	koord3d cnv_pos;
	char cnv_name[256];
	uint8 player_nr;
	koord gr = get_fenstergroesse();
	if(  file->is_saving()  ) {
		tstrncpy( cnv_name, cnv->get_name(), sizeof(cnv_name) );
		player_nr = sp->get_player_nr();
		cnv_pos = cnv->get_pos();
	}
	else {
		// dummy types
		old_fpl = new autofahrplan_t();
		fpl = new autofahrplan_t();
	}
	gr.rdwr( file );
	file->rdwr_byte( player_nr );
	file->rdwr_str( cnv_name, sizeof(cnv_name) );
	cnv_pos.rdwr( file );
	old_fpl->rdwr(file);
	fpl->rdwr(file);
	if(  file->is_loading()  ) {
		// find convoi by name and position (since there could be two convois with the same name)
		if(  grund_t *gr = welt->lookup(cnv_pos)  ) {
			for(  uint8 i=0;  i<gr->get_top();  i++  ) {
				if(  gr->obj_bei(i)->is_moving()  ) {
					vehikel_t const* const v = ding_cast<vehikel_t>(gr->obj_bei(i));
					if(  v  &&  v->get_besitzer()->get_player_nr()==player_nr  &&  v->get_convoi()  &&  strcmp(v->get_convoi()->get_name(),cnv_name)==0  &&  old_fpl->matches( welt, v->get_convoi()->get_schedule() )  ) {
						cnv = v->get_convoi()->self;
						break;
					}
				}
			}
		}
		if(  !cnv.is_bound() ) {
			// not found (most likely convoi in depot ... )
			for(  vector_tpl<convoihandle_t>::const_iterator i=welt->convois_begin(); i!=welt->convois_end();  ++i  ) {
				if(  (*i)->get_besitzer()->get_player_nr()==player_nr  &&  strncmp( (*i)->get_name(), cnv_name, 256 )==0  &&  old_fpl->matches( welt, (*i)->get_schedule() )  ) {
					// valid convoi found
					cnv = *i;
					break;
				}
			}
		}
		if(  cnv.is_bound() ) {
			// now we can open the window ...
			KOORD_VAL xpos = win_get_posx( this );
			KOORD_VAL ypos = win_get_posy( this );
			fahrplan_gui_t *w = new fahrplan_gui_t( cnv->get_schedule(), cnv->get_besitzer(), cnv );
			create_win( xpos, ypos, w, w_info, (long)cnv->get_schedule() );
			w->set_fenstergroesse( gr );
			w->fpl->copy_from( fpl );
			cnv->get_schedule()->eingabe_abschliessen();
			w->fpl->eingabe_abschliessen();
		}
		else {
			dbg->error( "fahrplan_gui_t::rdwr", "Could not restore schedule window for %s", cnv_name );
		}
		sp = NULL;
		delete old_fpl;
		delete fpl;
		fpl = old_fpl = NULL;
		cnv = convoihandle_t();
		destroy_win( this );
	}
}
