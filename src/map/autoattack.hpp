#ifndef AUTOATTACK_HPP
#define AUTOATTACK_HPP

#include <common/mmo.hpp>
#include <map/map.hpp>
#include <map/pc.hpp>

void auto_attack_iterate(map_session_data *sd);
static int buildin_autoattack_sub(block_list *bl, va_list ap);
void resethotkey(int slot, map_session_data *sd);
void process_self_buffs(map_session_data *sd);
void process_self_heal(map_session_data *sd);
void process_auto_sit(map_session_data *sd);
void process_teleport(map_session_data *sd);
void process_attack(map_session_data *sd);
void process_random_walk(map_session_data *sd);
void process_dead(map_session_data *sd);
void disable_auto_attack(map_session_data *sd);
void reset_route(map_session_data *sd);
void recalculate_route(map_session_data *sd, int target_x, int target_y);
bool has_more_target(map_session_data *sd);

struct s_hotkey_buff
{
    int hotkey_pos;
    int SCE;
};

struct s_ac_buff
{
    int id;
    int SCE;
};

#endif /* AUTOATTACK_HPP */
