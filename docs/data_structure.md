## Structure

*Map structure*

 * Slabs
 * Subtiles
 * Columns
 * Cubes

*Things*

Every thing has
  * class_id
    * Empty - Nothing
    * Object - Real
    * Shot
    * EffectElem
    * DeadCreature
    * Creature
    * Effect
    * EffectGen
    * Trap
    * Door
    * Unkn10 - Unused?
    * Unkn11 - Unused?
    * AmbientSnd
    * CaveIn
  * owner
  * model

Everything is a thing
  * `struct Thing`

Creatures have additional structure
  * `struct CreatureControl`

### Slabs

Slab is one of a map.
Most important attributes are:
  * "type of a room"
  * "ownership"

AI "pathfinding" is slab-based.

Some slabs settings are hardcoded other settings are stored at `terrain.cfg`

* Slb_ID - used for "altering walls" so slabs with same Slb_ID are not altered \
  0 - solid environment (dirt, rocks, gold etc, Slab50)
  1 - ? (path, bridge guardpost)
  2 - dungeon (claimed path, doors)
  3 - lava
  4 - water
  5 - entrance center and wall
  ... other rooms and walls share same id

Doors are implemented as replacing slabs with some delay

For each subtile there is a mapping of columns and objects to place whenever such tile is placed on map. 

I.e. torches on wall, chandeliers on treasure room etc

That mapping is depends on tiles next to one placed

### Subtiles (aka STLs)


Subtile is minimal 2d part of a map.
Vision is STL-based

### Columns

Column is a stack of cubes for each subtile

### Cubes

Cube is one textured cube.
Mapped to texutres at `cubes.cfg`

There are
  * animated textures (i.e Magic door == Center of a temple)
  * static textures (simple walls)
