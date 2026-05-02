/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 22:53:09 by aryahi            #+#    #+#             */
/*   Updated: 2026/05/02 18:23:37 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_coder_status(t_shared *shared, int *f_coders, int i)
{
	pthread_mutex_lock(&shared->coders[i].coder_mutex);
	if (shared->coders[i].compile_count >= shared->number_of_compiles_required)
	{
		(*f_coders)++;
		pthread_mutex_unlock(&shared->coders[i].coder_mutex);
		return (0);
	}
	if (get_current_time_in_ms()
		- shared->coders[i].last_compile_start >= shared->time_to_burnout)
	{
		print_log(shared, shared->coders[i].id, "burned out");
		set_sim_state(shared, false);
		pthread_mutex_unlock(&shared->coders[i].coder_mutex);
		pthread_mutex_lock(&shared->queue_mutex);
		pthread_cond_broadcast(&shared->queue_cond);
		pthread_mutex_unlock(&shared->queue_mutex);
		return (1);
	}
	pthread_mutex_unlock(&shared->coders[i].coder_mutex);
	return (0);
}

void	monitor_routine(t_shared *shared)
{
	int	i;
	int	f_coders;

	while (get_sim_state(shared) == true)
	{
		i = -1;
		f_coders = 0;
		while (++i < shared->number_of_coders)
			if (check_coder_status(shared, &f_coders, i))
				return ;
		if (f_coders == shared->number_of_coders)
		{
			set_sim_state(shared, false);
			pthread_mutex_lock(&shared->queue_mutex);
			pthread_cond_broadcast(&shared->queue_cond);
			pthread_mutex_unlock(&shared->queue_mutex);
			return ;
		}
		usleep(1000);
	}
}
