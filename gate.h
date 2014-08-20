/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

#ifndef _GATE_H
#define _GATE_H

#include <rpc/types.h>
#include <sys/nvpair.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum zgate_exit_codes {
  ZGATE_SUCCESS = 0,
  ZGATE_ERROR = 1,
  ZGATE_USAGE_ERROR = 3
} zgate_exit_codes_t;

char *remote_url;

int request_send_login_event(char*, char*);
int request_send_state_event(char*, char*, char*);

int zone_events_init();

int zdoor_gate_init();

int gate_attach(const char*);

#endif//_GATE_H
