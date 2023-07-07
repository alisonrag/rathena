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

    // if (sd->state.auto_attack.can_sit != 0 && sd->state.auto_attack.can_sit != 100)
    // process_auto_sit(sd);

    if (sd->state.auto_attack.can_move == 1)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[9].id].tick <= 0)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[10].id].tick <= 0)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[11].id].tick <= 0)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[12].id].tick <= 0)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[13].id].tick <= 0)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[14].id].tick <= 0)
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
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[15].id].tick <= 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION2))
            {
                pc_useitem(sd, item_index);
            }
        }
    }

    // TODO: add check if has skill Bishop

    if (rnd() % 100 <= 15)
    {
        if (sd->class_ == MAPID_ARCH_BISHOP || sd->class_ == MAPID_ARCH_BISHOP_T || sd->class_ == MAPID_CARDINAL)
        {
            if (!sd->sc.getSCE(SC_BLESSING) && pc_get_skillcooldown(sd, 2041, 3) && sd->canskill_tick)
            {
                unit_skilluse_id(&sd->bl, sd->bl.id, 2041, 3); // Blessing
            }
            // EFST_INC_AGI
            else if (!sd->sc.getSCE(SC_INCREASEAGI) && pc_get_skillcooldown(sd, 2042, 3) && sd->canskill_tick)
            {
                unit_skilluse_id(&sd->bl, sd->bl.id, 2042, 3); // Increase Agi
            }
            else if (!sd->sc.getSCE(SC_MAGNIFICAT) && pc_get_skillcooldown(sd, 74, 5) && sd->canskill_tick)
            {
                unit_skilluse_id(&sd->bl, sd->bl.id, 74, 5); // Magnificat
            }
            else if (!sd->sc.getSCE(EFST_KYRIE) && pc_get_skillcooldown(sd, 2045, 10) && sd->canskill_tick)
            {
                unit_skilluse_id(&sd->bl, sd->bl.id, 2045, 10); // Kyrie
            }
            else
            {
                return;
            }
        }
    }
    int random_buff_skill = (rnd() % 3) + 5;
    // random use buff registered
    if (sd->status.hotkeys[random_buff_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_buff_skill].id) == CAST_NODAMAGE && !sd->sc.getSCE(skill_get_sc(sd->status.hotkeys[random_buff_skill].id)) && sd->canskill_tick)
    {
        unit_skilluse_id(&sd->bl, sd->bl.id, sd->status.hotkeys[random_buff_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_buff_skill].id));
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
            else
            {
                sd->state.auto_attack.target.id = 0;
                return;
            }
        }
    }
}

void process_auto_sit(map_session_data *sd)
{
    if (sd->state.auto_attack.can_sit == 100 || sd->state.auto_attack.can_sit == 0)
        return;

    map_foreachinshootarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - 3, sd->bl.y - 3, sd->bl.x + 3, sd->bl.y + 3, BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKCLIFF, CELL_CHKNOPASS, CELL_CHKWALL);
    if (sd->state.auto_attack.target.id > 1)
        return;

    if (pc_setstand(sd, 1))
        return;

    if (((sd->battle_status.hp * 100) / sd->battle_status.max_hp <= sd->state.auto_attack.can_sit) && sd->state.auto_attack.can_sit != 100 && sd->state.auto_attack.can_sit != 0)
        pc_setstand(sd, 1);
    pc_setsit(sd);
    skill_sit(sd, 1);
    clif_sitting(&sd->bl);
    return;
}

// search for target and attack it
void process_attack(map_session_data *sd)
{
    // can we attack ?
    if (sd->state.auto_attack.can_attack != 1)
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }

    // do we have a target ?
    if (!sd->state.auto_attack.target.id || sd->state.auto_attack.target.id == 0)
    {

        for (int i = 1; i <= 3; i++)
        {
            if (map_foreachinshootarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - (i * 5), sd->bl.y - (i * 5), sd->bl.x + (i * 5), sd->bl.y + (i * 5), BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKCLIFF, CELL_CHKNOPASS, CELL_CHKWALL) > 0)
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
    }

    // we have target, what to do?
    if (sd->state.auto_attack.target.id && sd->state.auto_attack.target.id > 0 && sd->state.auto_attack.target.id != sd->bl.id)
    {
        // is target alive?
        struct block_list *target;
        target = map_id2bl(sd->state.auto_attack.target.id);
        if (target == NULL || status_isdead(target))
        {
            sd->state.auto_attack.target.id = 0;
            return;
        }

        // use skill block
        int random_hotkey_skill = rnd() % 5;
        if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_NODAMAGE && (sd->battle_status.sp * 100) / sd->battle_status.max_sp >= 10 && sd->canskill_tick)
        {
            // if not in range it will fail, but it is ok after we will try to reach the monster
            if (skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_GROUND)
            {
                unit_skilluse_id(&sd->bl, sd->state.auto_attack.target.id, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
            }
            else
            {
                unit_skilluse_pos(&sd->bl, target->x, target->y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id));
            }
        }

        // in range ?
        int range;
        range = sd->battle_status.rhw.range;

        // we are in range
        if (check_distance_bl(&sd->bl, target, range))
        {
            if (unit_attack(&sd->bl, sd->state.auto_attack.target.id, 1) == 0)
            {
                // failed to attack
                sd->state.auto_attack.target.id = 0;
                //ShowMessage("dropping target due to unit_attack failed\n");
                return;
            }
            recalculate_route(sd);
        }

        // can we reach the target?
        struct walkpath_data wpd_fake;

        if (!path_search(&wpd_fake, sd->bl.m, sd->bl.x, sd->bl.y, target->x, target->y, 0, CELL_CHKNOPASS))
        {
            sd->state.auto_attack.target.id = 0;
            //ShowMessage("dropping target due to path_search failed\n");
            return;
        }
        else if (wpd_fake.path_len > 17)
        {
            sd->state.auto_attack.target.id = 0;
            //ShowMessage("dropping target due to path_len > 17 (%d)\n", wpd_fake.path_len);
            return;
        }
        else
        {
            if (unit_walktobl(&sd->bl, target, range, 0))
            {
                //ShowMessage("can move to target\n");
            }
        }
    }
}

// find a random place to walk
void process_random_walk(map_session_data *sd)
{
    //ShowMessage("randomwalk - %d\n", sd->state.auto_attack.target.id);
    // has target?
    if (sd->state.auto_attack.target.id > 0)
        return;

    //ShowMessage("randomwalk no target\n");

    // can move?
    if (sd->state.auto_attack.can_move != 1)
        return;

    // map changed?
    if (sd->state.route.map_id && sd->state.route.map_id != sd->bl.m)
        reset_route(sd);

    //ShowMessage("can move and no target\n");
    // has reached the final destination?
    if (sd->state.route.x > 0 && sd->state.route.y > 0 && sd->bl.x == sd->state.route.x && sd->bl.y == sd->state.route.y)
    {
        //ShowMessage("reseting\n");
        // closer to destination, reset it
        reset_route(sd);
        return;
    }
    // has reached the current step pos?
    else if (sd->state.route.current_step && sd->state.route.current_step > 0 && sd->state.route.route_steps[sd->state.route.current_step - 1].x == sd->bl.x && sd->state.route.route_steps[sd->state.route.current_step - 1].y == sd->bl.y)
    {
        //ShowMessage("at the current step position adding one more\n");
        sd->state.route.current_step += 1;
    }

    // has destination?
    if (!sd->state.route.x && !sd->state.route.y)
    {
        int i = 0;
        do
        {
            struct map_data *mapdata = map_getmapdata(sd->bl.m);
            short x = sd->bl.x, y = sd->bl.y;
            x = rnd() % (mapdata->xs - 10) + 1;
            y = rnd() % (mapdata->ys - 10) + 1;

            // has portal in destination?
            if (npc_check_areanpc(1, sd->bl.m, x, y, 3) > 0)
                continue;

            // can reach?
            if (path_search(&sd->state.route.wpd, sd->bl.m, sd->bl.x, sd->bl.y, x, y, 0, CELL_CHKNOPASS))
            {
                sd->state.route.x = x;
                sd->state.route.y = y;
                sd->state.route.current_step = 0;

                // calculate steps
                int i, c;
                int temp_x = sd->bl.x, temp_y = sd->bl.y;
                //ShowStatus("path data: path len [%d]\n", sd->state.route.wpd.path_len);
                for (i = c = 0; i < sd->state.route.wpd.path_len; i++)
                {
                    temp_x += dirx[sd->state.route.wpd.path[i]];
                    temp_y += diry[sd->state.route.wpd.path[i]];
                    // //ShowStatus("path data: [%d] (%d, %d) - dir: %d \n", i, temp_x, temp_y, sd->state.route.wpd.path[i]);
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

        if (!check_distance_blxy(&sd->bl, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y, 6))
        {
            //ShowStatus("away from route recaulculate - %d,%d\n", sd->state.route.x, sd->state.route.y);
            reset_route(sd);
        };

        //ShowStatus("trying to walk to dest - %d,%d\n", sd->state.route.x, sd->state.route.y);

        //ShowStatus("path data: current step[%d] (%d, %d)\n", sd->state.route.current_step - 1, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y);

        if (!unit_walktoxy(&sd->bl, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y, 4))
        {
            //ShowStatus("route failed\n");
        }
        return;
    }
}

void recalculate_route(map_session_data *sd)
{
    // can reach?
    if (path_search(&sd->state.route.wpd, sd->bl.m, sd->bl.x, sd->bl.y, sd->state.route.x, sd->state.route.y, 0, CELL_CHKNOPASS))
    {
        sd->state.route.current_step = 0;

        // calculate steps
        int i, c;
        int temp_x = sd->bl.x, temp_y = sd->bl.y;
        //ShowStatus("path data: path len [%d]\n", sd->state.route.wpd.path_len);
        for (i = c = 0; i < sd->state.route.wpd.path_len; i++)
        {
            temp_x += dirx[sd->state.route.wpd.path[i]];
            temp_y += diry[sd->state.route.wpd.path[i]];
            // //ShowStatus("path data: [%d] (%d, %d) - dir: %d \n", i, temp_x, temp_y, sd->state.route.wpd.path[i]);
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
        //ShowStatus("failed in recalculate\n");
    }
}

// reset route
void reset_route(map_session_data *sd)
{
    sd->state.route.x = 0;
    sd->state.route.y = 0;
    sd->state.route.wpd.path_len = 0;
    sd->state.route.wpd.path_pos = 0;
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
    // sd->auto_attack_delay = gettick() + 1000; // check how to disable timer
    unit_stop_attack(&sd->bl);
    sd->state.auto_attack.timer = INVALID_TIMER;
    clif_hat_effect_single(sd, HAT_EF_C_BLESSINGS_OF_SOUL, false);
    clif_displaymessage(sd->fd, "Auto Attack - Automatic: OFF.");
}
