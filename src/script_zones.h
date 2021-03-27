//
// Created by sim7 on 2021-03-27.
//

#ifndef GIT_MAP_ZONES_H
#define GIT_MAP_ZONES_H

#define MAX_ZONE_RECORDS        32
#define INVALID_SCRIPT_ZONE     &gameadd.script_zone_records[0]

enum ScriptZoneSwapDirection
{
    SZS_Forward = 1,
    SZS_Backward,
    SZS_Shuffle
};


struct ScriptZoneRecord
{
    int zone_id;
    int next_idx, prev_idx;
    unsigned char min_x, min_y; // In slabs
    unsigned char hwidth, hheight;
};

struct ScriptZoneRecord* find_script_zone(int zone_id);
void swap_script_zone(int zone_id, enum ScriptZoneSwapDirection dir);
struct ScriptZoneRecord* add_script_zone();
struct ScriptZoneRecord* get_script_zone(int zone_id);
int script_zone_id(struct ScriptZoneRecord* zone);

#endif //GIT_MAP_ZONES_H
