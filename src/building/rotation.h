#ifndef BUILDING_ROTATION_H
#define BUILDING_ROTATION_H

void building_rotation_update_road_orientation(void);
int building_rotation_get_rotation(void);
int building_rotation_get_road_orientation(void);
int building_rotation_get_building_variant(void);

void building_rotation_force_two_orientations(void);
int building_rotation_get_building_orientation(int);
int building_rotation_get_delta_with_rotation(int default_delta);
void building_rotation_get_offset_with_rotation(int offset, int rotation, int *x, int *y);

int building_rotation_get_corner(int rot);

void building_rotation_rotate_by_hotkey(void);
void building_rotation_variant_by_hotkey(void);
void building_rotation_reset_rotation(void);

#endif // BUILDING_ROTATION_H