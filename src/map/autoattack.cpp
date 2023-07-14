#include <map/autoattack.hpp>
#include <map/npc.hpp>
#include <common/showmsg.hpp>
#include <map/map.hpp>
#include "battle.hpp"
#include <ctime>

using namespace std;

// static char atcmd_output[CHAT_SIZE_MAX]; ??

s_hotkey_buff hotkey_buffs[5] = {
    {11, SC_BLESSING},
    {12, SC_INCREASEAGI},
    {13, SC_ASPDPOTION0},
    {14, SC_ASPDPOTION1},
    {15, SC_ASPDPOTION2}};

s_ac_buff ac_buffs[7] = {
    {AB_CLEMENTIA, SC_BLESSING},
    {AL_BLESSING, SC_BLESSING},
    {AB_CANTO, SC_INCREASEAGI},
    {AL_INCAGI, SC_INCREASEAGI},
    {PR_MAGNIFICAT, SC_MAGNIFICAT},
    {AB_PRAEFATIO, SC_KYRIE},
    {PR_KYRIE, SC_KYRIE}};

int heal_skills[5] = {
    CD_REPARATIO,
    CD_DILECTIO_HEAL,
    AL_HEAL,
    AB_CHEAL,
    AB_HIGHNESSHEAL};

// main loop
void auto_attack_iterate(map_session_data *sd)
{
    if (map_getmapflag(sd->bl.m, MF_NOAUTOATTACK) && sd->state.auto_attack.enabled)
    {
        clif_displaymessage(sd->fd, "Auto Attack: Sorry Auto Attack cannot be used on this map.");
        disable_auto_attack(sd);
        return;
    }

    if (!sd->state.auto_attack.enabled || pc_isdead(sd) || pc_is90overweight(sd) || sd->vd.dead_sit == 2)
    {
        disable_auto_attack(sd);
        return;
    }

    process_dead(sd);
    process_self_heal(sd);
    process_self_buffs(sd);
    process_auto_sit(sd);
    process_attack(sd);
    process_teleport(sd);
    if (has_more_target(sd))
        return;
    process_random_walk(sd);
}

void process_dead(map_session_data *sd)
{
    // ShowMessage("process_dead\n");
    // return to saveMap or disable?
}

// use pots / heal itself
void process_self_heal(map_session_data *sd)
{
    // ShowMessage("process_self_heal\n");

    // HP
    short hp_percent = (sd->battle_status.hp * 100) / sd->battle_status.max_hp;

    if ((sd->state.auto_attack.use_potion_hp_min_percent > 0 && sd->state.auto_attack.use_potion_hp_min_percent < 100) && hp_percent < sd->state.auto_attack.use_potion_hp_min_percent)
    {
        // acolyte and evolution heal skills
        if ((sd->class_ & MAPID_FOURTHMASK) >= MAPID_ACOLYTE)
        {
            if ((hp_percent >= sd->state.auto_attack.use_potion_hp_min_percent))
                return;

            for (int i = 0; i < 5; i++)
            {
                int skill_level = pc_checkskill(sd, heal_skills[i]);
                if (skill_level > 0)
                {
                    if (!skill_isNotOk(heal_skills[i], sd))
                    {
                        unit_skilluse_id(&sd->bl, sd->bl.id, 5268, pc_checkskill(sd, heal_skills[i]));
                        return;
                    }
                }
            }
        }

        if (!sd->sc.cant.consume)
        {
            if (sd->status.hotkeys[9].type == 0 && sd->status.hotkeys[9].id && hp_percent <= sd->state.auto_attack.use_potion_hp_min_percent) // HP Potion
            {
                int item_index = pc_search_inventory(sd, sd->status.hotkeys[9].id);
                if (item_index >= 0 && sd->item_delay[sd->status.hotkeys[9].id].tick <= 0)
                {
                    pc_useitem(sd, item_index);
                }
            }
        }
    }

    // SP
    short sp_percent = (sd->battle_status.sp * 100) / sd->battle_status.max_sp;

    if ((sd->state.auto_attack.use_potion_sp_min_percent > 0 && sd->state.auto_attack.use_potion_sp_min_percent < 100) && sp_percent < sd->state.auto_attack.use_potion_sp_min_percent)
    {
        if (!sd->sc.cant.consume)
        {
            if (sd->status.hotkeys[10].type == 0 && sd->status.hotkeys[10].id && sp_percent <= sd->state.auto_attack.use_potion_sp_min_percent) // SP Potion
            {
                int item_index = pc_search_inventory(sd, sd->status.hotkeys[10].id);
                if (item_index >= 0 && sd->item_delay[sd->status.hotkeys[10].id].tick <= 0)
                {
                    pc_useitem(sd, item_index);
                }
            }
        }
    }
}

// buff itself using items registered in hotkey bar
void process_self_buffs(map_session_data *sd)
{
    // ShowMessage("process_self_buffs\n");
    // wait for stop walk
    if (unit_is_walking(&sd->bl))
        return;

    // hotkey consumable
    int item_index;
    for (int i = 0; i < 5; i++)
    {
        if (sd->status.hotkeys[hotkey_buffs[i].hotkey_pos].type == 0 && sd->status.hotkeys[hotkey_buffs[i].hotkey_pos].id) // Blessing Scroll
        {
            item_index = pc_search_inventory(sd, sd->status.hotkeys[hotkey_buffs[i].hotkey_pos].id);
            if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[hotkey_buffs[i].hotkey_pos].id].tick == 0)
            {
                if (!sd->sc.getSCE(hotkey_buffs[i].SCE))
                    pc_useitem(sd, item_index);
            }
        }
    }

    int sp_percent = (sd->battle_status.sp * 100) / sd->battle_status.max_sp;
    if (sp_percent >= 10)
    {
        // random use buff registered
        short random_buff_skill = (rnd() % 3) + 5;

        if (sd->status.hotkeys[random_buff_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_buff_skill].id) == CAST_NODAMAGE && !sd->sc.getSCE(skill_get_sc(sd->status.hotkeys[random_buff_skill].id)))
        {
            if (unit_skilluse_id(&sd->bl, sd->bl.id, sd->status.hotkeys[random_buff_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_buff_skill].id)) == 1)
            {
                return;
            }
        }

        // acolyte and evolution skills
        if ((sd->class_ & MAPID_FOURTHMASK) >= MAPID_ACOLYTE)
        {
            for (int i = 0; i < 7; i++)
            {
                if (!sd->sc.getSCE(ac_buffs[i].SCE) && pc_checkskill(sd, ac_buffs[i].id))
                {
                    if (unit_skilluse_id(&sd->bl, sd->bl.id, ac_buffs[i].id, pc_checkskill(sd, ac_buffs[i].id)) == 1)
                    {
                    }
                }
            }
        }
    }
}

// teleport if low life
// TODO: add check to avoid use twice
void process_teleport(map_session_data *sd)
{
    // ShowMessage("process_teleport\n");
    if (sd->state.auto_attack.use_fly_wing_hp_min_percent > 0 && sd->state.auto_attack.use_fly_wing_hp_min_percent < 100)
    {
        if (sd->status.hotkeys[8].type == 0 && sd->status.hotkeys[8].id && sd->status.hotkeys[8].id == 601)
        {
            if (!sd->sc.cant.consume)
            {
                int hp_percent = (sd->battle_status.hp * 100) / sd->battle_status.max_hp;
                if (hp_percent < sd->state.auto_attack.use_fly_wing_hp_min_percent)
                {
                    int item_index = pc_search_inventory(sd, sd->status.hotkeys[8].id);
                    if (item_index >= 0)
                        pc_useitem(sd, item_index);
                }
            }
        }
    }
}

void process_auto_sit(map_session_data *sd)
{
    // ShowMessage("process_auto_sit\n");
    if (sd->state.auto_attack.can_sit == 0)
        return;

    if (sd->state.auto_attack.target.id && sd->state.auto_attack.target.id > 1)
        return;

    if (!pc_issit(sd))
    {
        clif_parse_ActionRequest_sub(sd, 0x02, sd->bl.id, gettick());
    }
}

bool has_more_target(map_session_data *sd)
{
    // can we attack ?
    if (sd->state.auto_attack.can_attack != 1)
        return false;

    if (sd->state.auto_attack.target.id && sd->state.auto_attack.target.id > 0)
    {
        // ShowMessage("I still have target.id (%d)\n", sd->state.auto_attack.target.id);
        return true;
    }
    for (int i = 5; i <= 13; i += 4)
    {
        if (map_foreachinlosarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - i, sd->bl.y - i, sd->bl.x + i, sd->bl.y + i, BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKNOREACH) > 0)
        {
            struct block_list *target;
            target = map_id2bl(sd->state.auto_attack.target.id);
            if (path_search(NULL, sd->bl.m, sd->bl.x, sd->bl.y, target->x, target->y, 1, CELL_CHKNOREACH))
                return true;
        }
    }

    return false;
}

// search for target and attack it
void process_attack(map_session_data *sd)
{
    // ShowMessage("process_attack\n");

    // can we attack ?
    if (sd->state.auto_attack.can_attack != 1)
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }

    // wait walk
    if (unit_is_walking(&sd->bl))
        return;

    // are we casting ?
    struct unit_data *ud = NULL;
    ud = unit_bl2ud(&sd->bl);
    if (ud->skilltimer != INVALID_TIMER)
        return;

    // do we have a target ?
    if (!sd->state.auto_attack.target.id || sd->state.auto_attack.target.id == 0)
    {
        for (int i = 5; i <= 13; i += 4)
        {
            if (map_foreachinlosarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - i, sd->bl.y - i, sd->bl.x + i, sd->bl.y + i, BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKNOREACH) > 0) //&& sd->state.auto_attack.last_target_id != sd->state.auto_attack.target.id
            {
                break;
            }
            sd->state.auto_attack.target.id = 0;
        }
    }

    // no target return
    if (sd->state.auto_attack.target.id == 0)
        return;

    // ShowMessage("target ok(%d)\n", sd->state.auto_attack.target.id);
    // we have target, what to do?
    if (sd->state.auto_attack.target.id > 0 && sd->state.auto_attack.target.id != sd->bl.id)
    {
        struct block_list *target;
        target = map_id2bl(sd->state.auto_attack.target.id);
        if (target == NULL || status_isdead(target))
        {
            sd->state.auto_attack.target.id = 0;
            return;
        }

        // can use skill ?
        int sp_percent = (sd->battle_status.sp * 100) / sd->battle_status.max_sp;
        if (sp_percent >= 10)
        {
            // ShowMessage("sp ok random skill (%d)\n", sd->state.auto_attack.target.id);
            // use skill block
            int random_hotkey_skill = rnd() % 5;

            // use skill from hotkey bar
            if (sd->status.hotkeys[random_hotkey_skill].type == 1) // is skill?
            {
                // ShowMessage("skill type 1 (%d)\n", sd->state.auto_attack.target.id);
                int skill_casttype = skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id); // CAST_GROUND, CAST_DAMAGE, CAST_NODAMAGE
                if (skill_casttype != CAST_NODAMAGE)                                                 // is damage or ground?
                {
                    // ShowMessage("skill damage (%d)\n", sd->state.auto_attack.target.id);
                    int skill_range = skill_get_range2(&sd->bl, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id), true);
                    skill_range = (skill_range > 2) ? skill_range - 1 : skill_range;
                    if (check_distance_bl(&sd->bl, target, skill_range)) // we are in range?
                    {
                        // ShowMessage("skill in range (%d)\n", sd->state.auto_attack.target.id);
                        if (!skill_isNotOk(sd->status.hotkeys[random_hotkey_skill].id, sd))
                        {
                            int success = 0;

                            switch (skill_casttype)
                            {
                            case CAST_DAMAGE:
                                success = unit_skilluse_id(&sd->bl, sd->state.auto_attack.target.id, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
                                break;
                            case CAST_GROUND:
                                success = unit_skilluse_pos(&sd->bl, target->x, target->y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
                                break;
                            default:
                                break;
                            }
                            // ShowMessage("tried to use skill? (%d)\n", sd->state.auto_attack.target.id);
                            if (success == 1)
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        if (unit_walktobl(&sd->bl, target, skill_range, 0) == 1)
                        {
                            // ShowMessage("walking to monster due to be away to use skill - (%d)\n", sd->state.auto_attack.target.id);
                            recalculate_route(sd, target->x, target->y);
                            return;
                        }
                    }
                }
            }
        }

        // cant use skill
        // is player in attack range?
        int range = sd->battle_status.rhw.range;
        range = (range > 2) ? range - 1 : range;
        if (check_distance_bl(&sd->bl, target, range))
        {
            // ShowMessage("in range (%d)\n", sd->state.auto_attack.target.id);
            if (unit_attack(&sd->bl, sd->state.auto_attack.target.id, 1) == 0) // attack failed?
            {
                // ShowMessage("fail attack, dropping target \n");
                sd->state.auto_attack.target.id = 0;
                return;
            }
        }
        else
        {                                // walk to monster position
            if (unit_is_walking(target)) // wait monster finish walk
                return;

            if (path_search(NULL, sd->bl.m, sd->bl.x, sd->bl.y, target->x, target->y, 1, CELL_CHKNOREACH))
            {
                if (unit_walktobl(&sd->bl, target, sd->battle_status.rhw.range, 2) == 1)
                {
                    // ShowMessage("walking to monster (%d)\n", sd->state.auto_attack.target.id);
                    recalculate_route(sd, target->x, target->y);
                    return;
                }
            }
            // ShowMessage("cant do anithing reseting (%d)\n", sd->state.auto_attack.target.id);
            sd->state.auto_attack.target.id = 0;
            return;
        }
    }
}

// recalculate rote
void recalculate_route(map_session_data *sd, int target_x, int target_y)
{
    // ShowMessage("recalculate_route\n");
    // can reach?
    if (path_search(&sd->state.route.wpd, sd->bl.m, sd->bl.x, sd->bl.y, sd->state.route.x, sd->state.route.y, 0, CELL_CHKNOPASS))
    {
        sd->state.route.current_step = 0;

        // calculate steps
        int i, c;
        int temp_x = sd->bl.x, temp_y = sd->bl.y;
        // ShowStatus("path data: path len [%d]\n", sd->state.route.wpd.path_len);
        for (i = c = 0; i < sd->state.route.wpd.path_len; i++)
        {
            temp_x += dirx[sd->state.route.wpd.path[i]];
            temp_y += diry[sd->state.route.wpd.path[i]];
            // ShowStatus("path data: [%d] (%d, %d) - dir: %d \n", i, temp_x, temp_y, sd->state.route.wpd.path[i]);
            if (i % 6 == 5 || i == sd->state.route.wpd.path_len - 1)
            {
                sd->state.route.route_steps[c].x = temp_x;
                sd->state.route.route_steps[c].y = temp_y;
                c++;
            }
        }
        sd->state.route.steps_len = c;
        sd->state.route.current_step = 1;
    }
    else
    {
        reset_route(sd);
        // ShowStatus("failed in recalculate, from: %d, %d to: %d, %d\n", sd->bl.x, sd->bl.y, sd->state.route.x, sd->state.route.y);
    }
}

// find a random place to walk
void process_random_walk(map_session_data *sd)
{
    // ShowMessage("process_real_random_walk\n");

    // map changed?
    if (sd->state.route.map_id && sd->state.route.map_id != sd->bl.m)
        reset_route(sd);

    // can move? or under stats cant move or is moving using mouse?
    if (sd->state.auto_attack.can_move != 1 || sd->sc.cant.move || unit_is_walking(&sd->bl) || pc_ischasewalk(sd))
        return;

    //  has target?
    if (sd->state.auto_attack.target.id > 0)
        return;

    //   has reached the final destination?
    if (sd->state.route.x > 0 && sd->state.route.y > 0 && sd->bl.x == sd->state.route.x && sd->bl.y == sd->state.route.y)
    {
        //   closer to destination, reset it
        reset_route(sd);
        return;
    }
    // has reached the current step pos?
    else if (sd->state.route.current_step && sd->state.route.current_step > 0 && sd->state.route.route_steps[sd->state.route.current_step - 1].x == sd->bl.x && sd->state.route.route_steps[sd->state.route.current_step - 1].y == sd->bl.y)
    {
        // ShowMessage("at the current step position adding one more\n");
        sd->state.route.current_step += 1;
    }

    // has destination?
    if (!sd->state.route.x && !sd->state.route.y)
    {
        int i = 0;
        do
        {
            short x = sd->bl.x, y = sd->bl.y;

            x = sd->bl.x + (rand() % 2 == 0 ? -1 : 1) * ((rand() % MAX_WALKPATH) + 10);
            y = sd->bl.y + (rand() % 2 == 0 ? -1 : 1) * ((rand() % MAX_WALKPATH) + 10);

            // has portal in destination?
            if (npc_check_areanpc(1, sd->bl.m, x, y, 3) > 0)
                continue;

            // can reach?
            if (path_search(&sd->state.route.wpd, sd->bl.m, sd->bl.x, sd->bl.y, x, y, 0, CELL_CHKNOPASS))
            {
                sd->state.route.x = x;
                sd->state.route.y = y;
                sd->state.route.current_step = 0;
                sd->state.route.map_id = sd->bl.m;

                // calculate steps
                int i, c;
                int temp_x = sd->bl.x, temp_y = sd->bl.y;
                // ShowStatus("path data: path len [%d]\n", sd->state.route.wpd.path_len);
                for (i = c = 0; i < sd->state.route.wpd.path_len; i++)
                {
                    temp_x += dirx[sd->state.route.wpd.path[i]];
                    temp_y += diry[sd->state.route.wpd.path[i]];
                    // ShowStatus("path data: [%d] (%d, %d) - dir: %d \n", i, temp_x, temp_y, sd->state.route.wpd.path[i]);
                    if (i % 6 == 5 || i == sd->state.route.wpd.path_len - 1)
                    {
                        sd->state.route.route_steps[c].x = temp_x;
                        sd->state.route.route_steps[c].y = temp_y;
                        c++;
                    }
                }
                sd->state.route.steps_len = c;
                sd->state.route.current_step = 1;
                break;
            }
            i++;
        } while (i < 100);
    }

    // try route
    if (sd->state.route.x > 0 && sd->state.route.y > 0)
    {
        if (unit_is_walking(&sd->bl) == 1)
            return;

        if (!check_distance((&sd->bl)->x - (sd->state.route.route_steps[sd->state.route.current_step - 1].x), (&sd->bl)->y - (sd->state.route.route_steps[sd->state.route.current_step - 1].y), 6))
        {
            // ShowStatus("away from route recaulculate - %d,%d\n", sd->state.route.x, sd->state.route.y);
            recalculate_route(sd, sd->state.route.x, sd->state.route.y);
        };

        // ShowStatus("path data: current step[%d] (%d, %d)\n", sd->state.route.current_step - 1, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y);

        if (!unit_walktoxy(&sd->bl, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y, 4))
        {
            // ShowStatus("route failed\n");
            recalculate_route(sd, sd->state.route.x, sd->state.route.y);
        }
    }
    return;
}

// reset route
void reset_route(map_session_data *sd)
{
    // ShowMessage("reset_route\n");
    sd->state.route.x = 0;
    sd->state.route.y = 0;
    sd->state.route.wpd.path_len = 0;
    sd->state.route.current_step = 0;
    sd->state.route.map_id = 0;
    // reset sd->state.route.route_steps[x]
}

// add target to target.id
static int buildin_autoattack_sub(block_list *bl, va_list ap)
{
    int *target_id = va_arg(ap, int *);
    *target_id = bl->id;
    return 1;
}

void resethotkey(int slot, map_session_data *sd)
{
    sd->status.hotkeys[slot].type = 0;
    sd->status.hotkeys[slot].id = 0;
    sd->status.hotkeys[slot].lv = 0;
}

// disable system
void disable_auto_attack(map_session_data *sd)
{
    sd->state.auto_attack.enabled = 0;
    sd->state.auto_attack.can_move = 0;
    sd->state.auto_attack.can_attack = 0; // TODO: check the difference between auto_attack_enabled and auto_attack_can_attack
    sd->state.auto_attack.use_potion_hp_min_percent = 0;
    sd->state.auto_attack.use_potion_sp_min_percent = 0;
    sd->state.auto_attack.use_fly_wing_hp_min_percent = 0;
    sd->state.auto_attack.can_sit = 0;
    sd->state.autoloot = 0;
    unit_stop_attack(&sd->bl);
    reset_route(sd);
    sd->state.auto_attack.timer = INVALID_TIMER;
    sd->state.auto_attack.delay = INVALID_TIMER;

    clif_displaymessage(sd->fd, "Auto Attack - Automatic: OFF.");
}
