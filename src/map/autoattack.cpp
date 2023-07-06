#include <map/autoattack.hpp>
#include <map/npc.hpp>
#include <common/showmsg.hpp>
#include <map/map.hpp>
#include "battle.hpp"

static char atcmd_output[CHAT_SIZE_MAX];

// main loop
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

    if (sd->state.auto_attack.use_potion_hp_min_percent > 0 && sd->state.auto_attack.use_potion_hp_min_percent < 100)
        process_self_heal(sd);

    process_self_buffs(sd);
    process_auto_sit(sd);

    if (sd->state.auto_attack.can_move)
        process_random_walk(sd);

    process_attack(sd);

    if (sd->state.auto_attack.use_fly_wing_hp_min_percent > 0 && sd->state.auto_attack.use_fly_wing_hp_min_percent < 100)
        process_teleport(sd);
}

void process_dead(map_session_data *sd)
{
    // return to saveMap or disable?
}

// use pots / heal itself
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

// buff itself using items registered in hotkey bar
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

    // random use buff registered
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

// teleport if low life
// TODO: add check to avoid use twice
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

// search for target and attack it
void process_attack(map_session_data *sd)
{
    if (sd->state.auto_attack.can_attack != 1)
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }

    for (int i = 1; i <= 3; i++)
    {
        map_foreachinshootarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - (i * 5), sd->bl.y - (i * 5), sd->bl.x + (i * 5), sd->bl.y + (i * 5), BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKCLIFF, CELL_CHKNOPASS, CELL_CHKWALL);
        if (&sd->state.auto_attack.target.id)
        {
            break;
        }
        else
        {
            sd->state.auto_attack.target.id = 0;
        }
    }

    if (sd->state.auto_attack.target.id == 0)
        return;

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
        // TODO: add check for sp and range
        if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_GROUND && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_NODAMAGE && (sd->battle_status.sp * 100) / sd->battle_status.max_sp >= 10)
        {
            unit_skilluse_id(&sd->bl, sd->state.auto_attack.target.id, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
        }
        else if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) == CAST_GROUND && (sd->battle_status.sp * 100) / sd->battle_status.max_sp >= 10)
        {
            unit_skilluse_pos(&sd->bl, target->x, target->y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
        }
        // in range ?
        int range;
        range = sd->battle_status.rhw.range;
        if (!check_distance_bl(&sd->bl, target, range))
        {
            if (unit_walktobl(&sd->bl, target, range, 1) == 0)
            {
                ShowStatus("Drop target due to cant reach\n");
                sd->state.auto_attack.target.id = 0;
                return;
            }
        }
        unit_attack(&sd->bl, sd->state.auto_attack.target.id, 1);
        reset_route(sd);
    }
    else
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }
}

// find a random place to walk
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
            if (map_search_freecell(&sd->bl, sd->bl.m, &x, &y, 25, 25, 2)) // search random cell
            {
                // has portal in destination?
                if (npc_check_areanpc(1, sd->bl.m, x, y, 3) > 0)
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
            // try rand route
            short x = sd->bl.x, y = sd->bl.y;
            int intermediateX, intermediateY;
            int dx, dy, distance;
            dx = sd->state.route.x - x;
            dy = sd->state.route.y - y;
            distance = static_cast<int>(sqrt(sd->bl.x * dx + sd->bl.y * dy));
            int stepCount;
            stepCount = distance / 10; // Número de passos intermediários
            intermediateX = x + (rand() % 5 + 1);
            intermediateY = y + (rand() % 5 + 1);
            if (distance > 10)
            {
                intermediateX = x + (dx / stepCount);
                intermediateY = y + (dy / stepCount);
            }
            ShowStatus("trying to walk to altrout - %d,%d\n", intermediateX, intermediateY);
            if (!unit_walktoxy(&sd->bl, intermediateX, intermediateY, 4))
            {
                ShowStatus("route failed\n");
            }
        }
    }
}

// reset route
void reset_route(map_session_data *sd)
{
    sd->state.route.x = 0;
    sd->state.route.y = 0;
    sd->state.route.wpd.path_len = 0;
    sd->state.route.wpd.path_pos = 0;
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
    int effectID = 38;
    clif_hat_effect_single(sd, effectID, false);
    sd->state.auto_attack.enabled = 0;
    sd->state.auto_attack.can_move = 0;
    sd->state.auto_attack.can_attack = 0; // TODO: check the difference between auto_attack_enabled and auto_attack_can_attack
    sd->state.auto_attack.use_potion_hp_min_percent = 0;
    sd->state.auto_attack.use_potion_sp_min_percent = 0;
    sd->state.auto_attack.use_fly_wing_hp_min_percent = 0;
    sd->state.autoloot = 0;
    unit_stop_attack(&sd->bl);
    clif_displaymessage(sd->fd, "Auto Attack - Automatic: OFF.");
}