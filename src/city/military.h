#pragma once

void city_military_clear_legionary_legions();
void city_military_add_legionary_legion();
int city_military_has_legionary_legions();

int city_military_total_legions();
int city_military_total_soldiers();
int city_military_empire_service_legions();
void city_military_clear_empire_service_legions();

void city_military_update_totals();

int city_military_is_native_attack_active();
void city_military_start_native_attack();
void city_military_decrease_native_attack_duration();

void city_military_determine_distant_battle_city();
int city_military_distant_battle_city();
int city_military_distant_battle_city_is_roman();

int city_military_distant_battle_enemy_strength();

void city_military_dispatch_to_distant_battle(int roman_strength);
int city_military_distant_battle_kingdome_army_is_traveling();
int city_military_distant_battle_kingdome_army_is_traveling_forth();
int city_military_distant_battle_kingdome_army_is_traveling_back();

int city_military_distant_battle_enemy_months_traveled();
int city_military_distant_battle_kingdome_months_traveled();

void city_military_init_distant_battle(int enemy_strength);
int city_military_has_distant_battle();
int city_military_months_until_distant_battle();

void city_military_process_distant_battle();
