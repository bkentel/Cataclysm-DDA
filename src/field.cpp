#include "rng.h"
#include "map.h"
#include "field.h"
#include "game.h"
#include "monstergenerator.h"
#include "messages.h"

#include <algorithm>

#define INBOUNDS(x, y) \
 (x >= 0 && x < SEEX * my_MAPSIZE && y >= 0 && y < SEEY * my_MAPSIZE)



namespace {
/*
Controls the master listing of all possible field effects, indexed by a field_id. Does not store active fields, just metadata.
*/
std::array<field_t, num_fields> fieldlist;

} //namespace

field_t const& get_field_def(field_id const id)
{
    return fieldlist[static_cast<size_t>(id)];
}

void game::init_fields()
{
    field_t tmp_fields[num_fields] = {
        {
            "fd_null", '%', 0, 0, field_t::decay_type::none,
            {"", "", ""},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_blood", '%', 0, 28800, field_t::decay_type::liquid,
            {_("blood splatter"), _("blood stain"), _("puddle of blood")},
            {c_red, c_red, c_red},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_bile", '%', 0, 14400, field_t::decay_type::liquid,
            {_("bile splatter"), _("bile stain"), _("puddle of bile")},
            {c_pink, c_pink, c_pink},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_gibs_flesh", '~', 0, 28800, field_t::decay_type::liquid,
            {_("scraps of flesh"), _("bloody meat chunks"), _("heap of gore")},
            {c_brown, c_ltred, c_red},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_gibs_veggy", '~', 0, 28800, field_t::decay_type::liquid,
            {_("shredded leaves and twigs"), _("shattered branches and leaves"), _("broken vegetation tangle")},
            {c_ltgreen, c_ltgreen, c_green},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_web", '}', 2, 0, field_t::decay_type::none,
            {_("cobwebs"),_("webs"), _("thick webs")},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_slime", '%', 0, 14400, field_t::decay_type::liquid,
            {_("slime trail"), _("slime stain"), _("puddle of slime")},
            {c_ltgreen, c_ltgreen, c_green},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_acid", '5', 2, 10, field_t::decay_type::none,
            {_("acid splatter"), _("acid streak"), _("pool of acid")},
            {c_ltgreen, c_green, c_green},
            {true, true, true},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_sap", '5', 2, 20, field_t::decay_type::none,
            {_("sap splatter"), _("glob of sap"), _("pool of sap")},
            {c_yellow, c_brown, c_brown},
            {true, true, true},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_sludge", '5', 2, 3600, field_t::decay_type::none,
            {_("thin sludge trail"), _("sludge trail"), _("thick sludge trail")},
            {c_ltgray, c_dkgray, c_black},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_fire", '4', 4, 800, field_t::decay_type::fire,
            {_("small fire"), _("fire"), _("raging fire")},
            {c_yellow, c_ltred, c_red},
            {true, true, true},
            {1.0f, 1.0f, 1.0f},
            {16, 60, 160},
        },
        {
            "fd_rubble", '#', 0, 1, field_t::decay_type::none,
            {_("legacy rubble"), _("legacy rubble"), _("legacy rubble")},
            {c_dkgray, c_dkgray, c_dkgray},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_smoke", '8', 8, 300, field_t::decay_type::gas,
            {_("thin smoke"), _("smoke"), _("thick smoke")},
            {c_white, c_ltgray, c_dkgray},
            {false, true, true},
            {1.0f, 0.5f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_toxic_gas", '8', 8, 900, field_t::decay_type::gas,
            {_("hazy cloud"),_("toxic gas"),_("thick toxic gas")},
            {c_white, c_ltgreen, c_green},
            {false, true, true},
            {1.0f, 0.5f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_tear_gas", '8', 8, 600, field_t::decay_type::gas,
            {_("hazy cloud"),_("tear gas"),_("thick tear gas")},
            {c_white, c_yellow, c_brown},
            {true, true, true},
            {1.0f, 0.5f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_nuke_gas", '8', 8, 1000, field_t::decay_type::gas,
            {_("hazy cloud"),_("radioactive gas"), _("thick radioactive gas")},
            {c_white, c_ltgreen, c_green},
            {true, true, true},
            {0.5f, 0.5f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_gas_vent", '%', 0, 0, field_t::decay_type::none,
            {_("gas vent"), _("gas vent"), _("gas vent")},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_fire_vent", '&', -1, 0, field_t::decay_type::none,
            {"", "", ""},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {8, 8, 8},
        },
        {
            "fd_flame_burst", '5', 4, 0, field_t::decay_type::none,
            {_("fire"), _("fire"), _("fire")},
            {c_red, c_red, c_red},
            {true, true, true},
            {1.0f, 1.0f, 1.0f},
            {8, 8, 8},
        },
        {
            "fd_electricity", '9', 4, 2, field_t::decay_type::none,
            {_("sparks"), _("electric crackle"), _("electric cloud")},
            {c_white, c_cyan, c_blue},
            {true, true, true},
            {1.0f, 1.0f, 1.0f},
            {1, 1, 8},
        },
        {
            "fd_fatigue", '*', 8, 0, field_t::decay_type::none,
            {_("odd ripple"), _("swirling air"), _("tear in reality")},
            {c_ltgray, c_dkgray, c_magenta},
            {false, false, false},
            {1.0f, 1.0f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_push_items", '&', -1, 0, field_t::decay_type::none,
            {"", "", ""},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_shock_vent", '&', -1, 0, field_t::decay_type::none,
            {"", "", ""},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_acid_vent", '&', -1, 0, field_t::decay_type::none,
            {"", "", ""},
            {c_white, c_white, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_plasma", '9', 4, 2, field_t::decay_type::none,
            {_("faint plasma"), _("glowing plasma"), _("glaring plasma")},
            {c_magenta, c_pink, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {1, 1, 8},
        },
        {
            "fd_laser", '#', 4, 1, field_t::decay_type::none,
            {_("faint glimmer"), _("beam of light"), _("intense beam of light")},
            {c_blue, c_ltblue, c_white},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {1, 1, 1},
        },
        {
            "fd_spotlight", '&', 1, 1, field_t::decay_type::none,
            { _("spotlight"), _("spotlight"), _("spotlight") },
            {c_white, c_white, c_white},
            { false, false, false },
            {1.0f, 1.0f, 1.0f},
            {20, 20, 20},
        },
        {
            "fd_dazzling", '#', 4, 1, field_t::decay_type::none,
            { _("dazzling"), _("dazzling"), _("dazzling") },
            {c_ltred_yellow, c_ltred_yellow, c_ltred_yellow},
            { false, false, false },
            {1.0f, 1.0f, 1.0f},
            {2, 2, 2},
        },
        {
            "fd_blood_veggy", '%', 0, 28800, field_t::decay_type::liquid,
            {_("plant sap splatter"), _("plant sap stain"), _("puddle of resin")},
            {c_ltgreen, c_ltgreen, c_ltgreen},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_blood_insect", '%', 0, 28800, field_t::decay_type::liquid,
            {_("bug blood splatter"), _("bug blood stain"), _("puddle of bug blood")},
            {c_green, c_green, c_green},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_blood_invertebrate", '%', 0, 28800, field_t::decay_type::liquid,
            {_("hemolymph splatter"), _("hemolymph stain"), _("puddle of hemolymph")},
            {c_ltgray, c_ltgray, c_ltgray},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_gibs_insect", '~', 0, 28800, field_t::decay_type::liquid,
            {_("shards of chitin"), _("shattered bug leg"), _("torn insect organs")},
            {c_ltgreen, c_green, c_yellow},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_gibs_invertebrate", '~', 0, 28800, field_t::decay_type::liquid,
            {_("gooey scraps"), _("icky mess"), _("heap of squishy gore")},
            {c_ltgray, c_ltgray, c_dkgray},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_cigsmoke", '%', 8, 350, field_t::decay_type::gas,
            {_("swirl of tobacco smoke"), _("tobacco smoke"), _("thick tobacco smoke")},
            {c_white, c_ltgray, c_dkgray},
            {false, false, false},
            {0.7f, 0.7f, 0.7f},
            {0, 0, 0},
        },
        {
            "fd_weedsmoke", '%', 8, 325, field_t::decay_type::gas,
            {_("swirl of pot smoke"), _("pot smoke"), _("thick pot smoke")},
            {c_white, c_ltgray, c_dkgray},
            {false, false, false},
            {0.7f, 0.7f, 0.7f},
            {0, 0, 0},
        },
        {
            "fd_cracksmoke", '%', 8, 225, field_t::decay_type::gas,
            {_("swirl of crack smoke"), _("crack smoke"), _("thick crack smoke")},
            {c_white, c_ltgray, c_dkgray},
            {false, false, false},
            {0.7f, 0.7f, 0.7f},
            {0, 0, 0},
        },
        {
            "fd_methsmoke", '%', 8, 275, field_t::decay_type::gas,
            {_("swirl of meth smoke"), _("meth smoke"), _("thick meth smoke")},
            {c_white, c_ltgray, c_dkgray},
            {false, false, false},
            {0.7f, 0.7f, 0.7f},
            {0, 0, 0},
        },
        {
            "fd_bees", '8', 8, 1000, field_t::decay_type::none,
            {_("some bees"), _("swarm of bees"), _("angry swarm of bees")},
            {c_white, c_ltgray, c_dkgray},
            {true, true, true},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_incendiary", '8', 8, 500, field_t::decay_type::none,
            {_("smoke"),_("airborne incendiary"), _("airborne incendiary")},
            {c_white, c_ltred, c_ltred_red},
            {true, true, true},
            {1.0f, 0.5f, 0.0f},
            {8, 16, 30},
        },
        {
            "fd_relax_gas", '.', 8, 500, field_t::decay_type::gas,
            {_("hazy cloud"),_("sedative gas"),_("relaxation gas")},
            { c_white, c_pink, c_cyan },
            { false, true, true },
            {0.7f, 0.7f, 0.7f},
            {0, 0, 0},
        },
        {
            "fd_fungal_haze", '.', 8, 40, field_t::decay_type::gas,
            {_("hazy cloud"),_("fungal haze"),_("thick fungal haze")},
            { c_white, c_cyan, c_cyan },
            { true, true, true },
            {1.0f, 0.5f, 0.0f},
            {0, 0, 0},
        },
        {
            "fd_hot_air1", '&', -1, 500, field_t::decay_type::gas,
            {"", "", ""},
            {c_white, c_yellow, c_red},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_hot_air2", '&', -1, 500, field_t::decay_type::gas,
            {"", "", ""},
            {c_white, c_yellow, c_red},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_hot_air3", '&', -1, 500, field_t::decay_type::gas,
            {"", "", ""},
            {c_white, c_yellow, c_red},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
        {
            "fd_hot_air4", '&', -1, 500, field_t::decay_type::gas,
            {"", "", ""},
            {c_white, c_yellow, c_red},
            {false, false, false},
            {1.0f, 1.0f, 1.0f},
            {0, 0, 0},
        },
    };

    for(int i = 0; i < num_fields; i++) {
        fieldlist[i] = std::move(tmp_fields[i]);
    }
}

field_id field_from_ident(const std::string &field_ident)
{
    for( size_t i = 0; i < num_fields; i++) {
        if( fieldlist[i].id == field_ident ) {
            return static_cast<field_id>( i );
        }
    }
    debugmsg( "unknown field ident %s", field_ident.c_str() );
    return fd_null;
}

/*
Function: spread_gas
Helper function that encapsulates the logic involved in gas spread.
*/
void map::spread_gas( field_entry *cur, int x, int y, field_id curtype,
                        int percent_spread, int outdoor_age_speedup )
{
    // Reset nearby scents to zero
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            g->scent( x + i, y + j ) = 0;
        }
    }
    // Dissipate faster outdoors.
    if (is_outside(x, y)) { cur->decay( outdoor_age_speedup ); }

    // Bail out if we don't meet the spread chance.
    if( rng(1, 100) > percent_spread ) { return; }

    std::vector <point> spread;
    // Pick all eligible points to spread to.
    for( int a = -1; a <= 1; a++ ) {
        for( int b = -1; b <= 1; b++ ) {
            // Current field not a candidate.
            if( !(a || b) ) { continue; }
            const field_entry* tmpfld = get_field( point( x + a, y + b ), curtype );
            // Candidates are existing weaker fields or navigable/flagged tiles with no field.
            if( (tmpfld && tmpfld->density() < cur->density() &&
                 (move_cost( x + a, y + b ) > 0 || has_flag("PERMEABLE", x + a, y + b))) ||
                (!tmpfld && (move_cost( x + a, y + b ) > 0 || has_flag("PERMEABLE", x + a, y + b))) ) {
                spread.push_back( point( x + a, y + b ) );
            }
        }
    }
    // Then, spread to a nearby point.
    int const current_density = cur->density();
    int const current_age = cur->age();
    if (current_density > 1 && current_age > 0 && !spread.empty()) {
        point p = spread[ rng( 0, spread.size() - 1 ) ];
        // Nearby gas grows thicker, and ages are shared.
        int age_fraction = 0.5 + current_age / current_density;
        if (field_entry *candidate_field = get_field(p.x, p.y).find(curtype)) {
            candidate_field->intensify(1);
            cur->intensify(-1);
            candidate_field->decay(age_fraction);
            cur->decay(-age_fraction);
        // Or, just create a new field.
        } else if ( add_field( p.x, p.y, curtype, 1 ) ) {
            get_field(p.x, p.y).find( curtype )->decay(age_fraction);
            cur->intensify(-1);
            cur->decay(-age_fraction);
        }
    }
}

/*
Function: create_hot_air
Helper function that encapsulates the logic involved in creating hot air.
*/
void map::create_hot_air( int x, int y, int density )
{
    int counter = 0;
    while (counter < 5) {
        int dx = rng(-1, 1);
        int dy = rng(-1, 1);
        if (density == 1)      add_field(x + dx, y + dy, fd_hot_air1, 1);
        else if (density == 2) add_field(x + dx, y + dy, fd_hot_air2, 1);
        else if (density == 3) add_field(x + dx, y + dy, fd_hot_air3, 1);
        else if (density == 4) add_field(x + dx, y + dy, fd_hot_air4, 1);
        counter++;
    }
}

bool map::process_fields()
{
 bool found_field = false;
 for (int x = 0; x < my_MAPSIZE; x++) {
  for (int y = 0; y < my_MAPSIZE; y++) {
   submap * const current_submap = get_submap_at_grid(x, y);
   if (current_submap->field_count > 0)
    found_field |= process_fields_in_submap(current_submap, x, y);
  }
 }
 if (found_field) {
     // For now, just always dirty the transparency cache
     // when a field might possibly be changed.
     // TODO: check if there are any fields(mostly fire)
     //       that frequently change, if so set the dirty
     //       flag, otherwise only set the dirty flag if
     //       something actually changed
     set_transparency_cache_dirty();
 }
 return found_field;
}

/*
Function: process_fields_in_submap
Iterates over every field on every tile of the given submap given as parameter.
This is the general update function for field effects. This should only be called once per game turn.
If you need to insert a new field behavior per unit time add a case statement in the switch below.
*/
bool map::process_fields_in_submap( submap *const current_submap,
                                    const int submap_x, const int submap_y )
{
    // Realistically this is always true, this function only gets called if fields exist.
    bool found_field = false;
    //Holds m.field_at(x,y).find(fd_some_field) type returns.
    // Just to avoid typing that long string for a temp value.
    field_entry *tmpfld = NULL;
    field_id curtype; //Holds cur->type() as thats what the old system used before rewrite.

    bool skipIterIncr = false; // keep track on when not to increment it[erator]

    //Loop through all tiles in this submap indicated by current_submap
    for (int locx = 0; locx < SEEX; locx++) {
        for (int locy = 0; locy < SEEY; locy++) {
            // This is a translation from local coordinates to submap coords.
            // All submaps are in one long 1d array.
            int x = locx + submap_x * SEEX;
            int y = locy + submap_y * SEEY;
            // get a copy of the field variable from the submap;
            // contains all the pointers to the real field effects.
            field &curfield = current_submap->fld[locx][locy];
            for( auto it = curfield.begin(); it != curfield.end();) {
                //Iterating through all field effects in the submap's field.
                field_entry *cur = &*it;

                curtype = cur->type();
                // Setting our return value. fd_null really doesn't exist anymore,
                // its there for legacy support.
                if (!found_field && curtype != fd_null) {
                    found_field = true;
                }
                // Again, legacy support in the event someone Mods setFieldDensity to allow more values.
                if (cur->density() > 3 || cur->density() < 1) {
                    debugmsg("Whoooooa density of %d", cur->density());
                }

                // Don't process "newborn" fields. This gives the player time to run if they need to.
                if (cur->age() == 0) {
                    curtype = fd_null;
                }

                int part;
                vehicle *veh;
                switch (curtype) {

                    case fd_null:
                    case num_fields:
                        break;  // Do nothing, obviously.  OBVIOUSLY.

                    case fd_blood:
                    case fd_blood_veggy:
                    case fd_blood_insect:
                    case fd_blood_invertebrate:
                    case fd_bile:
                    case fd_gibs_flesh:
                    case fd_gibs_veggy:
                    case fd_gibs_insect:
                    case fd_gibs_invertebrate:
                        if (has_flag("SWIMMABLE", x, y)) { // Dissipate faster in water
                            cur->decay(250);
                        }
                        break;

                    case fd_acid:
                    {
                        std::vector<item> contents;
                        if (has_flag("SWIMMABLE", x, y)) { // Dissipate faster in water
                            cur->decay(20);
                        }
                        auto items = i_at(x, y);
                        for( auto melting = items.begin(); melting != items.end(); ) {
                            // see DEVELOPER_FAQ.txt for how acid resistance is calculated
                            int chance = melting->acid_resist();
                            if (chance == 0) {
                                melting->damage++;
                            } else if (chance > 0 && chance < 9) {
                                if (one_in(chance)) {
                                    melting->damage++;
                                }
                            }
                            if (melting->damage >= 5) {
                                //Destroy the object, age the field.
                                cur->decay(melting->volume());
                                contents.insert( contents.begin(),
                                                 melting->contents.begin(), melting->contents.end() );
                                melting = items.erase( melting );
                            } else {
                                melting++;
                            }
                        }
                        for( auto &c : contents ) {
                            add_item_or_charges( x, y, c );
                        }
                    }
                        break;

                        // Use the normal aging logic below this switch
                    case fd_web:
                        break;
                    case fd_sap:
                        break;
                    case fd_sludge:
                        break;
                    case fd_slime:
                        break;
                    case fd_plasma:
                        break;
                    case fd_laser:
                        break;

                        // TODO-MATERIALS: use fire resistance
                    case fd_fire: {
                        auto items_here = i_at(x, y);
                        // explosions will destroy items on this square, iterating
                        // backwards makes sure that every item is visited.
                        for( auto explosive = items_here.begin();
                             explosive != items_here.end(); ) {
                            if( explosive->type->explode_in_fire() ) {
                                // Make a copy and let the copy explode.
                                item tmp = *explosive;
                                i_rem( point(x, y), explosive );
                                tmp.detonate(point(x,y));
                                // Just restart from the beginning.
                                explosive = items_here.begin();
                            } else {
                                ++explosive;
                            }
                        }

                        std::vector<item> new_content;
                        // Consume items as fuel to help us grow/last longer.
                        bool destroyed = false; //Is the item destroyed?
                        // Volume, Smoke generation probability, consumed items count
                        int vol = 0, smoke = 0, consumed = 0;
                        // The highest # of items this fire can remove in one turn
                        int max_consume = cur->density() * 2;
                        for( auto fuel = items_here.begin();
                             fuel != items_here.end() && consumed < max_consume; ) {
                            // Stop when we hit the end of the item buffer OR we consumed
                            // more than max_consume items
                            destroyed = false;
                            // Used to feed the fire based on volume of item burnt.
                            vol = fuel->volume();
                            const islot_ammo *ammo_type = NULL; //Special case if its ammo.

                            if( fuel->is_ammo() ) {
                                ammo_type = fuel->type->ammo.get();
                            }
                            // Types of ammo with special effects.
                            bool cookoff = false;
                            bool special = false;
                            //Flame type ammo removed so gasoline isn't explosive, it just burns.
                            if( ammo_type != NULL &&
                                ( !fuel->made_of("hydrocarbons") && !fuel->made_of("oil") ) ) {
                                cookoff = ammo_type->ammo_effects.count("INCENDIARY") ||
                                          ammo_type->ammo_effects.count("COOKOFF");
                                special = ammo_type->ammo_effects.count("FRAG") ||
                                          ammo_type->ammo_effects.count("NAPALM") ||
                                          ammo_type->ammo_effects.count("NAPALM_BIG") ||
                                          ammo_type->ammo_effects.count("EXPLOSIVE") ||
                                          ammo_type->ammo_effects.count("EXPLOSIVE_BIG") ||
                                          ammo_type->ammo_effects.count("EXPLOSIVE_HUGE") ||
                                          ammo_type->ammo_effects.count("TEARGAS") ||
                                          ammo_type->ammo_effects.count("SMOKE") ||
                                          ammo_type->ammo_effects.count("SMOKE_BIG") ||
                                          ammo_type->ammo_effects.count("FLASHBANG");
                            }

                            // How much more burnt the item will be,
                            // should be a multiple of 'base_burn_amt'.
                            int burn_amt = 0;
                            // 'burn_amt' / 'base_burn_amt' == 1 to 'consumed',
                            // Right now all materials are 1, except paper, which is 3
                            // This means paper is consumed 3x as fast
                            int base_burn_amt = 1;
                            // How much time to add to the fire's life
                            int time_added = 0;

                            if( special || cookoff ) {
                                int charges_remaining = fuel->charges;
                                const long rounds_exploded = rng( 1, charges_remaining );
                                // Yank the exploding item off the map for the duration of the explosion
                                // so it doesn't blow itself up.
                                item temp_item = *fuel;
                                items_here.erase( fuel );
                                // cook off ammo instead of just burning it.
                                for(int j = 0; j < (rounds_exploded / 10) + 1; j++) {
                                    if( cookoff ) {
                                        // Ammo that cooks off, but doesn't have a
                                        // large intrinsic effect blows up with half
                                        // the ammos damage in force, for each bullet,
                                        // just creating shrapnel.
                                        g->explosion( x, y, ammo_type->damage / 2,
                                                      true, false, false );
                                    } else if( special ) {
                                        // If it has a special effect just trigger it.
                                        ammo_effects( x, y, ammo_type->ammo_effects );
                                    }
                                }
                                charges_remaining -= rounds_exploded;
                                if( charges_remaining > 0 ) {
                                    temp_item.charges = charges_remaining;
                                    items_here.push_back( temp_item );
                                }
                                // Can't find an easy way to handle reinserting the ammo into a potentially
                                // invalidated list and continuing iteration, so just bail out.
                                break;
                            } else if( fuel->made_of("paper") ) {
                                //paper items feed the fire moderately.
                                base_burn_amt = 3;
                                burn_amt = base_burn_amt * (max_consume - consumed);
                                if (ammo_type != NULL) {
                                    if( fuel->charges - burn_amt < 0 ) {
                                        burn_amt = fuel->charges;
                                    }
                                }
                                if (cur->density() == 1) {
                                    time_added = vol * 10;
                                    time_added += (vol * 10) * (burn_amt / base_burn_amt);
                                }
                                if (vol >= 4) {
                                    smoke++;    //Large paper items give chance to smoke.
                                }

                            } else if( fuel->made_of("wood") || fuel->made_of("veggy") ) {
                                //Wood or vegy items burn slowly.
                                if (vol <= cur->density() * 10 ||
                                    cur->density() == 3) {
                                    // A single wood item will just maintain at the current level.
                                    time_added = 1;
                                    // ammo has more surface area, and burns quicker
                                    if (one_in( (ammo_type != NULL) ? 25 : 50 )) {
                                        burn_amt = cur->density();
                                    }
                                } else if( fuel->burnt < cur->density() ) {
                                    burn_amt = 1;
                                }
                                smoke++;

                            } else if( (fuel->made_of("cotton") || fuel->made_of("wool")) &&
                                       !fuel->made_of("nomex") ) {
                                //Cotton and wool moderately quickly but don't feed the fire much.
                                if( vol <= 5 || cur->density() > 1 ) {
                                    time_added = 1;
                                    burn_amt = cur->density();
                                } else if( x_in_y( cur->density(), fuel->burnt ) ) {
                                    burn_amt = 1;
                                }
                                smoke++;

                            } else if( fuel->made_of("flesh") || fuel->made_of("hflesh") ||
                                       fuel->made_of("iflesh") ) {
                                // Slow and smokey
                                if( vol <= cur->density() * 5 ||
                                    (cur->density() == 3 && one_in(vol / 20))) {
                                    time_added = 1;
                                    burn_amt = cur->density();
                                    smoke += 3;
                                } else if( x_in_y( cur->density(), fuel->burnt ) ) {
                                    burn_amt = 1;
                                    smoke++;
                                }

                            } else if( fuel->made_of(LIQUID) ) {
                                // Lots of smoke if alcohol, and LOTS of fire fueling power
                                if( fuel->made_of("hydrocarbons") ) {
                                    time_added = 300;
                                    smoke += 6;
                                } else if( fuel->made_of("alcohol") && fuel->made_of().size() == 1 ) {
                                    // Only strong alcohol for now
                                    time_added = 250;
                                    smoke += 1;
                                } else if( fuel->type->id == "lamp_oil" ) {
                                    time_added = 300;
                                    smoke += 3;
                                } else {
                                    // kills a fire otherwise.
                                    time_added = -rng(80 * vol, 300 * vol);
                                    smoke++;
                                }
                                // burn_amt will get multiplied by stack size in item::burn
                                burn_amt = cur->density();

                            } else if( fuel->made_of("powder") ) {
                                // Any powder will fuel the fire as 100 times much as its volume
                                // but be immediately destroyed.
                                time_added = vol * 100;
                                destroyed = true;
                                smoke += 2;

                            } else if( fuel->made_of("plastic") && !fuel->made_of("nomex") ) {
                                //Smokey material, doesn't fuel well.
                                smoke += 3;
                                if( fuel->burnt <= cur->density() * 2 ||
                                    (cur->density() == 3 && one_in(vol)) ) {
                                    burn_amt = cur->density();
                                    if( one_in( (ammo_type != NULL) ? vol : fuel->burnt ) ) {
                                        time_added = 1;
                                    }
                                }
                            } else if( !fuel->made_of("nomex") ) {
                                // Generic materials, like bone, wheat or fruit
                                // Just damage and smoke, don't feed the fire
                                int best_res = 0;
                                for( auto mat : fuel->made_of_types() ) {
                                    best_res = std::max( best_res, mat->fire_resist() );
                                }
                                if( best_res < cur->density() && one_in( fuel->volume( true, false ) ) ) {
                                    smoke++;
                                    burn_amt = cur->density() - best_res;
                                }
                            }
                            if( !destroyed ) {
                                destroyed = fuel->burn( burn_amt );
                            }

                            //lower age is a longer lasting fire
                            cur->decay(-time_added);

                            if( destroyed ) {
                                //If we decided the item was destroyed by fire, remove it.
                                new_content.insert( new_content.end(),
                                                    fuel->contents.begin(), fuel->contents.end() );
                                fuel = items_here.erase( fuel );
                            } else {
                                ++fuel;
                            }
                        }
                        spawn_items( x, y, new_content );

                        veh = veh_at(x, y, part); //Get the part of the vehicle in the fire.
                        if (veh) {
                            veh->damage(part, cur->density() * 10, 2, false);
                            //Damage the vehicle in the fire.
                        }
                        // If the flames are in a brazier, they're fully contained,
                        // so skip consuming terrain
                        if((tr_brazier != tr_at(x, y)) &&
                           (has_flag("FIRE_CONTAINER", x, y) != true )) {
                            // Consume the terrain we're on
                            if (has_flag("FLAMMABLE", x, y) && one_in(32 - cur->density() * 10)) {
                                //The fire feeds on the ground itself until max density.
                                cur->decay(-cur->density() * cur->density() * 40);
                                smoke += 15;
                                if (cur->density() == 3) {
                                    destroy(x, y, true);
                                }

                            } else if (has_flag("FLAMMABLE_ASH", x, y) &&
                                       one_in(32 - cur->density() * 10)) {
                                //The fire feeds on the ground itself until max density.
                                cur->decay(-cur->density() * cur->density() * 40);
                                smoke += 15;
                                if (cur->density() == 3 || cur->age() < -600) {
                                    ter_set(x, y, t_dirt);
                                    furn_set(x, y, f_ash);
                                }

                            } else if (has_flag("FLAMMABLE_HARD", x, y) &&
                                       one_in(62 - cur->density() * 10)) {
                                //The fire feeds on the ground itself until max density.
                                cur->decay(-cur->density() * cur->density() * 30);
                                smoke += 10;
                                if (cur->density() == 3 || cur->age() < -600) {
                                    destroy(x, y, true);
                                }

                            } else if (terlist[ter(x, y)].has_flag("SWIMMABLE")) {
                                cur->decay(800);
                                // Flames die quickly on water
                            }
                        }

                        // If the flames are in a pit, it can't spread to non-pit
                        bool in_pit = (ter(x, y) == t_pit);

                        // If the flames are REALLY big, they contribute to adjacent flames
                        if (cur->age() < 0 && tr_brazier != tr_at(x, y) &&
                            (has_flag("FIRE_CONTAINER", x, y) != true  ) ) {
                            if(cur->density() == 3) {
                                // Randomly offset our x/y shifts by 0-2, to randomly pick
                                // a square to spread to
                                int starti = rng(0, 2);
                                int startj = rng(0, 2);
                                tmpfld = NULL;
                                // Basically: Scan around for a spot,
                                // if there is more fire there, make it bigger and
                                // both flames renew in power
                                // This is how level 3 fires spend their excess age:
                                // making other fires bigger. Flashpoint.
                                for (int i = 0; i < 3 && cur->age() < 0; i++) {
                                    for (int j = 0; j < 3 && cur->age() < 0; j++) {
                                        int fx = x + ((i + starti) % 3) - 1;
                                        int fy = y + ((j + startj) % 3) - 1;
                                        tmpfld = get_field(fx, fy).find(fd_fire);
                                        if (tmpfld && tmpfld != cur && cur->age() < 0 &&
                                            tmpfld->density() < 3 &&
                                            (in_pit == (ter(fx, fy) == t_pit))) {
                                            tmpfld->intensify(1);
                                            cur->decay(150);
                                        }
                                    }
                                }
                            } else {
                                // See if we can grow into a stage 2/3 fire, for this
                                // burning neighbours are necessary in addition to
                                // field age < 0, or alternatively, a LOT of fuel.

                                // The maximum fire density is 1 for a lone fire, 2 for at least 1 neighbour,
                                // 3 for at least 2 neighbours.
                                int maximum_density =  1;

                                // The following logic looks a bit complex due to optimization concerns, so here are the semantics:
                                // 1. Calculate maximum field density based on fuel, -500 is 2(raging), -1000 is 3(inferno)
                                // 2. Calculate maximum field density based on neighbours, 1 neighbours is 2(raging), 2 or more neighbours is 3(inferno)
                                // 3. Pick the higher maximum between 1. and 2.
                                // We don't just calculate both maximums and pick the higher because the adjacent field lookup is quite expensive and should be avoided if possible.
                                if(cur->age() < -1500) {
                                    maximum_density = 3;
                                } else {
                                    int adjacent_fires = 0;

                                    for (int i = 0; i < 3; i++) {
                                        for (int j = 0; j < 3; j++) {
                                            if( i == 1 && j == 1 ) {
                                                continue;
                                            }
                                            const point pnt( x + (i % 3) - 1, y + (j % 3) - 1 );
                                            if( get_field( pnt, fd_fire ) != nullptr ) {
                                                adjacent_fires++;
                                            }
                                        }
                                    }
                                    maximum_density = 1 + (adjacent_fires >= 1) + (adjacent_fires >= 2);

                                    if(maximum_density < 2 && cur->age() < -500) {
                                        maximum_density = 2;
                                    }
                                }

                                // If we consumed a lot, the flames grow higher
                                while (cur->density() < maximum_density && cur->age() < 0) {
                                    //Fires under 0 age grow in size. Level 3 fires under 0 spread later on.
                                    cur->decay(300);
                                    cur->intensify(1);
                                }
                            }
                        }

                        // Consume adjacent fuel / terrain / webs to spread.
                        // Randomly offset our x/y shifts by 0-2, to randomly pick a square to spread to
                        //Fires can only spread under 30 age. This is arbitrary but seems to work well.
                        //The reason is to differentiate a fire that spawned vs one created really.
                        //Fires spawned by fire in a new square START at 30 age, so if its a square with no fuel on it
                        //the fire won't keep crawling endlessly across the map.
                        int starti = rng(0, 2);
                        int startj = rng(0, 2);
                        for (int i = 0; i < 3; i++) {
                            for (int j = 0; j < 3; j++) {
                                int fx = x + ((i + starti) % 3) - 1, fy = y + ((j + startj) % 3) - 1;
                                if (INBOUNDS(fx, fy)) {
                                    field &nearby_field = get_field(fx, fy);
                                    field_entry *nearwebfld = nearby_field.find(fd_web);
                                    int spread_chance = 25 * (cur->density() - 1);
                                    if (nearwebfld) {
                                        spread_chance = 50 + spread_chance / 2;
                                    }
                                    if ((i != 0 || j != 0) && rng(1, 100) < spread_chance &&
                                          cur->age() < 200 && tr_brazier != tr_at(x, y) &&
                                          (has_flag("FIRE_CONTAINER", x, y) != true ) &&
                                          (in_pit == (ter(fx, fy) == t_pit)) &&
                                          ((cur->density() >= 2 && (has_flag("FLAMMABLE", fx, fy) && one_in(20))) ||
                                          (cur->density() >= 2  && (has_flag("FLAMMABLE_ASH", fx, fy) && one_in(10))) ||
                                          (cur->density() == 3  && (has_flag("FLAMMABLE_HARD", fx, fy) && one_in(10))) ||
                                          flammable_items_at(fx, fy) || nearwebfld )) {
                                        add_field(fx, fy, fd_fire, 1); //Nearby open flammable ground? Set it on fire.
                                        tmpfld = nearby_field.find(fd_fire);
                                        if(tmpfld) {
                                            tmpfld->decay(100);
                                            cur->decay(50);
                                        }
                                        if (nearwebfld) {
                                            auto const result = nearby_field.remove(fd_web);
                                            if (result.second) {
                                                get_submap_at(fx, fy)->field_count--;
                                            }

                                            if (fx == x && fy == y) {
                                                it = result.first;
                                                skipIterIncr = true;
                                            }
                                        }
                                    } else {
                                        bool nosmoke = true;
                                        for (int ii = -1; ii <= 1; ii++) {
                                            for (int jj = -1; jj <= 1; jj++) {
                                                const point pnt( x + ii, y + jj );
                                                const field_entry *fire = get_field( pnt, fd_fire );
                                                const field_entry *smoke = get_field( pnt, fd_smoke );
                                                if( fire != nullptr && ( fire->density() == 3 ||
                                                    ( fire->density() == 2 && one_in(4) ) ) ) {
                                                    smoke++; //The higher this gets, the more likely for smoke.
                                                } else if( smoke != nullptr ) {
                                                    nosmoke = false; //slightly, slightly, less likely to make smoke if there is already smoke
                                                }
                                            }
                                        }
                                        // If we're not spreading, maybe we'll stick out some smoke, huh?
                                        if(!(is_outside(fx, fy))) {
                                            //Lets make more smoke indoors since it doesn't dissipate
                                            smoke += 10; //10 is just a magic number. To much smoke indoors? Lower it. To little, raise it.
                                        }
                                        if (move_cost(fx, fy) > 0 &&
                                            (rng(0, 100) <= smoke || (nosmoke && one_in(40))) &&
                                            rng(3, 35) < cur->density() * 5 && cur->age() < 1000 &&
                                            (has_flag("SUPPRESS_SMOKE", x, y) != true )) {
                                            smoke--;
                                            add_field(fx, fy, fd_smoke, rng(1, cur->density())); //Add smoke!
                                        }
                                    }
                                }
                            }
                        }
                        create_hot_air( x, y, cur->density());
                    }
                    break;

                    case fd_smoke:
                        spread_gas( cur, x, y, curtype, 80, 50 );
                        break;

                    case fd_tear_gas:
                        spread_gas( cur, x, y, curtype, 33, 30 );
                        break;

                    case fd_relax_gas:
                        spread_gas( cur, x, y, curtype, 25, 50 );
                        break;

                    case fd_fungal_haze:
                        spread_gas( cur, x, y, curtype, 33,  5);
                        int mondex;
                        mondex = g->mon_at(x, y);
                        if (move_cost(x, y) > 0) {
                            if (mondex != -1) { // Haze'd!
                                if (!g->zombie(mondex).type->in_species("FUNGUS") &&
                                  !g->zombie(mondex).type->has_flag("NO_BREATHE")) {
                                    if (g->u.sees(x, y)) {
                                        add_msg(m_info, _("The %s inhales thousands of live spores!"),
                                    g->zombie(mondex).name().c_str());
                                    }
                                    monster &critter = g->zombie( mondex );
                                    if( !critter.make_fungus() ) {
                                        critter.die(nullptr);
                                    }
                                }
                        } if (one_in(5 - cur->density())) {
                            g->spread_fungus(x, y); //Haze'd terrain
                        }
                        }
                        break;

                    case fd_toxic_gas:
                        spread_gas( cur, x, y, curtype, 50, 30 );
                        break;

                    case fd_cigsmoke:
                        spread_gas( cur, x, y, curtype, 250, 65 );
                        break;

                    case fd_weedsmoke: {
                        spread_gas( cur, x, y, curtype, 200, 60 );

                        if(one_in(20)) {
                            int npcdex = g->npc_at(x, y);
                            if (npcdex != -1) {
                                npc *p = g->active_npc[npcdex];
                                if(p->is_friend()) {
                                    p->say(one_in(10) ? _("Whew... smells like skunk!") : _("Man, that smells like some good shit!"));
                                }
                            }
                        }

                    }
                        break;

                    case fd_methsmoke: {
                        spread_gas( cur, x, y, curtype, 175, 70 );

                        if(one_in(20)) {
                            int npcdex = g->npc_at(x, y);
                            if (npcdex != -1) {
                                npc *p = g->active_npc[npcdex];
                                if(p->is_friend()) {
                                    p->say(_("I don't know... should you really be smoking that stuff?"));
                                }
                            }
                        }
                    }
                        break;

                    case fd_cracksmoke: {
                        spread_gas( cur, x, y, curtype, 175, 80 );

                        if(one_in(20)) {
                            int npcdex = g->npc_at(x, y);
                            if (npcdex != -1) {
                                npc *p = g->active_npc[npcdex];
                                if(p->is_friend()) {
                                    p->say(one_in(2) ? _("Ew, smells like burning rubber!") : _("Ugh, that smells rancid!"));
                                }
                            }
                        }
                    }
                        break;

                    case fd_nuke_gas: {
                        int extra_radiation = rng(0, cur->density());
                        adjust_radiation(x, y, extra_radiation);
                        spread_gas( cur, x, y, curtype, 50, 10 );
                        break;
                    }

                    case fd_hot_air1:
                    case fd_hot_air2:
                    case fd_hot_air3:
                    case fd_hot_air4:
                        spread_gas( cur, x, y, curtype, 100, 1000 );
                        break;

                    case fd_gas_vent:
                        for (int i = x - 1; i <= x + 1; i++) {
                            for (int j = y - 1; j <= y + 1; j++) {
                                field &wandering_field = get_field(i, j);
                                tmpfld = wandering_field.find(fd_toxic_gas);
                                if (tmpfld && tmpfld->density() < 3) {
                                    tmpfld->intensify(1);
                                } else {
                                    add_field(i, j, fd_toxic_gas, 3);
                                }
                            }
                        }
                        break;

                    case fd_fire_vent:
                        if (cur->density() > 1) {
                            if (one_in(3)) {
                                cur->intensify(-1);
                            }
                        } else {
                            curfield.remove(fd_fire_vent);
                            curfield.add(fd_flame_burst, 3);
                            ++it;
                            continue;
                        }
                        create_hot_air( x, y, cur->density());
                        break;

                    case fd_flame_burst:
                        if (cur->density() > 1) {
                            cur->intensify(-1);
                        } else {
                            curfield.remove(fd_flame_burst);
                            curfield.add(fd_fire_vent, 3);
                            ++it;
                            continue;
                        }
                        create_hot_air( x, y, cur->density());
                        break;

                    case fd_electricity:

                        if (!one_in(5)) {   // 4 in 5 chance to spread
                            std::vector<point> valid;
                            if (move_cost(x, y) == 0 && cur->density() > 1) { // We're grounded
                                int tries = 0;
                                while (tries < 10 && cur->age() < 50 && cur->density() > 1) {
                                    int cx = x + rng(-1, 1), cy = y + rng(-1, 1);
                                    if (move_cost(cx, cy) != 0) {
                                        add_field(point(cx, cy), fd_electricity, 1, cur->age() + 1);
                                        cur->intensify(-1);
                                        tries = 0;
                                    } else {
                                        tries++;
                                    }
                                }
                            } else {    // We're not grounded; attempt to ground
                                for (int a = -1; a <= 1; a++) {
                                    for (int b = -1; b <= 1; b++) {
                                        if (move_cost(x + a, y + b) == 0) // Grounded tiles first

                                        {
                                            valid.push_back(point(x + a, y + b));
                                        }
                                    }
                                }
                                if (valid.empty()) {    // Spread to adjacent space, then
                                    int px = x + rng(-1, 1), py = y + rng(-1, 1);
                                    field_entry *elec = get_field( px, py ).find( fd_electricity );
                                    if (move_cost(px, py) > 0 && elec != nullptr &&
                                        elec->density() < 3) {
                                        elec->intensify(1);
                                        cur->intensify(-1);
                                    } else if (move_cost(px, py) > 0) {
                                        add_field(point(px, py), fd_electricity, 1, cur->age() + 1);
                                    }
                                    cur->intensify(-1);
                                }
                                while (!valid.empty() && cur->density() > 1) {
                                    int index = rng(0, valid.size() - 1);
                                    add_field(valid[index], fd_electricity, 1, cur->age() + 1);
                                    cur->intensify(-1);
                                    valid.erase(valid.begin() + index);
                                }
                            }
                        }
                        break;

                    case fd_fatigue:{
                        std::array<std::string, 9> monids = { { "mon_flying_polyp", "mon_hunting_horror",
                        "mon_mi_go", "mon_yugg", "mon_gelatin", "mon_flaming_eye", "mon_kreck", "mon_gracke",
                        "mon_blank" } };
                        if (cur->density() < 3 && int(calendar::turn) % 3600 == 0 && one_in(10)) {
                            cur->intensify(1);
                        } else if (cur->density() == 3 && one_in(600)) { // Spawn nether creature!
                            std::string type = monids[rng( 0, monids.size() - 1 )];
                            monster creature(GetMType(type));
                            creature.spawn(x + rng(-3, 3), y + rng(-3, 3));
                            g->add_zombie(creature);
                        }
                    }
                        break;

                    case fd_push_items: {
                        auto items = i_at(x, y);
                        for( auto pushee = items.begin(); pushee != items.end(); ) {
                            if( pushee->type->id != "rock" ||
                                pushee->bday >= int(calendar::turn) - 1 ) {
                                pushee++;
                            } else {
                                item tmp = *pushee;
                                tmp.bday = int(calendar::turn);
                                pushee = items.erase( pushee );
                                std::vector<point> valid;
                                for (int xx = x - 1; xx <= x + 1; xx++) {
                                    for (int yy = y - 1; yy <= y + 1; yy++) {
                                        if( get_field( point( xx, yy ), fd_push_items ) != nullptr ) {
                                            valid.push_back( point(xx, yy) );
                                        }
                                    }
                                }
                                if (!valid.empty()) {
                                    point newp = valid[rng(0, valid.size() - 1)];
                                    add_item_or_charges(newp.x, newp.y, tmp);
                                    if (g->u.posx() == newp.x && g->u.posy() == newp.y) {
                                        add_msg(m_bad, _("A %s hits you!"), tmp.tname().c_str());
                                        body_part hit = random_body_part();
                                        g->u.deal_damage( nullptr, hit, damage_instance( DT_BASH, 6 ) );
                                        g->u.check_dead_state();
                                    }
                                    int npcdex = g->npc_at(newp.x, newp.y);
                                    int mondex = g->mon_at(newp.x, newp.y);

                                    if (npcdex != -1) {
                                        // TODO: combine with player character code above
                                        npc *p = g->active_npc[npcdex];
                                        body_part hit = random_body_part();
                                        p->deal_damage( nullptr, hit, damage_instance( DT_BASH, 6 ) );
                                        if (g->u.sees( newp )) {
                                            add_msg(_("A %s hits %s!"), tmp.tname().c_str(), p->name.c_str());
                                        }
                                        p->check_dead_state();
                                    }

                                    if (mondex != -1) {
                                        monster *mon = &(g->zombie(mondex));
                                        mon->apply_damage( nullptr, bp_torso, 6 - mon->get_armor_bash( bp_torso ) );
                                        if (g->u.sees( newp ))
                                            add_msg(_("A %s hits the %s!"), tmp.tname().c_str(),
                                                       mon->name().c_str());
                                        mon->check_dead_state();
                                    }
                                }
                            }
                        }
                    }
                    break;

                    case fd_shock_vent:
                        if (cur->density() > 1) {
                            if (one_in(5)) {
                                cur->intensify(-1);
                            }
                        } else {
                            cur->maximize();
                            int num_bolts = rng(3, 6);
                            for (int i = 0; i < num_bolts; i++) {
                                int xdir = 0, ydir = 0;
                                while (xdir == 0 && ydir == 0) {
                                    xdir = rng(-1, 1);
                                    ydir = rng(-1, 1);
                                }
                                int dist = rng(4, 12);
                                int boltx = x, bolty = y;
                                for (int n = 0; n < dist; n++) {
                                    boltx += xdir;
                                    bolty += ydir;
                                    add_field(boltx, bolty, fd_electricity, rng(2, 3));
                                    if (one_in(4)) {
                                        if (xdir == 0) {
                                            xdir = rng(0, 1) * 2 - 1;
                                        } else {
                                            xdir = 0;
                                        }
                                    }
                                    if (one_in(4)) {
                                        if (ydir == 0) {
                                            ydir = rng(0, 1) * 2 - 1;
                                        } else {
                                            ydir = 0;
                                        }
                                    }
                                }
                            }
                        }
                        break;

                    case fd_acid_vent:
                        if (cur->density() > 1) {
                            if (cur->age() >= 10) {
                                cur->intensify(-1);
                                cur->decay(-cur->age()); // 0
                            }
                        } else {
                            cur->maximize();
                            for (int i = x - 5; i <= x + 5; i++) {
                                for (int j = y - 5; j <= y + 5; j++) {
                                    const field_entry *acid = get_field( point( i, j ), fd_acid );
                                    if( acid != nullptr && acid->density() == 0 ) {
                                            int newdens = 3 - (rl_dist(x, y, i, j) / 2) + (one_in(3) ? 1 : 0);
                                            if (newdens > 3) {
                                                newdens = 3;
                                            }
                                            if (newdens > 0) {
                                                add_field(i, j, fd_acid, newdens);
                                            }
                                    }
                                }
                            }
                        }
                        break;

                    case fd_bees:
                        // Poor bees are vulnerable to so many other fields.
                        // TODO: maybe adjust effects based on different fields.
                        if( curfield.find( fd_web ) ||
                            curfield.find( fd_fire ) ||
                            curfield.find( fd_smoke ) ||
                            curfield.find( fd_toxic_gas ) ||
                            curfield.find( fd_tear_gas ) ||
                            curfield.find( fd_relax_gas ) ||
                            curfield.find( fd_nuke_gas ) ||
                            curfield.find( fd_gas_vent ) ||
                            curfield.find( fd_fire_vent ) ||
                            curfield.find( fd_flame_burst ) ||
                            curfield.find( fd_electricity ) ||
                            curfield.find( fd_fatigue ) ||
                            curfield.find( fd_shock_vent ) ||
                            curfield.find( fd_plasma ) ||
                            curfield.find( fd_laser ) ||
                            curfield.find( fd_dazzling) ||
                            curfield.find( fd_electricity ) ||
                            curfield.find( fd_incendiary ) ) {
                            // Kill them at the end of processing.
                            cur->nullify();
                        } else {
                            // Bees chase the player if in range, wander randomly otherwise.
                            int junk;
                            if( !g->u.is_underwater() &&
                                rl_dist( x, y, g->u.posx(), g->u.posy() ) < 10 &&
                                clear_path( x, y, g->u.posx(), g->u.posy(), 10, 0, 100, junk ) ) {

                                std::vector<point> candidate_positions =
                                    squares_in_direction( x, y, g->u.posx(), g->u.posy() );
                                for( auto &candidate_position : candidate_positions ) {
                                    field &target_field =
                                        get_field( candidate_position.x, candidate_position.y );
                                    // Only shift if there are no bees already there.
                                    // TODO: Figure out a way to merge bee fields without allowing
                                    // Them to effectively move several times in a turn depending
                                    // on iteration direction.
                                    if( !target_field.find( fd_bees ) ) {
                                        add_field( candidate_position, fd_bees,
                                                   cur->density(), cur->age() );
                                        cur->nullify();
                                        break;
                                    }
                                }
                            } else {
                                spread_gas( cur, x, y, curtype, 5, 0 );
                            }
                        }
                        break;

                    case fd_incendiary:
                        { //Needed for variable scope
                            int offset_x = x + rng(-1,1);
                            int offset_y = y + rng(-1,1); //pick a random adjacent tile and attempt to set that on fire
                            if( has_flag("FLAMMABLE", offset_x, offset_y) ||
                                has_flag("FLAMMABLE_ASH",offset_x, offset_y) ||
                                has_flag("FLAMMABLE_HARD", offset_x, offset_y) ) {
                                add_field(offset_x, offset_y , fd_fire, 1);
                            }

                            //check piles for flammable items and set those on fire
                            for( auto &target_item : i_at(x, y) ) {
                                if( target_item.made_of("paper") || target_item.made_of("wood") ||
                                    target_item.made_of("veggy") || target_item.made_of("cotton") ||
                                    target_item.made_of("wool") || target_item.made_of("flesh") ||
                                    target_item.made_of("hflesh") || target_item.made_of("iflesh") ||
                                    target_item.type->id == "gasoline" || target_item.type->id == "diesel" ||
                                    target_item.type->id == "lamp_oil" || target_item.type->id == "tequila" ||
                                    target_item.type->id == "whiskey" || target_item.type->id == "vodka" ||
                                    target_item.type->id == "rum" || target_item.type->id == "single_malt_whiskey" ||
                                    target_item.type->id == "gin" || target_item.type->id == "moonshine" ||
                                    target_item.type->id == "brandy" ) {
                                    add_field(x, y, fd_fire, 1);
                                }
                            }

                            spread_gas( cur, x, y, curtype, 66, 40 );
                            create_hot_air( x, y, cur->density());
                        }
                        break;

                    //Legacy Stuff
                    case fd_rubble:
                        make_rubble(x, y);
                        break;

                    default:
                        //Suppress warnings
                        break;

                } // switch (curtype)

                if (!cur->update()) {
                    current_submap->field_count--;
                    it = current_submap->fld[locx][locy].remove(cur->type()).first;
                    continue;
                }

                if (!skipIterIncr)
                    ++it;
                skipIterIncr = false;
            }
        }
    }
    return found_field;
}

bool field_entry::update()
{
    if (!is_alive()) {
        return false;
    }

    ++age_;
    int const halflife = definition().halflife;
    if (halflife <= 0) {
        return true;
    }

    if (dice(2, age_) > halflife) {
        age_ = 0;
        intensify(-1);
    }

    return is_alive();
}

void field_entry::decay(int const delta)
{
    age_ += delta;
}

bool field_entry::intensify(int const delta)
{
    set_density_(density_ + delta);
    return is_alive();
}

void field_entry::nullify()
{
    set_density_(0);
}
    
void field_entry::maximize()
{
    set_density_(3);
}

//This entire function makes very little sense. Why are the rules the way they are? Why does walking into some things destroy them but not others?

/*
Function: step_in_field
Triggers any active abilities a field effect would have. Fire burns you, acid melts you, etc.
If you add a field effect that interacts with the player place a case statement in the switch here.
If you wish for a field effect to do something over time (propagate, interact with terrain, etc) place it in process_subfields
*/
void map::player_in_field( player &u )
{
    // A copy of the current field for reference. Do not add fields to it, use map::add_field
    field &curfield = get_field( u.posx(), u.posy() );
    int veh_part; // vehicle part existing on this tile.
    vehicle *veh = NULL; // Vehicle reference if there is one.
    bool inside = false; // Are we inside?
    //to modify power of a field based on... whatever is relevant for the effect.
    int adjusted_intensity;

    //If we are in a vehicle figure out if we are inside (reduces effects usually)
    // and what part of the vehicle we need to deal with.
    if (u.in_vehicle) {
        veh = veh_at( u.posx(), u.posy(), veh_part );
        inside = (veh && veh->is_inside(veh_part));
    }

    // Iterate through all field effects on this tile.
    // When removing a field, do field_list_it = curfield.remove(type) and continue
    // This ensures proper iteration through the fields.
    for( auto field_list_it = curfield.begin(); field_list_it != curfield.end(); ){
        field_entry *cur = &*field_list_it;

        //Do things based on what field effect we are currently in.
        switch (cur->type()) {
        case fd_null:
        case fd_blood: // It doesn't actually do anything //necessary to add other types of blood?
        case fd_bile:  // Ditto
        case fd_cigsmoke:
        case fd_weedsmoke:
        case fd_methsmoke:
        case fd_cracksmoke:
            //break instead of return in the event of post-processing in the future;
            // also we're in a loop now!
            break;

        case fd_web: {
            //If we are in a web, can't walk in webs or are in a vehicle, get webbed maybe.
            //Moving through multiple webs stacks the effect.
            if (!u.in_vehicle && !u.has_trait("WEB_WALKER")) {
                //between 5 and 15 minus your current web level.
                u.add_effect("webbed", 1, num_bp, true, cur->density());
            }

            // If you are in a vehicle destroy the web.
            // It should have been destroyed when you ran over it anyway.
            if (u.in_vehicle || !u.has_trait("WEB_WALKER")) {
                field_list_it = curfield.remove(fd_web).first;
                get_submap_at(u.posx(), u.posy())->field_count--;
                continue;
            }
        } break;

        case fd_acid:
            //TODO: Add resistance to this with rubber shoes or something?
            // Assume vehicles block acid damage entirely,
            // you're certainly not standing in it.
            if (veh) {
                break;
            }
            if (cur->density() == 3) {
                u.add_msg_player_or_npc(m_bad, _("The acid burns your legs and feet!"), _("The acid burns <npcname>s legs and feet!"));
                u.deal_damage( nullptr, bp_foot_l, damage_instance( DT_ACID, rng( 4, 10 ) ) );
                u.deal_damage( nullptr, bp_foot_r, damage_instance( DT_ACID, rng( 4, 10 ) ) );
                u.deal_damage( nullptr, bp_leg_l, damage_instance( DT_ACID, rng( 2, 8 ) ) );
                u.deal_damage( nullptr, bp_leg_r, damage_instance( DT_ACID, rng( 2, 8 ) ) );
            } else if (cur->density() == 2) {
                u.add_msg_player_or_npc(m_bad, _("The acid burns your legs and feet!"), _("The acid burns <npcname>s legs and feet!"));
                u.deal_damage( nullptr, bp_foot_l, damage_instance( DT_ACID, rng( 2, 5 ) ) );
                u.deal_damage( nullptr, bp_foot_r, damage_instance( DT_ACID, rng( 2, 5 ) ) );
                u.deal_damage( nullptr, bp_leg_l, damage_instance( DT_ACID, rng( 1, 4 ) ) );
                u.deal_damage( nullptr, bp_leg_r, damage_instance( DT_ACID, rng( 1, 4 ) ) );
            } else {
                u.add_msg_player_or_npc(m_bad, _("The acid burns your legs and feet!"), _("The acid burns <npcname>s legs and feet!"));
                u.deal_damage( nullptr, bp_foot_l, damage_instance( DT_ACID, rng( 1, 3 ) ) );
                u.deal_damage( nullptr, bp_foot_r, damage_instance( DT_ACID, rng( 1, 3 ) ) );
                u.deal_damage( nullptr, bp_leg_l, damage_instance( DT_ACID, rng( 0, 2 ) ) );
                u.deal_damage( nullptr, bp_leg_r, damage_instance( DT_ACID, rng( 0, 2 ) ) );
            }
            u.check_dead_state();
            break;

        case fd_sap:
            //Sap causes the player to get sap disease, slowing them down.
            if ( u.in_vehicle ) {
                break; //sap does nothing to cars.
            }
            u.add_msg_player_or_npc(m_bad, _("The sap sticks to you!"), _("The sap sticks to <npcname>!"));
            u.add_effect("sap", cur->density() * 2);
            if (cur->density() == 1) {
                field_list_it = curfield.remove(fd_sap).first;
                get_submap_at(u.posx(), u.posy())->field_count--;
                continue;
            } else {
                cur->intensify(-1); //Use up sap.
            }
            break;

        case fd_sludge:
            u.add_msg_if_player(m_bad, _("The sludge is thick and sticky. You struggle to pull free."));
            u.moves -= cur->density() * 300;
            field_list_it = curfield.remove( fd_sludge ).first;
            get_submap_at(u.posx(), u.posy())->field_count--;
            break;

        case fd_fire:
            if( u.has_active_bionic("bio_heatsink") || u.is_wearing("rm13_armor_on") ||
                u.has_trait("M_SKIN2") ) {
                //heatsink, suit, or internal restructuring prevents ALL fire damage.
                break;
            }
            //Burn the player. Less so if you are in a car or ON a car.
            adjusted_intensity = cur->density();
            if( u.in_vehicle ) {
                if( inside ) {
                    adjusted_intensity -= 2;
                } else {
                    adjusted_intensity -= 1;
                }
            }
            {
                std::list<int> parts_burned;
                int burn_min = 0;
                int burn_max = 0;
                // first is for the player, second for the npc
                std::string burn_message[2];
                switch( adjusted_intensity ) {
                case 3:
                    burn_message[0] = _("You're set ablaze!");
                    burn_message[1] = _("<npcname> is set ablaze!");
                    burn_min = 4;
                    burn_max = 12;
                    parts_burned.push_back( bp_hand_l );
                    parts_burned.push_back( bp_hand_r );
                    parts_burned.push_back( bp_arm_l );
                    parts_burned.push_back( bp_arm_r );
                    // Only blasing fires set you ablaze.
                    u.add_effect("onfire", 5);
                    // Fallthrough intentional.
                case 2:
                    if( burn_message[0].empty() ) {
                        burn_message[0] = _("You're burning up!");
                        burn_message[1] = _("<npcname> is burning up!");
                        burn_min = 2;
                        burn_max = 9;
                    }
                    parts_burned.push_back( bp_torso );
                    // Fallthrough intentional.
                case 1:
                    if( burn_message[0].empty() ) {
                        burn_message[0] = _("You burn your legs and feet!");
                        burn_message[1] = _("<npcname> burns their legs and feet!");
                        burn_min = 1;
                        burn_max = 6;
                    }
                    parts_burned.push_back( bp_foot_l );
                    parts_burned.push_back( bp_foot_r );
                    parts_burned.push_back( bp_leg_l );
                    parts_burned.push_back( bp_leg_r );
                }
                if( u.is_on_ground() ) {
                    // Lying in the fire is BAAAD news, hits every body part.
                    burn_message[0] = _("Your whole body is burning!");
                    burn_message[1] = _("<npcname>s whole body is burning!");
                    parts_burned.clear();
                    for( int i = 0; i < num_bp; ++i ) {
                        parts_burned.push_back( i );
                    }
                }
                u.add_msg_player_or_npc( m_bad, burn_message[0].c_str(), burn_message[1].c_str() );
                for( auto part_burned : parts_burned ) {
                    u.deal_damage( nullptr, (enum body_part)part_burned,
                                      damage_instance( DT_HEAT, rng( burn_min, burn_max ) ) );
                }
                u.check_dead_state();
            }
            break;

        case fd_smoke:
            {
                if (!inside) {
                    //Get smoke disease from standing in smoke.
                    int density = cur->density();
                    int coughStr;
                    int coughDur;
                    if (density >= 3) {   // thick smoke
                        coughStr = 4;
                        coughDur = 15;
                    } else if (density == 2) {  // smoke
                        coughStr = 2;
                        coughDur = 7;
                    } else {    // density 1, thin smoke
                        coughStr = 1;
                        coughDur = 2;
                    }
                    u.add_env_effect("smoke", bp_mouth, coughStr, coughDur);
                }
            }
            break;

        case fd_tear_gas:
            //Tear gas will both give you teargas disease and/or blind you.
            if ((cur->density() > 1 || !one_in(3)) && (!inside || (inside && one_in(3))))
            {
                u.add_env_effect("teargas", bp_mouth, 5, 20);
            }
            if (cur->density() > 1 && (!inside || (inside && one_in(3))))
            {
                u.add_env_effect("blind", bp_eyes, cur->density() * 2, 10);
            }
            break;

        case fd_relax_gas:
            if ((cur->density() > 1 || !one_in(3)) && (!inside || (inside && one_in(3))))
            {
                u.add_env_effect("relax_gas", bp_mouth, cur->density() * 2, 3);
            }
            break;

        case fd_fungal_haze:
            if (!u.has_trait("M_IMMUNE") && (!inside || (inside && one_in(4))) ) {
                u.add_env_effect("fungus", bp_mouth, 4, 100, num_bp, true);
                u.add_env_effect("fungus", bp_eyes, 4, 100, num_bp, true);
            }
            break;

        case fd_dazzling:
            if (cur->density() > 1 || one_in(5)){
                u.add_env_effect("blind", bp_eyes, 10, 10);
            } else{
                u.add_env_effect("blind", bp_eyes, 2, 2);
            }
            break;

        case fd_toxic_gas:
            // Toxic gas at low levels poisons you.
            // Toxic gas at high levels will cause very nasty poison.
            {
                bool inhaled = false;
                if( cur->density() == 2 &&
                    (!inside || (cur->density() == 3 && inside)) ) {
                    inhaled = u.add_env_effect("poison", bp_mouth, 5, 30);
                } else if( cur->density() == 3 && !inside ) {
                    inhaled = u.add_env_effect("badpoison", bp_mouth, 5, 30);
                } else if( cur->density() == 1 && (!inside) ) {
                    inhaled = u.add_env_effect("poison", bp_mouth, 2, 20);
                }
                if( inhaled ) {
                    // player does not know how the npc feels, so no message.
                    u.add_msg_if_player(m_bad, _("You feel sick from inhaling the %s"), cur->name().c_str());
                }
            }
            break;

        case fd_nuke_gas:
            // Get irradiated by the nuclear fallout.
            // Changed to min of density, not 0.
            u.radiation += rng(cur->density(),
                                  cur->density() * (cur->density() + 1));
            if (cur->density() == 3) {
                u.add_msg_if_player(m_bad, _("This radioactive gas burns!"));
                u.hurtall(rng(1, 3), nullptr);
            }
            break;

        case fd_flame_burst:
            //A burst of flame? Only hits the legs and torso.
            if (inside) break; //fireballs can't touch you inside a car.
            if (!u.has_active_bionic("bio_heatsink") && !u.is_wearing("rm13_armor_on") &&
                !u.has_trait("M_SKIN2")) { //heatsink, suit, or Mycus fireproofing stops fire.
                u.add_msg_player_or_npc(m_bad, _("You're torched by flames!"), _("<npcname> is torched by flames!"));
                u.deal_damage( nullptr, bp_leg_l, damage_instance( DT_HEAT, rng( 2, 6 ) ) );
                u.deal_damage( nullptr, bp_leg_r, damage_instance( DT_HEAT, rng( 2, 6 ) ) );
                u.deal_damage( nullptr, bp_torso, damage_instance( DT_HEAT, rng( 4, 9 ) ) );
                u.check_dead_state();
            } else
                u.add_msg_player_or_npc(_("These flames do not burn you."), _("Those flames do not burn <npcname>."));
            break;

        case fd_electricity:
            if (u.has_artifact_with(AEP_RESIST_ELECTRICITY) || u.has_active_bionic("bio_faraday")) //Artifact or bionic stops electricity.
                u.add_msg_if_player(_("The electricity flows around you."));
            else if (u.worn_with_flag("ELECTRIC_IMMUNE")) //Artifact or bionic stops electricity.
                u.add_msg_if_player(_("Your armor safely grounds the electrical discharge."));
            else {
                u.add_msg_player_or_npc(m_bad, _("You're electrocuted!"), _("<npcname> is electrocuted!"));
                //small universal damage based on density.
                u.hurtall(rng(1, cur->density()), nullptr);
                if (one_in(8 - cur->density()) && !one_in(30 - u.str_cur)) {
                    //str of 30 stops this from happening.
                    u.add_msg_player_or_npc(m_bad, _("You're paralyzed!"), _("<npcname> is paralyzed!"));
                    u.moves -= rng(cur->density() * 150, cur->density() * 200);
                }
            }
            break;

        case fd_fatigue:
            //Teleports you... somewhere.
            if (rng(0, 2) < cur->density() && u.is_player() ) {
                // TODO: allow teleporting for npcs
                add_msg(m_bad, _("You're violently teleported!"));
                u.hurtall(cur->density(), nullptr);
                g->teleport();
            }
            break;

            //Why do these get removed???
        case fd_shock_vent:
            //Stepping on a shock vent shuts it down.
            field_list_it = curfield.remove( fd_shock_vent ).first;
            get_submap_at(u.posx(), u.posy())->field_count--;
            continue;

        case fd_acid_vent:
            //Stepping on an acid vent shuts it down.
            field_list_it = curfield.remove( fd_acid_vent ).first;
            get_submap_at(u.posx(), u.posy())->field_count--;
            continue;

        case fd_bees:
            // Player is immune to bees while underwater.
            if( !u.is_underwater() ) {
                int times_stung = 0;
                int density = cur->density();
                // If the bees can get at you, they cause steadily increasing pain.
                // TODO: Specific stinging messages.
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                times_stung += one_in(4) &&
                    u.add_env_effect( "stung", bp_torso, density, 90 );
                switch( times_stung ) {
                case 0:
                    // Woo, unscathed!
                    break;
                case 1:
                    u.add_msg_if_player( m_bad, _("The bees sting you!") );
                    break;
                case 2:
                case 3:
                    u.add_msg_if_player( m_bad, _("The bees sting you several times!") );
                    break;
                case 4:
                case 5:
                    u.add_msg_if_player( m_bad, _("The bees sting you many times!") );
                    break;
                case 6:
                case 7:
                case 8:
                default:
                    u.add_msg_if_player( m_bad, _("The bees sting you all over your body!") );
                    break;
                }
            }
            break;

        case fd_incendiary:
        // Mysterious incendiary substance melts you horribly.
            if (u.has_trait("M_SKIN2") || cur->density() == 1) {
                u.add_msg_player_or_npc(m_bad, _("The incendiary burns you!"), _("The incendiary burns <npcname>!"));
                u.hurtall(rng(1, 3), nullptr);
            } else {
                u.add_msg_player_or_npc(m_bad, _("The incendiary melts into your skin!"), _("The incendiary melts into <npcname>s skin!"));
                u.add_effect("onfire", 8);
                u.hurtall(rng(2, 6), nullptr);
            }
            break;

        default:
            //Suppress warnings
            break;
        }
        if (field_list_it != curfield.end()) {
            // It may have became the last one as a result of a field
            // being removed, in which case incrementing would make us
            // pass on by, so only increment if that's not the case
            ++field_list_it;
        }
    }

}

void map::creature_in_field( Creature &critter )
{
    auto m = dynamic_cast<monster *>( &critter );
    auto p = dynamic_cast<player *>( &critter );
    if( m != nullptr ) {
        monster_in_field( *m );
    } else if( p != nullptr ) {
        player_in_field( *p );
    }
}

void map::monster_in_field( monster &z )
{
    if (z.digging()) {
        return; // Digging monsters are immune to fields
    }
    field &curfield = get_field( z.posx(), z.posy() );

    int dam = 0;
    for( auto field_list_it = curfield.begin(); field_list_it != curfield.end(); ) {
        field_entry *cur = &*field_list_it;

        switch (cur->type()) {
        case fd_null:
        case fd_blood: // It doesn't actually do anything
        case fd_bile:  // Ditto
            break;

        case fd_web:
            if (!z.has_flag(MF_WEBWALK)) {
                z.add_effect("webbed", 1, num_bp, true, cur->density());
                field_list_it = curfield.remove( fd_web ).first;
                get_submap_at(z.posx(), z.posy())->field_count--;
                continue;
            }
            break;

        case fd_acid:
            if( !z.has_flag( MF_FLIES ) ) {
                if (cur->density() == 3) {
                    const int d = rng( 4, 10 ) + rng( 2, 8 );
                    z.deal_damage( nullptr, bp_torso, damage_instance( DT_ACID, d ) );
                } else {
                    const int d = rng( cur->density(), cur->density() * 4 );
                    z.deal_damage( nullptr, bp_torso, damage_instance( DT_ACID, d ) );
                }
                z.check_dead_state();
            }
            break;

        case fd_sap:
            z.moves -= cur->density() * 5;
            if (cur->density() == 1) {
                field_list_it = curfield.remove( fd_sap ).first;
                get_submap_at(z.posx(), z.posy())->field_count--;
                continue;
            } else {
                cur->intensify(-1);
            }
            break;

        case fd_sludge:
            if (!z.has_flag(MF_DIGS) && !z.has_flag(MF_FLIES) && !z.has_flag(MF_SLUDGEPROOF)) {
              z.moves -= cur->density() * 300;
              field_list_it = curfield.remove( fd_sludge ).first;
              get_submap_at(z.posx(), z.posy())->field_count--;
            }
            break;

            // MATERIALS-TODO: Use fire resistance
        case fd_fire:
            if ( z.made_of("flesh") || z.made_of("hflesh") || z.made_of("iflesh") ) {
                dam += 3;
            }
            if (z.made_of("veggy")) {
                dam += 12;
            }
            if (z.made_of("paper") || z.made_of(LIQUID) || z.made_of("powder") ||
                z.made_of("wood")  || z.made_of("cotton") || z.made_of("wool")) {
                dam += 20;
            }
            if (z.made_of("stone") || z.made_of("kevlar") || z.made_of("steel")) {
                dam += -20;
            }
            if (z.has_flag(MF_FLIES)) {
                dam -= 15;
            }

            if (cur->density() == 1) {
                dam += rng(2, 6);
            } else if (cur->density() == 2) {
                dam += rng(6, 12);
                if (!z.has_flag(MF_FLIES)) {
                    z.moves -= 20;
                    if (!z.made_of(LIQUID) && !z.made_of("stone") && !z.made_of("kevlar") &&
                        !z.made_of("steel") && !z.has_flag(MF_FIREY)) {
                        z.add_effect("onfire", rng(3, 8));
                    }
                }
            } else if (cur->density() == 3) {
                dam += rng(10, 20);
                if (!z.has_flag(MF_FLIES) || one_in(3)) {
                    z.moves -= 40;
                    if (!z.made_of(LIQUID) && !z.made_of("stone") && !z.made_of("kevlar") &&
                        !z.made_of("steel") && !z.has_flag(MF_FIREY)) {
                        z.add_effect("onfire", rng(8, 12));
                    }
                }
            }
            // Drop through to smoke no longer needed as smoke will exist in the same square now,
            // this would double apply otherwise.
            break;

        case fd_smoke:
            if (!z.has_flag(MF_NO_BREATHE)) {
                if (cur->density() == 3) {
                    z.moves -= rng(10, 20);
                }
                if (z.made_of("veggy")) { // Plants suffer from smoke even worse
                    z.moves -= rng(1, cur->density() * 12);
                }
            }
            break;

        case fd_tear_gas:
            if ((z.made_of("flesh") || z.made_of("hflesh") || z.made_of("veggy") || z.made_of("iflesh")) &&
                !z.has_flag(MF_NO_BREATHE)) {
                if (cur->density() == 3) {
                    z.add_effect("stunned", rng(10, 20));
                    dam += rng(4, 10);
                } else if (cur->density() == 2) {
                    z.add_effect("stunned", rng(5, 10));
                    dam += rng(2, 5);
                } else {
                    z.add_effect("stunned", rng(1, 5));
                }
                if (z.made_of("veggy")) {
                    z.moves -= rng(cur->density() * 5, cur->density() * 12);
                    dam += cur->density() * rng(8, 14);
                }
                if (z.has_flag(MF_SEES)) {
                     z.add_effect("blind", cur->density() * 8);
                }
            }
            break;

        case fd_relax_gas:
            if ((z.made_of("flesh") || z.made_of("hflesh") || z.made_of("veggy") || z.made_of("iflesh")) &&
                !z.has_flag(MF_NO_BREATHE)) {
                z.add_effect("stunned", rng(cur->density() * 4, cur->density() * 8));
            }
            break;

        case fd_dazzling:
            if (z.has_flag(MF_SEES)) {
                z.add_effect("blind", cur->density() * 12);
                z.add_effect("stunned", cur->density() * rng(5, 12));
            }
            break;

        case fd_toxic_gas:
            if(!z.has_flag(MF_NO_BREATHE)) {
                dam += cur->density();
                z.moves -= cur->density();
            }
            break;

        case fd_nuke_gas:
            if(!z.has_flag(MF_NO_BREATHE)) {
                if (cur->density() == 3) {
                    z.moves -= rng(60, 120);
                    dam += rng(30, 50);
                } else if (cur->density() == 2) {
                    z.moves -= rng(20, 50);
                    dam += rng(10, 25);
                } else {
                    z.moves -= rng(0, 15);
                    dam += rng(0, 12);
                }
                if (z.made_of("veggy")) {
                    z.moves -= rng(cur->density() * 5, cur->density() * 12);
                    dam *= cur->density();
                }
            }
            break;

            // MATERIALS-TODO: Use fire resistance
        case fd_flame_burst:
            if (z.made_of("flesh") || z.made_of("hflesh") || z.made_of("iflesh")) {
                dam += 3;
            }
            if (z.made_of("veggy")) {
                dam += 12;
            }
            if (z.made_of("paper") || z.made_of(LIQUID) || z.made_of("powder") ||
                z.made_of("wood")  || z.made_of("cotton") || z.made_of("wool")) {
                dam += 50;
            }
            if (z.made_of("stone") || z.made_of("kevlar") || z.made_of("steel")) {
                dam += -25;
            }
            dam += rng(0, 8);
            z.moves -= 20;
            break;

        case fd_electricity:
            dam += rng(1, cur->density());
            if (one_in(8 - cur->density())) {
                z.moves -= cur->density() * 150;
            }
            break;

        case fd_fatigue:
            if (rng(0, 2) < cur->density()) {
                dam += cur->density();
                int tries = 0;
                int newposx, newposy;
                do {
                    newposx = rng(z.posx() - SEEX, z.posx() + SEEX);
                    newposy = rng(z.posy() - SEEY, z.posy() + SEEY);
                    tries++;
                } while (move_cost(newposx, newposy) == 0 && tries != 10);

                if (tries == 10) {
                    z.die_in_explosion( nullptr );
                } else {
                    int mon_hit = g->mon_at(newposx, newposy);
                    if (mon_hit != -1) {
                        if (g->u.sees(z)) {
                            add_msg(_("The %s teleports into a %s, killing them both!"),
                                       z.name().c_str(), g->zombie(mon_hit).name().c_str());
                        }
                        g->zombie( mon_hit ).die_in_explosion( &z );
                    } else {
                        z.setpos(newposx, newposy);
                    }
                }
            }
            break;

        case fd_incendiary:
            // MATERIALS-TODO: Use fire resistance
            if ( z.made_of("flesh") || z.made_of("hflesh") || z.made_of("iflesh") ) {
                dam += 3;
            }
            if (z.made_of("veggy")) {
                dam += 12;
            }
            if (z.made_of("paper") || z.made_of(LIQUID) || z.made_of("powder") ||
                z.made_of("wood")  || z.made_of("cotton") || z.made_of("wool")) {
                dam += 20;
            }
            if (z.made_of("stone") || z.made_of("kevlar") || z.made_of("steel")) {
                dam += -5;
            }

            if (cur->density() == 1) {
                dam += rng(2, 6);
            } else if (cur->density() == 2) {
                dam += rng(6, 12);
                z.moves -= 20;
                if (!z.made_of(LIQUID) && !z.made_of("stone") && !z.made_of("kevlar") &&
                !z.made_of("steel") && !z.has_flag(MF_FIREY)) {
                    z.add_effect("onfire", rng(8, 12));
                }
            } else if (cur->density() == 3) {
                dam += rng(10, 20);
                z.moves -= 40;
                if (!z.made_of(LIQUID) && !z.made_of("stone") && !z.made_of("kevlar") &&
                !z.made_of("steel") && !z.has_flag(MF_FIREY)) {
                        z.add_effect("onfire", rng(12, 16));
                }
            }
            break;

        default:
            //Suppress warnings
            break;
        }

        if (field_list_it != curfield.end()) {
            // It may have became the last one as a result of a field
            // being removed, in which case incrementing would make us
            // pass on by, so only increment if that's not the case
            ++field_list_it;
        }
    }
    if (dam > 0) {
        z.apply_damage( nullptr, bp_torso, dam );
        z.check_dead_state();
    }
}

int field_entry::set_density_(const int new_density)
{
    if(new_density > 3) {
        density_ = 3;
    } else if (new_density < 1) {
        density_ = 1;
        is_alive_ = false;
    } else {
        density_ = new_density;
    }

    return density_;
}

field::iterator field::find_(field_id const id)
{
    return std::find_if(begin(), end(), [&](const_reference fld) {
        return fld.type() == id;
    });
}

field::const_iterator field::find_(field_id const id) const
{
    return const_cast<field*>(this)->find_(id);
}

field_entry* field::find(field_id const id)
{
    auto const it = find_(id);
    return it != end() ? &*it : nullptr;
}

field_entry const* field::find(field_id const id) const
{
    return const_cast<field*>(this)->find(id);
}

bool field::add(const field_id id, const int new_density, const int new_age)
{
    if (get_field_def(id).priority >= get_field_def(draw_symbol).priority) {
        draw_symbol = id;
    }

    if (field_entry *entry = find(id)) {
        //Already exists, but lets update it. This is tentative.
        //TODO age??
        entry->intensify(new_density);
        return false;
    }

    field_list.emplace_back(id, new_density, new_age);
    
    if (id == fd_slime) {
        has_scent_ = true;
    } else if (id == fd_fire) {
        has_fire_ = true;
    }

    field_t const &fld = get_field_def(id);
    if (fld.luminance[2] || fld.luminance[1] || fld.luminance[0]) {
        ++luminous_count_;
    }

    if (fld.transparency[2] < 1.0f || fld.transparency[1] < 1.0f && fld.transparency[0] < 1.0f) {
        ++opaque_count_;
    }
   
    return true;
}

std::pair<field::iterator, bool> field::remove(field_id const id)
{
    auto it = find_(id);
    if (it == end()) {
        return std::make_pair(it, false);
    }

    field_list.erase(it++);

    draw_symbol = fd_null;
    int const priority = get_field_def(draw_symbol).priority;
    for (auto const &fld : field_list) {
        if (get_field_def(fld.type()).priority >= priority) {
            draw_symbol = fld.type();
        }
    }

    if (id == fd_slime) {
        has_scent_ = false;
    } else if (id == fd_fire) {
        has_fire_ = false;
    }

    field_t const &fld = get_field_def(id);
    if (fld.luminance[2] || fld.luminance[1] || fld.luminance[0]) {
        --luminous_count_;
    }

    if (fld.transparency[2] < 1.0f || fld.transparency[1] < 1.0f && fld.transparency[0] < 1.0f) {
        --opaque_count_;
    }

    return std::make_pair(it, true);
}

bool field::is_dangerous() const
{
    return std::any_of(begin(), end(), [](const_reference ref) {
        return ref.is_dangerous();
    });
}

bool field::is_dangerous(Creature const &subject) const
{
    return std::any_of(begin(), end(), [&](const_reference ref) {
        return ref.is_dangerous(subject);
    });
}

float field::transparency() const
{
    float value = 1.0f;
    for (auto const &fld : field_list) {
        if (value == 0.0f) {
            return 0.0f;
        }

        field_t const &f = get_field_def(fld.type());
        value *= f.transparency[fld.density() - 1];
    }

    return value;
}

void field::decay(int const amount)
{
    for (auto &fld : field_list) {
        auto const decay = static_cast<int>(fld.definition().decay);
        if (decay <= 0) {
            continue;
        }

        fld.decay(amount / decay);
    }
}

void field::write(JsonOut &jout)
{
    jout.start_array();
    for (auto const &fld : field_list) {
        // We don't seem to have a string identifier for fields anywhere.
        jout.write(fld.type());
        jout.write(fld.density());
        jout.write(fld.age());
    }
    jout.end_array();
}

float field::luminance() const
{
    float max = 0.0f;
    for (auto const &fld : field_list) {
        field_t const& f = get_field_def(fld.type());
        max = std::max(max, f.luminance[fld.density() - 1]);
    }

    return max;
}

bool field_entry::is_dangerous(Creature const &subject) const
{
    switch (type()) {
    case fd_smoke:
        return subject.get_env_resist(bp_mouth) < 7;
    case fd_tear_gas:
    case fd_toxic_gas:
    case fd_gas_vent:
    case fd_relax_gas:
        return subject.get_env_resist(bp_mouth) < 15;
    case fd_fungal_haze:
        return !subject.has_trait("M_IMMUNE") && (
                    subject.get_env_resist(bp_mouth) < 15 ||
                    subject.get_env_resist(bp_eyes)  < 15
                );
    default:
        return is_dangerous();
    }
}
