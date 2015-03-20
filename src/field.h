#ifndef FIELD_H
#define FIELD_H

#include <vector>
#include <string>
#include <map>

class Creature;
class JsonOut;

/*
 * Used to store field effects metadata. Not used to store a field, just queried to find out
 * specifics of an existing field.
 */
struct field_t {
    // The divisor for the rate at which different categories of fields decay.
    enum class decay_type : int {
        none   = 0,
        fire   = 1,
        liquid = 3,
        gas    = 5,
    };

    // id used for tileset and for serializing
    // should be the same as the entry in field_id below (e.g. "fd_fire").
    std::string id;
    
    // The symbol to draw for this field. Note that some are reserved like * and %.
    // You will have to check the draw function for specifics.
    char sym;

    // Inferior numbers have lower priority.
    //-1 is ?
    // 0 is "ground" (splatter),
    // 2 is "on the ground",
    // 4 is "above the ground" (fire),
    // 6 is reserved for furniture,
    // 8 is "in the air" (smoke).
    int priority;

    // Controls, albeit randomly, how long a field of a given type will last before going down in density.
    int halflife; // In turns

    decay_type decay;

    // The display name of the given density (ie: light smoke, smoke, heavy smoke)
    std::string name[3];

    // The color the field will be drawn as on the screen, by density.
    nc_color color[3];

    // Dangerous tiles ask you before you step in them.
    bool dangerous[3];      

    // [0, 1] where 0 indicates fully opaque, and 1 indicates fully transparent.
    float transparency[3];

    // [0, inf] the light emitted by the field, if any.
    float luminance[3];
};

// The master list of id's for a field.
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
    field_entry(field_id const type, int const density, int const age) noexcept
      : type_(type), density_(density), age_(age), is_alive_(true)
    {
    }

    bool update();
    void decay(int delta);
    
    // modify the current density (intensity) by delta
    bool intensify(int delta);
    
    // kill the field and mark it as needing to be removed
    void nullify();
    
    // set the field to maximum density (intensity)
    void maximize();

    int move_cost() const noexcept {
        return 0;
    }

    field_id type() const noexcept {
        return type_;
    }

    int density() const noexcept {
        return density_;
    }

    //Returns the age (usually turns to live) of the current field entry.
    int age() const noexcept {
        return age_;
    }

    bool is_dangerous() const noexcept {
        return definition().dangerous[density_ - 1];
    }

    bool is_dangerous(Creature const &subject) const;

    //Returns the display name of the current field given its current density.
    //IE: light smoke, smoke, heavy smoke
    std::string const& name() const {
        return definition().name[density_ - 1];
    }

    //Returns true if this is an active field, false if it should be removed.
    bool is_alive() const noexcept {
        return is_alive_;
    }

    field_t const& definition() const {
        return get_field_def(type_);
    }
private:
    int set_density_(int new_density);

    field_id type_     = fd_null; // The field identifier.
    int      density_  = 1;       // The density, or intensity (higher is stronger), of the field entry.
    int      age_      = 0;       // The age, or time to live, of the field effect. 0 is permanent.
    bool     is_alive_ = false;   // True if this is an active field, false if it should be destroyed next check.
};

/**
 * A variable sized collection of field entries on a given map square.
 * It contains one (at most) entry of each field type (e.g. one smoke entry and one
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

    //! The field entry corresponding to field_id if present, nullptr otherwise.
    field_entry*       find(field_id id);
    field_entry const* find(field_id id) const;

    /**
     * Inserts the given field_id into the field list for a given tile if it does not already exist.
     * If you wish to modify an already existing field use find and modify the result.
     * The density is added to an existing field entry, but the age is only used for newly added entries.
     * @return false if the field_id already exists, true otherwise.
     */
    bool add(field_id id, int new_density = 1, int new_age = 0);

    /**
     * Removes the field entry with a type equal to the field_id parameter.
     * @return The iterator to the field after the removed on.
     * The result might be the @ref end iterator.
     */
    std::pair<iterator, bool> remove(field_id id);

    bool   empty() const noexcept { return field_list.empty(); }
    size_t size()  const noexcept { return field_list.size(); }

    iterator       begin()       { return field_list.begin(); }
    const_iterator begin() const { return field_list.begin(); }
    iterator       end()         { return field_list.end(); }
    const_iterator end()   const { return field_list.end(); }

    void clear() {
        field_list.clear();
    }

    //! Returns the id of the field that should be drawn.
    field_id symbol() const noexcept {
        return draw_symbol;
    }

    //! total move cost from all fields; TODO: always 0 currently.
    int move_cost() const noexcept { return 0; }

    //! true if there is at least one dangerous field.
    bool is_dangerous() const;
    bool is_dangerous(Creature const &subject) const;

    std::string danger_description(Creature const &subject) const;

    //! total degree of tranparency for all fields; [0, 1]
    float transparency() const;

    //! decay alls fields by the given amount of time.
    void decay(int amount);

    //! serialization helper
    void write(JsonOut &jout);

    //! total luminance from all fields; [0, inf]
    float luminance() const;

    //! true if there is a field that generates scent.
    bool has_scent() const noexcept {
        return has_scent_;
    }

    // true if there is a fire field.
    bool has_fire() const noexcept {
        return has_fire_;
    }

    //! true if there are no fields which are not totally transparent.
    bool is_transparent() const noexcept {
        return opaque_count_ == 0;
    }

    //! true if there are light emitting fields.
    bool has_luminous() const noexcept {
        return luminous_count_ > 0;
    }
private:
    iterator       find_(field_id id);
    const_iterator find_(field_id id) const;

    container_t field_list;

    // Draw_symbol currently is equal to the last field added to the square.
    // You can modify this behavior in the class functions if you wish.
    field_id draw_symbol = fd_null;
    bool has_scent_      = false;
    bool has_fire_       = false;
    char opaque_count_   = 0;
    char luminous_count_ = 0;
};

#endif
