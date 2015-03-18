#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <string>
#include <map>

class Creature;
class JsonOut;

/*
struct field_t
Used to store the master field effects list metadata. Not used to store a field, just queried to find out specifics
of an existing field.
*/
struct field_t {
    enum class decay_type : int {
        none   = 0,
        fire   = 1,
        liquid = 3,
        gas    = 5,
    };

    // internal ident, used for tileset and for serializing,
    // should be the same as the entry in field_id below (e.g. "fd_fire").
    std::string id;
    
    // The symbol to draw for this field.
    // Note that some are reserved like * and %.
    // You will have to check the draw function for specifics.
    char sym;

    // Inferior numbers have lower priority.
    //-1 is ?
    // 0 is "ground" (splatter),
    // 2 is "on the ground",
    // 4 is "above the ground" (fire),
    // 6 is reserved for furniture,
    // and 8 is "in the air" (smoke).
    int priority;

    //Controls, albeit randomly, how long a field of a given type will last before going down in density.
    int halflife; // In turns

    decay_type decay;

    // The display name of the given density (ie: light smoke, smoke, heavy smoke)
    std::string name[3];

    nc_color color[3]; //The color the field will be drawn as on the screen, by density.

    //Dangerous tiles ask you before you step in them.
    bool dangerous[3];      

    /*
    If not 0, does not invoke a check to block line of sight. If false, may block line of sight.
    Note that this does nothing by itself. You must go to the code block in lightmap.cpp and modify
    transparancy code there with a case statement as well!
    */
    float transparency[3];

    float luminance[3];
};

//The master list of id's for a field, corresponding to the fieldlist array.
enum field_id : int {
    fd_null,
    fd_blood,
    fd_bile,
    fd_gibs_flesh,
    fd_gibs_veggy,
    fd_web,
    fd_slime,
    fd_acid,
    fd_sap,
    fd_sludge,
    fd_fire,
    fd_rubble,
    fd_smoke,
    fd_toxic_gas,
    fd_tear_gas,
    fd_nuke_gas,
    fd_gas_vent,
    fd_fire_vent,
    fd_flame_burst,
    fd_electricity,
    fd_fatigue,
    fd_push_items,
    fd_shock_vent,
    fd_acid_vent,
    fd_plasma,
    fd_laser,
    fd_spotlight,
    fd_dazzling,
    fd_blood_veggy,
    fd_blood_insect,
    fd_blood_invertebrate,
    fd_gibs_insect,
    fd_gibs_invertebrate,
    fd_cigsmoke,
    fd_weedsmoke,
    fd_cracksmoke,
    fd_methsmoke,
    fd_bees,
    fd_incendiary,
    fd_relax_gas,
    fd_fungal_haze,
    fd_hot_air1,
    fd_hot_air2,
    fd_hot_air3,
    fd_hot_air4,
    num_fields
};

field_t const& get_field_def(field_id id);

/**
 * Returns the field_id of the field whose ident (field::id) matches the given ident.
 * Returns fd_null (and prints a debug message!) if the field ident is unknown.
 * Never returns num_fields.
 */
field_id field_from_ident(const std::string &field_ident);

/**
 * An active or passive effect existing on a tile.
 * Each effect can vary in intensity (density) and age (usually used as a time to live).
 */
class field_entry {
public:
    field_entry() = default;
    field_entry(field_id const type, int const density, int const age)
      : type(type), density(density), age(age), is_alive(true)
    {
    }

    //returns the move cost of this field
    int move_cost() const;

    //Returns the field_id of the current field entry.
    field_id getFieldType() const;

    //Returns the current density (aka intensity) of the current field entry.
    int getFieldDensity() const;

    //Returns the age (usually turns to live) of the current field entry.
    int getFieldAge() const;

    //Allows you to modify the field_id of the current field entry.
    //This probably shouldn't be called outside of field::replaceField, as it
    //breaks the field drawing code and field lookup
    field_id setFieldType(field_id new_field_id);

    //Allows you to modify the density of the current field entry.
    int setFieldDensity(int new_density);

    //Allows you to modify the age of the current field entry.
    int setFieldAge(int new_age);

    //Returns if the current field is dangerous or not.
    bool is_dangerous() const {
        return get_field_def(type).dangerous[density - 1];
    }

    bool is_dangerous(Creature const &subject) const;

    //Returns the display name of the current field given its current density.
    //IE: light smoke, smoke, heavy smoke
    std::string const& name() const {
        return get_field_def(type).name[density - 1];
    }

    //Returns true if this is an active field, false if it should be removed.
    bool isAlive() const {
        return is_alive;
    }

    field_t const& definition() const {
        return get_field_def(type);
    }
private:
    field_id type     = fd_null; // The field identifier.
    int      density  = 1;       // The density, or intensity (higher is stronger), of the field entry.
    int      age      = 0;       // The age, or time to live, of the field effect. 0 is permanent.
    bool     is_alive = false;   // True if this is an active field, false if it should be destroyed next check.
};

/**
 * A variable sized collection of field entries on a given map square.
 * It contains one (at most) entry of each field type (e. g. one smoke entry and one
 * fire entry, but not two fire entries).
 * Use @ref find to get the field entry of a specific type, or iterate over
 * all entries via @ref begin and @ref end (allows range based iteration).
 * There is @ref symbol to specific which field should be drawn on the map.
*/
class field {
public:
    using container_t     = std::vector<field_entry>;
    using iterator        = container_t::iterator;
    using const_iterator  = container_t::const_iterator;
    using const_reference = container_t::const_reference;
    using reference       = container_t::reference;

    field() = default;

    /**
     * Returns a field entry corresponding to the field_id parameter passed in.
     * If no fields are found then nullptr is returned.
     */
    field_entry*       find(field_id id);
    field_entry const* find(field_id id) const;

    /**
     * Inserts the given field_id into the field list for a given tile if it does not already exist.
     * If you wish to modify an already existing field use find and modify the result.
     * Density defaults to 1, and age to 0 (permanent) if not specified.
     * The density is added to an existing field entry, but the age is only used for newly added entries.
     * @return false if the field_id already exists, true otherwise.
     * Function: addfield
     * Inserts the given field_id into the field list for a given tile if it does not already exist.
     * Returns false if the field_id already exists, true otherwise.
     * If the field already exists, it will return false BUT it will add the density/age to the current values for upkeep.
     * If you wish to modify an already existing field use find and modify the result.
     * Density defaults to 1, and age to 0 (permanent) if not specified.
    */
    bool add(field_id id, int new_density = 1, int new_age = 0);

    /**
     * Removes the field entry with a type equal to the field_id parameter.
     * @return The iterator to the field after the removed on.
     * The result might be the @ref end iterator.
     */
    iterator remove(field_id id);

    bool empty() const noexcept { return field_list.empty(); }

    //Returns the number of fields existing on the current tile.
    size_t size() const noexcept { return field_list.size(); }

    iterator       begin()       { return field_list.begin(); }
    const_iterator begin() const { return field_list.begin(); }

    iterator       end()       { return field_list.end(); }
    const_iterator end() const { return field_list.begin(); }

    void clear() { field_list.clear(); }

    /**
     * Returns the id of the field that should be drawn.
     */
    field_id symbol() const noexcept {
        return draw_symbol;
    }

    //iterator replace(field_id old_field, field_id new_field);

    /**
     * Returns the total move cost from all fields.
     * TODO: always 0 currently
     */
    int move_cost() const noexcept { return 0; }

    bool is_dangerous() const;
    bool is_dangerous(Creature const &subject) const;

    float transparency() const;

    void decay(int amount);

    void write(JsonOut &jout);

    float luminance() const;

    bool has_scent() const noexcept {
        return has_scent_;
    }

    bool has_fire() const noexcept {
        return has_fire_;
    }

    bool is_transparent() const noexcept {
        return opaque_count_ == 0;
    }

    bool has_luminous() const noexcept {
        return luminous_count_ > 0;
    }
private:
    iterator       find_(field_id id);
    const_iterator find_(field_id id) const;

    // A pointer lookup table of all field effects on the current tile.
    // Draw_symbol currently is equal to the last field added to the square.
    // You can modify this behavior in the class functions if you wish.
    container_t field_list;
    field_id draw_symbol = fd_null;
    bool has_scent_      = false;
    bool has_fire_       = false;
    char opaque_count_   = 0;
    char luminous_count_ = 0;
};

#endif
