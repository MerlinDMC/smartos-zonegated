/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

#include <unistd.h>

#include "gate.h"

int
main(int argc, char **argv)
{
  if (argc < 2) {
    errx(ZGATE_USAGE_ERROR, "Usage: %s <remote_url>", argv[0]);
  }

  remote_url = argv[1];

  gate_list_init();

  // init gates for all running zones
  zdoor_gate_init();

  // listen for state changes
  zone_events_init();

  pause();

  gate_list_free();

  return (ZGATE_SUCCESS);
}
