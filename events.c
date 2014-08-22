/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

#include <sys/zone.h>
#include <libsysevent.h>
#include <libzonecfg.h>
#include <strings.h>
#include <unistd.h>

#include "gate.h"

// XXX: maybe use zonecfg_notify_bind()
// https://github.com/joyent/illumos-joyent/blob/master/usr/src/lib/libzdoor/common/zdoor.c

static int
sysev_evc_handler(sysevent_t *ev, void *cookie)
{
  char *name, *zonename, *oldstate, *newstate;
  nvlist_t *nvlist;
  nvpair_t *curr, *next;

  cookie = cookie; // SILENCE!

  if (sysevent_get_attr_list(ev, &nvlist) != 0) {
    // XXX Error
    return (1);
  }

  // loop through event nvpairs to get our interesting values
  curr = nvlist_next_nvpair(nvlist, NULL);
  while (curr != NULL) {
    // all fields we're interested in are strings
    if (nvpair_type(curr) == DATA_TYPE_STRING) {
      name = nvpair_name(curr);

      if (strcmp(name, ZONE_CB_NAME) == 0) {
        nvpair_value_string(curr, &zonename);
      } else if (strcmp(name, ZONE_CB_NEWSTATE) == 0) {
        nvpair_value_string(curr, &newstate);
      } else if (strcmp(name, ZONE_CB_OLDSTATE) == 0) {
        nvpair_value_string(curr, &oldstate);
      }
    }

    next = nvlist_next_nvpair(nvlist, curr);
    curr = next;
  }

  // if it is a boot into running we need to enable the gate
  if (strcmp(oldstate, ZONE_EVENT_READY) == 0 &&
      strcmp(newstate, ZONE_EVENT_RUNNING) == 0) {
    gate_attach(zonename);
  }

  // ignore multiple shutting_down -> shutting_down transitions
  if (strcmp(oldstate, newstate) != 0) {
    char zonebrand[MAXNAMELEN];

    // get brand for zone
    if (zone_get_brand(zonename, zonebrand, sizeof (zonebrand)) == Z_OK) {
      request_send_state_event(zonename, zonebrand, oldstate, newstate);
    } else {
      request_send_state_event(zonename, NULL, oldstate, newstate);
    }
  }

  return (0);
}

int
zone_events_init()
{
  int res;
  evchan_t *ch;
  char subid[16];

  if ((res = sysevent_evc_bind(ZONE_EVENT_CHANNEL, &ch, 0)) != 0) {
    fprintf(stderr, "failed to bind to sysevent channel: %d\n", res);

    return (-1);
  }

  snprintf(subid, sizeof (subid), "gate-%ld", (long int)getpid());

  if ((res = sysevent_evc_subscribe(ch, subid, ZONE_EVENT_STATUS_CLASS,
      sysev_evc_handler, (void *)ZONE_EVENT_CHANNEL, 0)) != 0) {
    fprintf(stderr, "failed to subscribe to channel: %d\n", res);

    return (-1);
  }

  return (0);
}