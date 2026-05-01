/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:01:10 by aryahi            #+#    #+#             */
/*   Updated: 2026/05/01 00:57:39 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H
# include <pthread.h>
# include <stdbool.h>
# include <stddef.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_coder	t_coder;

typedef struct s_shared
{
	int					number_of_coders;
	int					time_to_burnout;
	int					time_to_compile;
	int					time_to_debug;
	int					time_to_refactor;
	int					number_of_compiles_required;
	int					total_finished_coders;
	int					dongle_cooldown;
	int					scheduler;
	int					queue_size;
	int					*dongle_states;
	long long			*cooldowns;
	long long			simulation_start_time;
	unsigned long long	ticket_counter;
	bool				simulation_state;

	pthread_mutex_t		queue_mutex;
	pthread_mutex_t		print_mutex;
	pthread_cond_t		queue_cond;

	t_coder				*coders;
	t_coder				**queue;
	pthread_mutex_t		*dongles;

}						t_shared;

typedef struct s_coder
{
	int					id;
	int					min_dongle;
	int					max_dongle;
	int					compile_count;
	long long			last_compile_start;
	unsigned long long	ticket;
	pthread_t			thread_id;

	pthread_mutex_t		coder_mutex;

	t_shared			*shared_env;

}						t_coder;

bool					has_finished_compiling(t_shared *shared,
							t_coder *coder);
bool					can_take_dongles(t_coder *coder, long long now);
bool					init_shared_env(t_shared *shared, char **argv);
bool					get_sim_state(t_shared *shared);
int						has_higher_priority(t_shared *s, int i, int j);
int						start_simulation(t_shared *shared, int i, int j);
int						cleanup_memory(t_shared *shared);
long long				get_current_time_in_ms(void);
void					print_log(t_shared *shared, int coder_id, char *status);
void					enqueue_coder(t_shared *shared, t_coder *coder);
void					ft_usleep(long long time_to_sleep, t_shared *shared);
void					dequeue_coder(t_shared *shared, t_coder *coder);
void					set_sim_state(t_shared *shared, bool new_state);
void					heapify_down(t_shared *s, int index);
void					heapify_up(t_shared *s, int index);
void					monitor_routine(t_shared *shared);
void					*coder_routine(void *arg);

#endif
