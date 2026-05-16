/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:01:15 by aryahi            #+#    #+#             */
/*   Updated: 2026/05/16 17:43:43 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	start_simulation(t_shared *shared, int i, int j)
{
	pthread_mutex_lock(&shared->queue_mutex);
	shared->simulation_start_time = get_current_time_in_ms();
	while (i < shared->number_of_coders)
	{
		shared->coders[i].last_compile_start = shared->simulation_start_time;
		shared->coders[i].ticket = i;
		if (pthread_create(&shared->coders[i].thread_id, NULL, &coder_routine,
				&shared->coders[i]))
		{
			shared->simulation_state = false;
			pthread_mutex_unlock(&shared->queue_mutex);
			j = -1;
			while (++j < i)
				pthread_join(shared->coders[j].thread_id, NULL);
			return (0);
		}
		i++;
	}
	pthread_mutex_unlock(&shared->queue_mutex);
	monitor_routine(shared);
	i = -1;
	while (++i < shared->number_of_coders)
		pthread_join(shared->coders[i].thread_id, NULL);
	return (1);
}

int	main(int argc, char **argv)
{
	t_shared	shared;
	int			i;

	memset(&shared, 0, sizeof(t_shared));
	if (argc != 9)
	{
		printf("ERROR: Please enter 8 arguments...\n");
		return (1);
	}
	if (!init_shared_env(&shared, argv))
		return (1);
	if (!start_simulation(&shared, 0, 0))
		printf("ERROR: Failed to create threads.\n");
	i = -1;
	while (++i < shared.number_of_coders)
	{
		pthread_mutex_destroy(&shared.coders[i].coder_mutex);
		pthread_mutex_destroy(&shared.dongles[i]);
	}
	pthread_mutex_destroy(&shared.print_mutex);
	pthread_mutex_destroy(&shared.queue_mutex);
	pthread_cond_destroy(&shared.queue_cond);
	cleanup_memory(&shared);
	return (0);
}
