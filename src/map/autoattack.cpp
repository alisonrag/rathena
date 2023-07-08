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

    if (((sd->state.auto_attack.use_potion_hp_min_percent > 0 && sd->state.auto_attack.use_potion_hp_min_percent < 100) || (sd->state.auto_attack.use_potion_sp_min_percent > 0 && sd->state.auto_attack.use_potion_sp_min_percent < 100)) && ((sd->battle_status.hp * 100) / sd->battle_status.max_hp < sd->state.auto_attack.use_potion_hp_min_percent || (sd->battle_status.sp * 100) / sd->battle_status.max_sp < sd->state.auto_attack.use_potion_sp_min_percent))
        process_self_heal(sd);

    process_self_buffs(sd);

    if (sd->state.auto_attack.can_sit != 0 && sd->state.auto_attack.can_sit != 100)
        process_auto_sit(sd);

    process_attack(sd);

    if (has_more_target(sd))
        return;

    if (sd->state.auto_attack.can_move == 1)
        process_random_walk(sd);

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
    short SP_percent = (sd->battle_status.sp * 100) / sd->battle_status.max_sp;
    short HP_percent = (sd->battle_status.hp * 100) / sd->battle_status.max_hp;
    if ((sd->class_ & MAPID_FOURTHMASK) >= MAPID_ACOLYTE)
    {
        if ((HP_percent >= sd->state.auto_attack.use_potion_hp_min_percent))
        {
            return;
        }

        if ((HP_percent <= 10 && pc_checkskill(sd, 5268) && sd->canskill_tick && sd->class_ == MAPID_CARDINAL))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 5268, pc_checkskill(sd, 5268)); // reparatio
        }
        else if (pc_checkskill(sd, 5280) && sd->canskill_tick && sd->class_ == MAPID_CARDINAL)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 5280, pc_checkskill(sd, 5280)); // Dilectio Heal
        }
        else if (pc_checkskill(sd, 28) && sd->canskill_tick && (sd->class_ == MAPID_ACOLYTE || sd->class_ == MAPID_PRIEST))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 28, pc_checkskill(sd, 28)); // Heal
        }
        else if (pc_checkskill(sd, 2043) && sd->canskill_tick && sd->class_ == MAPID_ARCH_BISHOP)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2043, pc_checkskill(sd, 2043)); // Heal AB
        }
        else
        {
            return;
        }
        sd->state.route.sent_route_move = false;
    }

    int item_index;
    if (sd->status.hotkeys[9].type == 0 && sd->status.hotkeys[9].id && !sd->sc.cant.consume && HP_percent <= sd->state.auto_attack.use_potion_hp_min_percent) // HP Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[9].id);
        if (item_index >= 0 && sd->item_delay[sd->status.hotkeys[9].id].tick <= 0)
        {
            pc_useitem(sd, item_index);
        }
        else
        {
            return;
        }
    }

    if (sd->status.hotkeys[10].type == 0 && sd->status.hotkeys[10].id && !sd->sc.cant.consume && SP_percent <= sd->state.auto_attack.use_potion_sp_min_percent) // SP Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[10].id);
        if (item_index >= 0 && sd->item_delay[sd->status.hotkeys[10].id].tick <= 0)
        {
            pc_useitem(sd, item_index);
        }
        else
        {
            return;
        }
    }
}

// buff itself using items registered in hotkey bar
void process_self_buffs(map_session_data *sd)
{

    // skill check e lv cast added
    if ((sd->class_ & MAPID_FOURTHMASK) >= MAPID_ACOLYTE)
    {
        if (!sd->sc.getSCE(SC_BLESSING) && pc_checkskill(sd, 2041) && sd->canskill_tick)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2041, pc_checkskill(sd, 2041)); // Blessing
        }
        else if (!sd->sc.getSCE(SC_BLESSING) && pc_checkskill(sd, 34) && sd->canskill_tick)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 34, pc_checkskill(sd, 34)); // Blessing
        }
        else if (!sd->sc.getSCE(SC_INCREASEAGI) && pc_checkskill(sd, 2042) && sd->canskill_tick)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2042, pc_checkskill(sd, 2042)); // Increase Agi
        }
        else if (!sd->sc.getSCE(SC_INCREASEAGI) && pc_checkskill(sd, 29))
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 29, pc_checkskill(sd, 29)); // Increase Agi
        }
        else if (!sd->sc.getSCE(SC_MAGNIFICAT) && pc_checkskill(sd, 74) && sd->canskill_tick)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 74, pc_checkskill(sd, 74)); // Magnificat
        }
        else if (!sd->sc.getSCE(EFST_KYRIE) && pc_checkskill(sd, 2045) && sd->canskill_tick)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 2045, pc_checkskill(sd, 2045)); // Kyrie
        }
        else if (!sd->sc.getSCE(EFST_KYRIE) && pc_checkskill(sd, 73) && sd->canskill_tick)
        {
            unit_skilluse_id(&sd->bl, sd->bl.id, 73, pc_checkskill(sd, 73)); // Kyrie
        }
        else
        {
            return;
        }
        sd->state.route.sent_route_move = false;
    }

    int random_buff_skill = (rnd() % 3) + 5;
    // random use buff registered
    if (sd->status.hotkeys[random_buff_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_buff_skill].id) == CAST_NODAMAGE && !sd->sc.getSCE(skill_get_sc(sd->status.hotkeys[random_buff_skill].id)) && sd->canskill_tick)
    {
        unit_skilluse_id(&sd->bl, sd->bl.id, sd->status.hotkeys[random_buff_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_buff_skill].id));
        sd->state.route.sent_route_move = false;
    }
    else
    {
        return;
    }

    int item_index;
    if (sd->status.hotkeys[11].type == 0 && sd->status.hotkeys[11].id) // Blessing Scroll
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[11].id);
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[11].id].tick == 0)
        {
            if (!sd->sc.getSCE(SC_BLESSING))
            {
                pc_useitem(sd, item_index);
            }
            else
            {
                return;
            }
        }
    }
    if (sd->status.hotkeys[12].type == 0 && sd->status.hotkeys[12].id) // Increase Agi Scroll
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[12].id);
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[12].id].tick == 0)
        {
            if (!sd->sc.getSCE(SC_INCREASEAGI))
            {
                pc_useitem(sd, item_index);
            }
            else
            {
                return;
            }
        }
    }
    if (sd->status.hotkeys[13].type == 0 && sd->status.hotkeys[13].id) // Concentration Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[13].id);
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[13].id].tick == 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION0))
            {
                pc_useitem(sd, item_index);
            }
            else
            {
                return;
            }
        }
    }
    if (sd->status.hotkeys[14].type == 0 && sd->status.hotkeys[14].id) // Awakening Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[14].id);
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[14].id].tick == 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION1))
            {
                pc_useitem(sd, item_index);
            }
            else
            {
                return;
            }
        }
    }
    if (sd->status.hotkeys[15].type == 0 && sd->status.hotkeys[15].id) // Bersek Potion
    {
        item_index = pc_search_inventory(sd, sd->status.hotkeys[15].id);
        if (!sd->sc.cant.consume && item_index >= 0 && sd->item_delay[sd->status.hotkeys[15].id].tick == 0)
        {
            if (!sd->sc.getSCE(SC_ASPDPOTION2))
            {
                pc_useitem(sd, item_index);
            }
            else
            {
                return;
            }
        }
    }
}

// teleport if low life
// TODO: add check to avoid use twice
void process_teleport(map_session_data *sd)
{
    if (sd->status.hotkeys[8].type == 0 && sd->status.hotkeys[8].id)
    {
        if (!sd->sc.cant.consume)
        {
            int item_index = pc_search_inventory(sd, sd->status.hotkeys[8].id);
            if (item_index >= 0 && ((sd->battle_status.hp * 100) / sd->battle_status.max_hp <= sd->state.auto_attack.use_fly_wing_hp_min_percent) && sd->status.hotkeys[8].id == 601)
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

    map_foreachinshootarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - 1, sd->bl.y - 1, sd->bl.x + 1, sd->bl.y + 1, BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKCLIFF, CELL_CHKNOPASS, CELL_CHKWALL);
    if (sd->state.auto_attack.target.id > 1)
    {
        sd->state.auto_attack.can_move = 1;
        return;
    }

    sd->state.auto_attack.can_move = 0;
    if (!pc_setstand(sd, 1))
    {
        //ShowStatus("cant sit now\n");
        return;
    }

    pc_setsit(sd) if (!skill_sit(sd, 1))
    {
        //ShowStatus("cant sit now\n");
        return;
    }

    clif_sitting(&sd->bl);
}

bool has_more_target(map_session_data *sd)
{
    // can we attack ?
    if (sd->state.auto_attack.can_attack != 1)
    {
        return false;
    }

    // do we have a target ?
    if (!sd->state.auto_attack.target.id || sd->state.auto_attack.target.id == 0)
    {
        for (int i = 1; i <= 3; i++)
        {
            if (map_foreachinshootarea(buildin_autoattack_sub, sd->bl.m, sd->bl.x - (i * 5), sd->bl.y - (i * 5), sd->bl.x + (i * 5), sd->bl.y + (i * 5), BL_MOB, &sd->state.auto_attack.target.id, CELL_CHKCLIFF, CELL_CHKNOPASS, CELL_CHKWALL) > 0)
            {
                return true;
            }
        }
    }
    return false;
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
        struct block_list *target;
        target = map_id2bl(sd->state.auto_attack.target.id);
        if (target == NULL || status_isdead(target))
        {
            sd->state.auto_attack.target.id = 0;
            return;
        }

        //        snprintf(atcmd_output, sizeof atcmd_output, msg_txt(sd, 1187), ((double)sd->state.autoloot) / 50.); // Autolooting items with drop rates of %0.02f%% and below.

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
        // use skill block
        int random_hotkey_skill = rnd() % 5;
        if (sd->status.hotkeys[random_hotkey_skill].type == 1 && skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_NODAMAGE && (sd->battle_status.sp * 100) / sd->battle_status.max_sp >= 10 && sd->canskill_tick)
        {
            // if not in range it will fail, but it is ok after we will try to reach the monster
            if (skill_get_casttype(sd->status.hotkeys[random_hotkey_skill].id) != CAST_GROUND)
            {
                if (!unit_skilluse_id(&sd->bl, sd->state.auto_attack.target.id, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id)) || !battle_check_range(&sd->bl, target, skill_get_range2(&sd->bl, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id), true)))
                {
                    return;
                }
            }
            else
            {
                if (!unit_skilluse_pos(&sd->bl, target->x, target->y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id)) || !battle_check_range(&sd->bl, target, skill_get_range2(&sd->bl, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id), true)))
                {
                    if (!battle_check_range(&sd->bl, target, skill_get_range2(&sd->bl, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id), true)) || !unit_skilluse_pos2(&sd->bl, target->x, target->y, sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id), skill_get_cast(sd->status.hotkeys[random_hotkey_skill].id, pc_checkskill(sd, sd->status.hotkeys[random_hotkey_skill].id)), 1))
                    {
                        return;
                    }
                }
            }
        }
        // can we reach the target?
        struct walkpath_data wpd_fake;

        if (!path_search(&wpd_fake, sd->bl.m, sd->bl.x, sd->bl.y, target->x, target->y, 0, CELL_CHKNOPASS))
        {
            sd->state.auto_attack.target.id = 0;
            //ShowMessage("dropping target due to path_search failed\n");
            return;
        }
        else if (wpd_fake.path_len > 20)
        {
            sd->state.auto_attack.target.id = 0;
            //ShowMessage("dropping target due to path_len > 20 (%d)\n", wpd_fake.path_len);
            return;
        }
        else
        {
            if (unit_walktobl(&sd->bl, target, range, CELL_CHKNOPASS))
            {
                recalculate_route(sd);
                return;
            }
        }
    }
    else
    {
        sd->state.auto_attack.target.id = 0;
        return;
    }

    return;
}

// find a random place to walk
void process_random_walk(map_session_data *sd)
{
    //ShowMessage("randomwalk - %d\n", sd->state.auto_attack.target.id);
    //  has target?
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
    //  has reached the final destination?
    if (sd->state.route.x > 0 && sd->state.route.y > 0 && sd->bl.x == sd->state.route.x && sd->bl.y == sd->state.route.y)
    {
        //ShowMessage("reseting\n");
        //  closer to destination, reset it
        reset_route(sd);
        return;
    }
    // has reached the current step pos?
    else if (sd->state.route.current_step && sd->state.route.current_step > 0 && sd->state.route.route_steps[sd->state.route.current_step - 1].x == sd->bl.x && sd->state.route.route_steps[sd->state.route.current_step - 1].y == sd->bl.y)
    {
        //ShowMessage("at the current step position adding one more\n");
        sd->state.route.sent_route_move = false;
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
                sd->state.route.map_id = sd->bl.m;

                // calculate steps
                int i, c;
                int temp_x = sd->bl.x, temp_y = sd->bl.y;
                //ShowStatus("path data: path len [%d]\n", sd->state.route.wpd.path_len);
                for (i = c = 0; i < sd->state.route.wpd.path_len; i++)
                {
                    temp_x += dirx[sd->state.route.wpd.path[i]];
                    temp_y += diry[sd->state.route.wpd.path[i]];
                    // ////ShowStatus("path data: [%d] (%d, %d) - dir: %d \n", i, temp_x, temp_y, sd->state.route.wpd.path[i]);
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

        if (!check_distance((&sd->bl)->x - (sd->state.route.route_steps[sd->state.route.current_step - 1].x), (&sd->bl)->y - (sd->state.route.route_steps[sd->state.route.current_step - 1].y), 6))
        {
            //ShowStatus("away from route recaulculate - %d,%d\n", sd->state.route.x, sd->state.route.y);
            recalculate_route(sd);
        };

        //ShowStatus("trying to walk to dest - %d,%d\n", sd->state.route.x, sd->state.route.y);

        //ShowStatus("path data: current step[%d] (%d, %d)\n", sd->state.route.current_step - 1, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y);

        if (!sd->state.route.sent_route_move)
        {
            if (!unit_walktoxy(&sd->bl, sd->state.route.route_steps[sd->state.route.current_step - 1].x, sd->state.route.route_steps[sd->state.route.current_step - 1].y, 4))
            {
                //ShowStatus("route failed\n");
            }
            else
            {
                sd->state.route.sent_route_move = true;
            }
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
        sd->state.route.sent_route_move = false;

        // calculate steps
        int i, c;
        int temp_x = sd->bl.x, temp_y = sd->bl.y;
        //ShowStatus("path data: path len [%d]\n", sd->state.route.wpd.path_len);
        for (i = c = 0; i < sd->state.route.wpd.path_len; i++)
        {
            temp_x += dirx[sd->state.route.wpd.path[i]];
            temp_y += diry[sd->state.route.wpd.path[i]];
            // ////ShowStatus("path data: [%d] (%d, %d) - dir: %d \n", i, temp_x, temp_y, sd->state.route.wpd.path[i]);
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
        // reset_route(sd);
        //ShowStatus("failed in recalculate, from: %d, %d to: %d, %d\n", sd->bl.x, sd->bl.y, sd->state.route.x, sd->state.route.y);
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
    sd->state.route.sent_route_move = false;
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
