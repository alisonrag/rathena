// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

#include <map/autoattack.hpp>

TIMER_FUNC(autoattack_timer)
{
	map_session_data* sd = NULL;
	sd = map_id2sd(id);

	if (sd == NULL || pc_isdead(sd) || !sd->state.auto_attack.enabled)
		return 0;

	if (sd->state.auto_attack.enabled)
	{
		unit_stop_attack(&sd->bl);
		auto_attack_iterate(sd);
		if (DIFF_TICK(sd->state.auto_attack.delay, gettick()) > 0) {
			clif_authfail_fd(sd->fd, 15);
			return 0;
		}
		else {
			sd->state.auto_attack.timer = add_timer(gettick() + 1000, autoattack_timer, sd->bl.id, 0);
			sd->state.auto_attack.delay = gettick() + 1000;
		}
	}
	return 0;
}

ACMD_FUNC(autoattack)
{
	if (map_getmapflag(sd->bl.m, MF_NOAUTOATTACK)) {
		clif_displaymessage(sd->fd,"sorry but auto attack cannot be activated on this map.");
		return -1;
	}

	int auto_attack_enabled;
	int auto_attack_can_move; // Canmove 0 notmove 1 move 2 disable
	int min_hp_percent;
	int min_sp_percent;
	int min_hp_percent_fly_wing;
	
	ShowStatus("message: %s", message);
	if (!message || !*message || sscanf(message, "%d %d %d %d %d",&auto_attack_enabled, &auto_attack_can_move, &min_hp_percent, &min_sp_percent, &min_hp_percent_fly_wing) < 5)
	{
        clif_displaymessage(sd->fd, "*-----------------------[ :+: Auto Mode :+: ]------------------------*");
		clif_displaymessage(sd->fd, "Use o comando: @autoattack <OFF/ON> 0-1 <AutoWalk> 0-2 <PotHP> 1-100 <PotSP> 1-100 <FlyWingHP> 1-100.");
        clif_displaymessage(sd->fd, "Auto attack config. AutoWalk(1=ON 0/2=OFF/STAYALERT)");
	    clif_displaymessage(sd->fd, "*-----------------------[ :+: Hotkey :+: ]------------------------*");
        clif_displaymessage(sd->fd, "F1-F5 = Skill Ofensiva");
        clif_displaymessage(sd->fd, "F6-F8 = Buff");
        clif_displaymessage(sd->fd, "F9 = Asa de Mosca(HpFuga)");
        clif_displaymessage(sd->fd, "hotkey 2-1 = HP Pot");
        clif_displaymessage(sd->fd, "hotkey 2-2 = SP Pot");
        clif_displaymessage(sd->fd, "hotkey 2-3 = Blessing Skill Scroll");
        clif_displaymessage(sd->fd, "hotkey 2-4 = Agi Skill Scroll");
        clif_displaymessage(sd->fd, "hotkey 2-5 = Concentration Potion");
        clif_displaymessage(sd->fd, "hotkey 2-6 = Awake Potion");
        clif_displaymessage(sd->fd, "hotkey 2-7 = Berserk Potion");
        clif_displaymessage(sd->fd, "^FF0000 Info^000000 Priest/HighPriest ative PotHP para AutoHeal");
        clif_displaymessage(sd->fd, "*-----------------------[ :+: End :+: ]------------------------*");
		return -1;
	}

	nullpo_retr(-1, sd);

	if (auto_attack_enabled)
	{
		sd->state.auto_attack.enabled = auto_attack_enabled;
		sd->state.auto_attack.can_move = auto_attack_can_move;
		sd->state.auto_attack.can_attack = auto_attack_enabled; // TODO: check the difference between auto_attack_enabled and auto_attack_can_attack
		sd->state.auto_attack.use_potion_hp_min_percent = (min_hp_percent) ? min_hp_percent : 0;
		sd->state.auto_attack.use_potion_sp_min_percent = (min_sp_percent) ? min_sp_percent : 0;
		sd->state.auto_attack.use_fly_wing_hp_min_percent = (min_hp_percent_fly_wing) ? min_hp_percent_fly_wing : 0;
		sd->state.autoloot = 10000;
		sd->state.auto_attack.delay = gettick() + 1000;
		clif_hotkeys_send(sd,1);
		ShowStatus("hp: %d - sp: %d - fw: %d\n",min_hp_percent,min_sp_percent,min_hp_percent_fly_wing);
		(auto_attack_can_move) ? clif_displaymessage(sd->fd, "Auto Attack - Automatic: ON.") : clif_displaymessage(fd, "Auto Attack - Retaliate: ON.");

		sd->state.auto_attack.timer = add_timer(gettick() + 1000, autoattack_timer, sd->bl.id, 0);
	} 
	else // disable system
	{
		disable_auto_attack(sd);
	}

	return 0;
}
