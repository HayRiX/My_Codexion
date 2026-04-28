/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 15:34:07 by aryahi            #+#    #+#             */
/*   Updated: 2026/04/26 14:00:55 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	acquire_dongles(t_coder *coder, t_shared *shared)
{
	pthread_mutex_lock(&shared->queue_mutex);
	enqueue_coder(shared, coder);
	while (1)
	{
		if (can_take_dongles(coder, get_current_time_in_ms()))
			break ;
		if (shared->dongle_states[coder->min_dongle] == 0
			&& shared->dongle_states[coder->max_dongle] == 0)
		{
			pthread_mutex_unlock(&shared->queue_mutex);
			usleep(500);
			pthread_mutex_lock(&shared->queue_mutex);
		}
		else
			pthread_cond_wait(&shared->queue_cond, &shared->queue_mutex);
	}
	dequeue_coder(shared, coder);
	shared->dongle_states[coder->min_dongle] = 1;
	shared->dongle_states[coder->max_dongle] = 1;
	pthread_mutex_unlock(&shared->queue_mutex);
	pthread_mutex_lock(&shared->dongles[coder->min_dongle]);
	pthread_mutex_lock(&shared->dongles[coder->max_dongle]);
}

static void	compile_and_release(t_coder *coder, t_shared *shared)
{
	print_log(shared, coder->id, "has taken a dongle");
	print_log(shared, coder->id, "has taken a dongle");
	print_log(shared, coder->id, "is compiling");
	pthread_mutex_lock(&coder->time_mutex);
	coder->last_compile_start = get_current_time_in_ms();
	pthread_mutex_unlock(&coder->time_mutex);
	ft_usleep(shared->time_to_compile, shared);
	pthread_mutex_lock(&coder->time_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->time_mutex);
	pthread_mutex_unlock(&shared->dongles[coder->min_dongle]);
	pthread_mutex_unlock(&shared->dongles[coder->max_dongle]);
	pthread_mutex_lock(&shared->queue_mutex);
	shared->cooldowns[coder->min_dongle] = get_current_time_in_ms()
		+ shared->dongle_cooldown;
	shared->cooldowns[coder->max_dongle] = get_current_time_in_ms()
		+ shared->dongle_cooldown;
	shared->dongle_states[coder->min_dongle] = 0;
	shared->dongle_states[coder->max_dongle] = 0;
	pthread_mutex_unlock(&shared->queue_mutex);
	pthread_cond_broadcast(&shared->queue_cond);
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
	pthread_mutex_lock(&shared->queue_mutex);
	pthread_mutex_unlock(&shared->queue_mutex);
	pthread_mutex_lock(&coder->time_mutex);
	coder->last_compile_start = get_current_time_in_ms();
	pthread_mutex_unlock(&coder->time_mutex);
	if (shared->number_of_coders == 1)
		return (handle_lone_coder(coder, shared));
	while (get_sim_state(shared) == true)
	{
		acquire_dongles(coder, shared);
		compile_and_release(coder, shared);
		print_log(shared, coder->id, "is debugging");
		ft_usleep(shared->time_to_debug, shared);
		print_log(shared, coder->id, "is refactoring");
		ft_usleep(shared->time_to_refactor, shared);
	}
	return (NULL);
}
