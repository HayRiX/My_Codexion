/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aryahi <aryahi@student.1337.ma>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 15:35:41 by aryahi            #+#    #+#             */
/*   Updated: 2026/04/21 17:12:39 by aryahi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	has_higher_priority(t_shared *s, int i, int j)
{
	t_coder	*c1;
	t_coder	*c2;

	c1 = s->queue[i];
	c2 = s->queue[j];
	if (s->scheduler == 0)
		return (c1->ticket < c2->ticket);
	if (c1->last_compile_start < c2->last_compile_start)
		return (1);
	if (c1->last_compile_start > c2->last_compile_start)
		return (0);
	return (c1->id > c2->id);
}

void	heapify_up(t_shared *s, int index)
{
	int		p;
	t_coder	*temp;

	while (index > 0)
	{
		p = (index - 1) / 2;
		if (has_higher_priority(s, index, p))
		{
			temp = s->queue[index];
			s->queue[index] = s->queue[p];
			s->queue[p] = temp;
			index = p;
		}
		else
			break ;
	}
}

void	heapify_down(t_shared *s, int index)
{
	int		l;
	int		r;
	int		max;
	t_coder	*t;

	while (1)
	{
		l = 2 * index + 1;
		r = 2 * index + 2;
		max = index;
		if (l < s->queue_size && has_higher_priority(s, l, max))
			max = l;
		if (r < s->queue_size && has_higher_priority(s, r, max))
			max = r;
		if (max != index)
		{
			t = s->queue[index];
			s->queue[index] = s->queue[max];
			s->queue[max] = t;
			index = max;
		}
		else
			break ;
	}
}

void	enqueue_coder(t_shared *shared, t_coder *coder)
{
	coder->ticket = shared->ticket_counter++;
	shared->queue[shared->queue_size] = coder;
	shared->queue_size++;
	heapify_up(shared, shared->queue_size - 1);
}

void	dequeue_coder(t_shared *shared, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < shared->queue_size)
	{
		if (shared->queue[i] == coder)
			break ;
		i++;
	}
	if (i == shared->queue_size)
		return ;
	shared->queue[i] = shared->queue[shared->queue_size - 1];
	shared->queue_size--;
	if (i < shared->queue_size)
	{
		heapify_up(shared, i);
		heapify_down(shared, i);
	}
}
