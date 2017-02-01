/*
 * Copyright 2015 Hannes Ellinger
 *
 * This file is part of Project Ubertooth.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "ubertooth_fifo.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

fifo_t* fifo_init()
{
	fifo_t* fifo = (fifo_t*)malloc(sizeof(fifo_t));
  fifo->lock = (sem_t*)malloc(sizeof(sem_t));
  fifo->pkt_present = (sem_t*)malloc(sizeof(sem_t));

  printf("Initialized lock %d\n", fifo->lock);
  printf("Initialized pkt_present %d\n", fifo->pkt_present);

	fifo->read_ptr = 0;
	fifo->write_ptr = 0;

  if ( sem_init(fifo->lock, 0, 1) < 0 )
  {
    fprintf(stderr, "FIFO semaphore did not initialize");
  }

  if ( sem_init(fifo->pkt_present, 0, 0) < 0 )
  {
    fprintf(stderr, "FIFO semaphore did not initialize");
  }

	return fifo;
}

void fifo_inc_write_ptr(fifo_t* fifo)
{
 // sem_wait( fifo->lock );
	if (fifo->read_ptr != (fifo->write_ptr+1) % FIFO_SIZE) {
		fifo->write_ptr = (fifo->write_ptr+1) % FIFO_SIZE;
    //printf("Posting\n");
    sem_post( fifo->pkt_present );
	} else {
		fprintf(stderr, "FIFO overflow, packet discarded\n");
	}
 // sem_post( fifo->lock );
}

uint8_t fifo_empty(fifo_t* fifo)
{
	return (fifo->read_ptr == fifo->write_ptr);
}

int wait_for_pkt(fifo_t* fifo, int seconds)
{
  int rtn = 0;
  if (seconds < 0)
  {
    sem_wait( fifo->pkt_present );  
  }
  else
  {
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = 0L;
    rtn = sem_timedwait( fifo->pkt_present, &ts );
  }
  return rtn;
}

void fifo_push(fifo_t* fifo, const usb_pkt_rx* packet)
{
  //sem_wait( fifo->lock );
	memcpy(&(fifo->packets[fifo->write_ptr]), packet, sizeof(usb_pkt_rx));

	fifo_inc_write_ptr(fifo);
  //sem_post( fifo->lock );
}

usb_pkt_rx fifo_pop(fifo_t* fifo)
{
  //sem_wait( fifo->lock );
	size_t selected = fifo->read_ptr;

	fifo->read_ptr = (fifo->read_ptr+1) % FIFO_SIZE;

  usb_pkt_rx rx = fifo->packets[selected]; 
  //sem_post( fifo->lock );
	return rx;
}

usb_pkt_rx* fifo_get_write_element(fifo_t* fifo)
{
  //sem_wait( fifo->lock );
  usb_pkt_rx* p_rx = &(fifo->packets[fifo->write_ptr]);
  //sem_post( fifo->lock );
	return p_rx;
}
