/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/19 16:01:06 by aryahi            #+#    #+#             */
/*   Updated: 2026/05/11 18:41:50 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	checker_atoi(char *str)
{
	long long	res;
	int			i;

	if (str == NULL || *str == '-')
		return (-1);
	i = 0;
	res = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (-1);
		res = res * 10 + (str[i++] - '0');
		if (res > 2147483647)
			return (-1);
	}
	return ((int)res);
}

static int	set_inputs(t_shared *shared, char **argv)
{
	shared->number_of_coders = checker_atoi(argv[1]);
	shared->time_to_burnout = checker_atoi(argv[2]);
	shared->time_to_compile = checker_atoi(argv[3]);
	shared->time_to_debug = checker_atoi(argv[4]);
	shared->time_to_refactor = checker_atoi(argv[5]);
	shared->number_of_compiles_required = checker_atoi(argv[6]);
	shared->dongle_cooldown = checker_atoi(argv[7]);
	if (shared->number_of_coders <= 0 || shared->number_of_coders > 1000
		|| shared->time_to_burnout <= 0 || shared->time_to_compile <= 0
		|| shared->time_to_debug <= 0 || shared->time_to_refactor <= 0
		|| shared->number_of_compiles_required <= 0
		|| shared->dongle_cooldown < 0)
	{
		printf("ERROR: Invalid input.\n");
		return (-1);
	}
	if (argv[8] != NULL && strcmp(argv[8], "fifo") == 0)
		shared->scheduler = 0;
	else if (argv[8] != NULL && strcmp(argv[8], "edf") == 0)
		shared->scheduler = 1;
	else
		return (-2);
	return (shared->number_of_coders);
}

static int	set_array(t_shared *shared, int n_coders)
{
	int	i;

	shared->dongle_states = malloc(sizeof(int) * n_coders);
	shared->cooldowns = malloc(sizeof(long long) * n_coders);
	shared->dongles = malloc(sizeof(pthread_mutex_t) * n_coders);
	shared->coders = malloc(sizeof(t_coder) * n_coders);
	shared->queue = malloc(sizeof(t_coder *) * n_coders);
	if (!shared->dongles || !shared->cooldowns || !shared->coders
		|| !shared->dongle_states || !shared->queue)
	{
		printf("ERROR: Mallocing memory error\n");
		return (cleanup_memory(shared));
	}
	i = 0;
	while (i < n_coders)
		shared->dongle_states[i++] = 0;
	return (1);
}

static void	set_coder(t_shared *shared, int n_coders, int l_dongle,
		int r_dongle)
{
	int	i;

	i = 0;
	while (i < n_coders)
	{
		l_dongle = i;
		r_dongle = (i + 1) % n_coders;
		if (l_dongle < r_dongle)
		{
			shared->coders[i].min_dongle = l_dongle;
			shared->coders[i].max_dongle = r_dongle;
		}
		else
		{
			shared->coders[i].min_dongle = r_dongle;
			shared->coders[i].max_dongle = l_dongle;
		}
		shared->coders[i].id = i + 1;
		shared->coders[i].compile_count = 0;
		shared->coders[i].shared_env = shared;
		pthread_mutex_init(&shared->coders[i].coder_mutex, NULL);
		pthread_mutex_init(&shared->dongles[i], NULL);
		shared->cooldowns[i++] = 0;
	}
}

bool	init_shared_env(t_shared *shared, char **argv)
{
	int	n_coders;
	int	l_d;
	int	r_d;

	n_coders = set_inputs(shared, argv);
	if (n_coders == -1)
		return (false);
	else if (n_coders == -2)
	{
		printf("ERROR: Please enter 'fifo' or 'edf'.\n");
		return (false);
	}
	l_d = 0;
	r_d = 0;
	shared->simulation_state = true;
	pthread_mutex_init(&shared->queue_mutex, NULL);
	pthread_mutex_init(&shared->print_mutex, NULL);
	pthread_cond_init(&shared->queue_cond, NULL);
	if (set_array(shared, n_coders) == -1)
		return (false);
	set_coder(shared, n_coders, l_d, r_d);
	return (true);
}
