/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 15:37:13 by aryahi            #+#    #+#             */
/*   Updated: 2026/05/16 18:17:36 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	has_conflict(t_shared *s, t_coder *coder, int my)
{
	int		i;
	t_coder	*o;

	i = -1;
	while (++i < s->queue_size)
	{
		if (i != my && has_higher_priority(s, i, my))
		{
			o = s->queue[i];
			if ((o->min_dongle == coder->min_dongle
					|| o->max_dongle == coder->max_dongle
					|| o->min_dongle == coder->max_dongle
					|| o->max_dongle == coder->min_dongle)
				&& s->dongle_states[o->min_dongle] == 0
				&& s->dongle_states[o->max_dongle] == 0)
				return (true);
		}
	}
	return (false);
}

bool	get_sim_state(t_shared *shared)
{
	bool	state;

	pthread_mutex_lock(&shared->print_mutex);
	state = shared->simulation_state;
	pthread_mutex_unlock(&shared->print_mutex);
	return (state);
}

void	set_sim_state(t_shared *shared, bool new_state)
{
	pthread_mutex_lock(&shared->print_mutex);
	shared->simulation_state = new_state;
	pthread_mutex_unlock(&shared->print_mutex);
}

bool	can_take_dongles(t_coder *coder, long long now)
{
	int			my;
	t_shared	*s;

	s = coder->shared_env;
	if (s->dongle_states[coder->min_dongle] == 1
		|| s->dongle_states[coder->max_dongle] == 1
		|| now < s->cooldowns[coder->min_dongle]
		|| now < s->cooldowns[coder->max_dongle])
		return (false);
	my = 0;
	while (my < s->queue_size && s->queue[my] != coder)
		my++;
	if (has_conflict(s, coder, my))
		return (false);
	return (true);
}
