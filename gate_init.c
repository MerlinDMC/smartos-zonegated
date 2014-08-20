/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

#include <zone.h>

#include "gate.h"

int
zdoor_gate_init()
{
  zoneid_t *zids = NULL;
  uint_t nzents, nzents_saved, i;

  if (zone_list(NULL, &nzents) != 0) {
    fprintf(stderr, "failed to get the current zones list");

    return (-1);
  }

again:
  if (nzents == 0) {
    return (0);
  }

  zids = calloc(nzents, sizeof (zoneid_t));
  nzents_saved = nzents;

  if (zone_list(zids, &nzents) != 0) {
    fprintf(stderr, "failed to get the current zones list");

    free(zids);

    return (-1);
  }

  if (nzents != nzents_saved) {
    /* list changed, try again */
    free(zids);
    goto again;
  }

  for (i = 0; i < nzents; i++) {
    char name[ZONENAME_MAX];

    if (zids[i] == GLOBAL_ZONEID ||
        getzonenamebyid(zids[i], name, sizeof (name)) < 0) {
      continue;
    }

#ifdef  DEBUG
    fprintf(stderr, "found zone: %02d - %s\n", zids[i], name);
#endif//DEBUG

    // attach the gate and ignore failures for now
    gate_attach(name);
  }

  free(zids);

  return (0);
}
