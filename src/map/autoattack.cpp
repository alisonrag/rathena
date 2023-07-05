
#include <map/autoattack.hpp>
#include <map/npc.hpp>
#include <common/showmsg.hpp>
#include <map/map.hpp>
#include "battle.hpp"

static char atcmd_output[CHAT_SIZE_MAX];

void auto_attack_iterate(map_session_data *sd)
{
    if (map_getmapflag(sd->bl.m, MF_NOAUTOATTACK) && sd->state.auto_attack.enabled)
    {
        clif_displaymessage(sd->fd, "Auto Attack: Sorry Auto Attack cannot be used on this map.");
        disable_auto_attack(sd);
        return;
    }

    if (!sd->state.auto_attack.enabled)
    {
        disable_auto_attack(sd);
        return;
    }
    process_dead(sd);
    if (pc_isdead(sd))
        return;

    process_self_heal(sd);
    process_self_buffs(sd);
    process_auto_sit(sd);
    process_random_walk(sd);
    process_attack(sd);
    process_teleport(sd);
}

void process_dead(map_session_data *sd)
{
    // return to saveMap or disable?
}

void process_self_heal(map_session_data *sd)
{
    if (sd->class_ == MAPID_ARCH_BISHOP || sd->class_ == MAPID_ARCH_BISHOP_T || sd->class_ == MAPID_CARDINAL)
    {
        if (((sd->battle_status.hp * 100) / sd->battle_status.max_hp <= sd->state.auto_attack.use_potion_hp_min_percent) && sd->state.auto_attack.use_potion_hp_min_percent != 100 && sd->state.auto_attack.use_potion_hp_min_percent != 0)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2043, 3); // Heal
        }
    }

    int item_index;
    if (sd->status.hotkeys[9].type == 0 && sd->status.hotkeys[9].id) // HP Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[9].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (((sd->battle_status.hp * 100) / sd->battle_status.max_hp <= sd->state.auto_attack.use_potion_hp_min_percent) && sd->state.auto_attack.use_potion_hp_min_percent != 100 && sd->state.auto_attack.use_potion_hp_min_percent != 0)
            {
                pc_useitem(sd, item_index);
            }
        }
    }

    if (sd->status.hotkeys[10].type == 0 && sd->status.hotkeys[10].id) // SP Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[10].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (((sd->battle_status.sp * 100) / sd->battle_status.max_sp <= sd->state.auto_attack.use_potion_sp_min_percent) && sd->state.auto_attack.use_potion_sp_min_percent != 100 && sd->state.auto_attack.use_potion_sp_min_percent != 0)
            {
                pc_useitem(sd, item_index);
            }
        }
    }
}

void process_self_buffs(map_session_data *sd)
{
    int item_index;
    if (sd->status.hotkeys[11].type == 0 && sd->status.hotkeys[11].id) // Blessing Scroll
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[11].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (!sd->sc.getSCE(SC_BLESSING))
            {
                pc_useitem(sd, item_index);
            }
        }
    }
    if (sd->status.hotkeys[12].type == 0 && sd->status.hotkeys[12].id) // Increase Agi Scroll
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[12].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (!sd->sc.getSCE(SC_INCREASEAGI))
            {
                pc_useitem(sd, item_index);
            }
        }
    }
    if (sd->status.hotkeys[13].type == 0 && sd->status.hotkeys[13].id) // Concentration Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[13].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION0))
            {
                pc_useitem(sd, item_index);
            }
        }
    }
    if (sd->status.hotkeys[14].type == 0 && sd->status.hotkeys[14].id) // Awakening Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[14].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION1))
            {
                pc_useitem(sd, item_index);
            }
        }
    }
    if (sd->status.hotkeys[15].type == 0 && sd->status.hotkeys[15].id) // Bersek Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[15].id);
        if (!sd->sc.cant.consume && item_index >= 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION2))
            {
                pc_useitem(sd, item_index);
            }
        }
    }

    // TODO: add check if has skill Bishop
    if (sd->class_ == MAPID_ARCH_BISHOP || sd->class_ == MAPID_ARCH_BISHOP_T || sd->class_ == MAPID_CARDINAL)
    {
        if (!sd->sc.getSCE(SC_BLESSING))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2041, 3); // Blessing
        }
        // EFST_INC_AGI
        else if (!sd->sc.getSCE(SC_INCREASEAGI))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2042, 3); // Increase Agi
        }
        else if (!sd->sc.getSCE(SC_MAGNIFICAT))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 74, 5); // Magnificat
        }
        else if (!sd->sc.getSCE(EFST_KYRIE))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2045, 10); // Kyrie
        }
    }

    if (rand() % 100 <= 15)
    {
        if (sd->status.hotkeys[5].type == 1 && skill_get_casttype(sd->status.hotkeys[5].id) == CAST_NODAMAGE)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, sd->status.hotkeys[5].id, pc_checkskill(sd, sd->status.hotkeys[5].id));
        }
    }
    if (rand() % 100 <= 15)
    {
        if (sd->status.hotkeys[6].type == 1 && skill_get_casttype(sd->status.hotkeys[6].id) == CAST_NODAMAGE)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, sd->status.hotkeys[6].id, pc_checkskill(sd, sd->status.hotkeys[6].id));
        }
    }
    if (rand() % 100 <= 15)
    {
        if (sd->status.hotkeys[7].type == 1 && skill_get_casttype(sd->status.hotkeys[7].id) == CAST_NODAMAGE)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, sd->status.hotkeys[7].id, pc_checkskill(sd, sd->status.hotkeys[7].id));
        }
    }
}

void process_teleport(map_session_data *sd)
{
    if (sd->status.hotkeys[8].type == 0 && sd->status.hotkeys[8].id)
    {
        if (!sd->sc.cant.consume && sd->status.hotkeys[8].id == 601)
        {
            int item_index = pc_search_inventory(sd, sd->status.hotkeys[8].id);
            if (item_index >= 0 && ((sd->battle_status.hp * 100) / sd->battle_status.max_hp <= sd->state.auto_attack.use_fly_wing_hp_min_percent) && sd->state.auto_attack.use_fly_wing_hp_min_percent != 100 && sd->state.auto_attack.use_fly_wing_hp_min_percent != 0)
            {
                pc_useitem(sd, item_index);
                sd->state.auto_attack.target.id = 0;
            }
        }
    }
}

void process_auto_sit(map_session_data *sd)
{
}

void process_attack(map_session_data *sd)
{
    if (sd->state.auto_attack.can_attack != 1)
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }

    if (!sd->state.auto_attack.target.id)
    {
        sd->state.auto_attack.target.id = 0;

        if (map_foreachinshootarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - 14, sd->bl.y - 14, sd->bl.x + 14, sd->bl.y + 14, BL_MOB, &sd->state.auto_attack.target.id) == 0)
        {
            return;
        }
    }

    if (sd->state.auto_attack.target.id && sd->state.auto_attack.target.id > 0 && sd->state.auto_attack.target.id != sd->bl.id)
    {
        struct block_list *target;
        target = map_id2bl(sd->state.auto_attack.target.id);
        if (target == NULL || status_isdead(target))
        {
            sd->state.auto_attack.target.id = 0;
            return;
        }

        snprintf(atcmd_output, sizeof atcmd_output, msg_txt(sd, 1187), ((double)sd->state.autoloot) / 50.); // Autolooting items with drop rates of %0.02f%% and below.

        int random_hotkey_skill = rnd() % 5;
        
        if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_GROUND && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_NODAMAGE)
        {
            unit_skilluse_id(&sd->bl, sd->state.auto_attack.target.id, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
        }
        else if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) == CAST_GROUND)
        {
            unit_skilluse_pos(&sd->bl, target->x, target->y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
        }
        else
        {
            // in range ?
            int range;
            range = sd->battle_status.rhw.range;
            if (!check_distance_bl(&sd->bl, target, range))
            {
                if (unit_walktobl(&sd->bl, target, range, 0) == 0)
                {
                    ShowStatus("Drop target due to cant reach\n");
                    sd->state.auto_attack.target.id = 0;
                    return;
                }
            }
            reset_route(sd);
            unit_attack(&sd->bl, sd->state.auto_attack.target.id, 1);
        }
    }
    else
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }
}

void process_random_walk(map_session_data *sd)
{
    // has target?
    if (sd->state.auto_attack.target.id > 0)
        return;
    
    // can move?
    if (sd->state.auto_attack.can_move != 1)
        return;

    // has reached the final destination?
    if (sd->state.route.x > 0 && sd->state.route.y > 0 && sd->bl.x == sd->state.route.x && sd->bl.y == sd->state.route.y)
    {
        // closer to destination, reset it
        reset_route(sd);
        return;
    }

    // has destination?
    if (!sd->state.route.x && !sd->state.route.y)
    {
        int i = 0;
        do
        {
            short x = sd->bl.x, y = sd->bl.y;
            if (map_search_freecell(&sd->bl, sd->bl.m, &x, &y, 32, 32, 2)) // search random cell
            {
                // has portal in destination?
                if(npc_check_areanpc(1,sd->bl.m,x,y,3) > 0)
                    continue;
                
                // can reach?
                if (path_search(&sd->state.route.wpd, sd->bl.m, sd->bl.x, sd->bl.y, x, y, 0, CELL_CHKNOPASS))
                {
                    sd->state.route.x = x;
                    sd->state.route.y = y;
                    break;
                }
            }
            i++;
        } while (i < 10);
    }

    // try to route
    if (sd->state.route.x > 0 && sd->state.route.y > 0)
    {
        ShowStatus("trying to walk to dest - %d,%d\n", sd->state.route.x, sd->state.route.y);
        if (!unit_walktoxy(&sd->bl, sd->state.route.x, sd->state.route.y, 4))
        {
            ShowStatus("route failed\n");
        }
    }
}

void reset_route(map_session_data *sd)
{
    sd->state.route.x = 0;
    sd->state.route.y = 0;
    sd->state.route.wpd.path_len = 0;
    sd->state.route.wpd.path_pos = 0;
}

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

void disable_auto_attack(map_session_data *sd)
{
    sd->state.auto_attack.enabled = 0;
    sd->state.auto_attack.can_move = 0;
    sd->state.auto_attack.can_attack = 0; // TODO: check the difference between auto_attack_enabled and auto_attack_can_attack
    sd->state.auto_attack.use_potion_hp_min_percent = 0;
    sd->state.auto_attack.use_potion_sp_min_percent = 0;
    sd->state.auto_attack.use_fly_wing_hp_min_percent = 0;
    sd->state.autoloot = 0;
    // sd->auto_attack_delay = gettick() + 1000; // check how to disable timer
    unit_stop_attack(&sd->bl);
    sd->state.auto_attack.timer = INVALID_TIMER;
    clif_displaymessage(sd->fd, "Auto Attack - Automatic: OFF.");
}
