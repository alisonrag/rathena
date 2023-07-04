
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
    process_attack(sd);
    process_teleport(sd);
    process_random_walk(sd);
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
    if (sd->state.auto_attack.can_attack == 1)
    {
        ShowStatus("Can attack\n");
        sd->state.auto_attack.target.id = 0;

        map_foreachinarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - 10, sd->bl.y - 10, sd->bl.x + 10, sd->bl.y + 10, BL_MOB, &sd->state.auto_attack.target.id);

        ShowStatus("Can attack %d -  my id %d\n ", sd->state.auto_attack.target.id, sd->bl.id);
        if (sd->state.auto_attack.target.id && sd->state.auto_attack.target.id > 0 && sd->state.auto_attack.target.id != sd->bl.id)
        {
            ShowStatus("Target found attack\n");
            unit_attack(&sd->bl, sd->state.auto_attack.target.id, 1);

            int random_hotkey_skill = rnd() % 5;

            if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_GROUND && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_NODAMAGE)
            {
                ShowStatus("use skill 1\n");
                unit_skilluse_id(&sd->bl, sd->state.auto_attack.target.id, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
            }
            else if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) == CAST_GROUND)
            {
                ShowStatus("use skill 1\n");
                unit_skilluse_pos(&sd->bl, sd->state.auto_attack.target.x, sd->state.auto_attack.target.y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
            }
            snprintf(atcmd_output, sizeof atcmd_output, msg_txt(sd, 1187), ((double)sd->state.autoloot) / 50.); // Autolooting items with drop rates of %0.02f%% and below.
        }
        else
        {
            sd->state.auto_attack.target.id = 0;
            // sd->state.auto_attack.target = nullptr;
        }
    }
    else
    {
        sd->state.auto_attack.target.id = 0;
    }
}

void process_random_walk(map_session_data *sd)
{
    ShowStatus("can random walk\n");
    // has target?
    if (sd->state.auto_attack.target.id > 0)
        return;
    ShowStatus("can move, non target %d\n", sd->state.auto_attack.can_move);
    // can move?
    if (sd->state.auto_attack.can_move != 1)
        return;
    ShowStatus("can move, can move\n");
    // has destination?
    if (sd->state.route.x && sd->state.route.y)
    {
        ShowStatus("dest ok\n");
        // has reached the final destination?
        if (sd->bl.x == sd->state.route.x && sd->bl.y == sd->state.route.y)
        {
            ShowStatus("reached the dest\n");
            // closer to destination, reset it
            sd->state.route.x = 0;
            sd->state.route.y = 0;
            sd->state.route.wpd.path_len = 0;
            sd->state.route.wpd.path_pos = 0;
        }
        else
        {
            ShowStatus("trying to walk to dest - %d,%d\n", sd->state.route.x, sd->state.route.y);
            // has small dest?
            // TODO: cut longest route in small pieces
            // walk small distances
            unit_walktoxy(&sd->bl, sd->state.route.x, sd->state.route.y, 4);
        }
    }
    else // get random spot to walk
    {
        ShowStatus("trying to get random spot\n");

        int x, y;
        struct map_data *mapdata = map_getmapdata(sd->bl.m);
        int i = 0;
        do
        {
            x = rnd() % (mapdata->xs - 2) + 1;
            y = rnd() % (mapdata->ys - 2) + 1;
        } while ((map_getcell(sd->bl.m, x, y, CELL_CHKNOPASS) || (!battle_config.teleport_on_portal && npc_check_areanpc(1, sd->bl.m, x, y, 1))) && (i++) < 1000);

        if (i < 1000)
        {
            sd->state.route.x = x;
            sd->state.route.y = y;
            path_search(&sd->state.route.wpd, sd->bl.m, sd->bl.x, sd->bl.y, x, y, 0, CELL_CHKWALL);
        }
    }
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
