/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 15:38:05 by aryahi            #+#    #+#             */
/*   Updated: 2026/04/22 19:53:18 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	get_current_time_in_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	ft_usleep(long long time_to_sleep, t_shared *shared)
{
	long long	start_time;

	start_time = get_current_time_in_ms();
	while ((get_current_time_in_ms() - start_time) < time_to_sleep)
	{
		if (get_sim_state(shared) == false)
			break ;
		usleep(500);
	}
}

void	print_log(t_shared *shared, int coder_id, char *status)
{
	long long	time;

	pthread_mutex_lock(&shared->print_mutex);
	if (shared->simulation_state == true)
	{
		time = get_current_time_in_ms() - shared->simulation_start_time;
		printf("%lld %d %s\n", time, coder_id, status);
	}
	pthread_mutex_unlock(&shared->print_mutex);
}

int	cleanup_memory(t_shared *shared)
{
	if (shared->coders)
		free(shared->coders);
	if (shared->cooldowns)
		free(shared->cooldowns);
	if (shared->dongles)
		free(shared->dongles);
	if (shared->dongle_states)
		free(shared->dongle_states);
	if (shared->queue)
		free(shared->queue);
	return (-1);
}
