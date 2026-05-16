/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 15:34:07 by aryahi            #+#    #+#             */
/*   Updated: 2026/05/16 17:10:12 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	wait_for_dongles(t_coder *coder, t_shared *shared, long long now)
{
	while (1)
	{
		if (get_sim_state(shared) == false)
		{
			dequeue_coder(shared, coder);
			pthread_mutex_unlock(&shared->queue_mutex);
			return (false);
		}
		now = get_current_time_in_ms();
		if (can_take_dongles(coder, now))
			break ;
		if (shared->dongle_states[coder->min_dongle] == 0
			&& shared->dongle_states[coder->max_dongle] == 0
			&& (now < shared->cooldowns[coder->min_dongle]
				|| now < shared->cooldowns[coder->max_dongle]))
		{
			pthread_mutex_unlock(&shared->queue_mutex);
			usleep(50);
			pthread_mutex_lock(&shared->queue_mutex);
		}
		else
			pthread_cond_wait(&shared->queue_cond, &shared->queue_mutex);
	}
	return (true);
}

static bool	acquire_dongles(t_coder *coder, t_shared *shared)
{
	pthread_mutex_lock(&shared->queue_mutex);
	enqueue_coder(shared, coder);
	if (!wait_for_dongles(coder, shared, 0))
		return (false);
	dequeue_coder(shared, coder);
	shared->dongle_states[coder->min_dongle] = 1;
	shared->dongle_states[coder->max_dongle] = 1;
	pthread_mutex_unlock(&shared->queue_mutex);
	pthread_mutex_lock(&shared->dongles[coder->min_dongle]);
	pthread_mutex_lock(&shared->dongles[coder->max_dongle]);
	return (true);
}

static void	compile_and_release(t_coder *coder, t_shared *shared)
{
	print_log(shared, coder->id, "has taken a dongle");
	print_log(shared, coder->id, "has taken a dongle");
	print_log(shared, coder->id, "is compiling");
	pthread_mutex_lock(&coder->coder_mutex);
	coder->last_compile_start = get_current_time_in_ms();
	pthread_mutex_unlock(&coder->coder_mutex);
	ft_usleep(shared->time_to_compile, shared);
	pthread_mutex_lock(&coder->coder_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->coder_mutex);
	pthread_mutex_unlock(&shared->dongles[coder->min_dongle]);
	pthread_mutex_unlock(&shared->dongles[coder->max_dongle]);
	pthread_mutex_lock(&shared->queue_mutex);
	shared->cooldowns[coder->min_dongle] = get_current_time_in_ms()
		+ shared->dongle_cooldown;
	shared->cooldowns[coder->max_dongle] = get_current_time_in_ms()
		+ shared->dongle_cooldown;
	shared->dongle_states[coder->min_dongle] = 0;
	shared->dongle_states[coder->max_dongle] = 0;
	pthread_cond_broadcast(&shared->queue_cond);
	pthread_mutex_unlock(&shared->queue_mutex);
}

static void	*handle_lone_coder(t_coder *coder, t_shared *shared)
{
	pthread_mutex_lock(&shared->dongles[coder->min_dongle]);
	print_log(shared, coder->id, "has taken a dongle");
	while (get_sim_state(shared) == true)
		ft_usleep(10, shared);
	pthread_mutex_unlock(&shared->dongles[coder->min_dongle]);
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_coder		*coder;
	t_shared	*shared;

	coder = (t_coder *)arg;
	shared = coder->shared_env;
	pthread_mutex_lock(&coder->coder_mutex);
	coder->last_compile_start = get_current_time_in_ms();
	pthread_mutex_unlock(&coder->coder_mutex);
	if (shared->number_of_coders == 1)
		return (handle_lone_coder(coder, shared));
	if (coder->id % 2 == 0)
		usleep(500);
	while (get_sim_state(shared) == true)
	{
		if (has_finished_compiling(shared, coder))
			break ;
		if (!acquire_dongles(coder, shared))
			break ;
		compile_and_release(coder, shared);
		print_log(shared, coder->id, "is debugging");
		ft_usleep(shared->time_to_debug, shared);
		print_log(shared, coder->id, "is refactoring");
		ft_usleep(shared->time_to_refactor, shared);
	}
	return (NULL);
}
