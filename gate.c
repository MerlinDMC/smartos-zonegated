/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

#include <strings.h>
#include <zdoor.h>

#include "gate.h"

const char *gate_service_name = "_joyent_sshd_key_is_authorized";
const char *gate_result_login_block = "0";
const char *gate_result_login_allow = "1";

nvlist_t *gate_nvl;

zdoor_result_t *
zdoor_gate_handler(zdoor_cookie_t *cookie, char *argp, size_t arpg_sz)
{
  zdoor_result_t *result;
  int res;

  // make compiler happy the stupid way
  arpg_sz = arpg_sz;

  result = malloc(sizeof(zdoor_result_t));

  // clear fields
  result->zdr_size = 2;
  result->zdr_data = NULL;

#ifdef  DEBUG
  fprintf(stdout, "got a zdoor request on zone %s for fingerprint: %s\n", cookie->zdc_zonename, argp);
#endif//DEBUG

  // send the fingerprint data to the api and forward the auth result
  res = request_send_login_event(cookie->zdc_zonename, argp);

#ifdef  DEBUG
  fprintf(stdout, "http request done result: %d\n", res);
#endif//DEBUG

  // a code of 2xx means we're good to let the login through
  if (res >= 200 && res < 300) {
    result->zdr_data = strdup(gate_result_login_allow);
  } else {
    result->zdr_data = strdup(gate_result_login_block);
  }

#ifdef  DEBUG
  fprintf(stdout, "returning zdoor result: %s (%d)\n", result->zdr_data, (int)result->zdr_size);
#endif//DEBUG

  return result;
}

int
gate_list_init()
{
  return nvlist_alloc(&gate_nvl, NV_UNIQUE_NAME, 0);
}

void
gate_list_free()
{
  nvlist_free(gate_nvl);
}

boolean_t
gate_exists(const char *zonename)
{
  return nvlist_exists(gate_nvl, zonename);
}

int
gate_attach(const char *zonename)
{
  zdoor_handle_t *zdoor_id;
  int res;

  zdoor_id = malloc(sizeof(zdoor_handle_t));

  // save address of zdoor handle in the nvlist
  res = nvlist_add_uint64(gate_nvl, zonename, (uint64_t)&zdoor_id);
  if (res != 0) {
    fprintf(stderr, "failed adding the zdoor handle for zone: %s", zonename);

    // adding the address failed so we free the memory
    // and don't allocate the actual zdoor
    free(zdoor_id);

    return res;
  }

  // init the zdoor handle
  *zdoor_id = zdoor_handle_init();

  // and create the door for the sshd service
  res = zdoor_open(*zdoor_id, zonename, (char*)gate_service_name,
                   NULL, zdoor_gate_handler);
#ifdef  DEBUG
  if (res == 0) {
    fprintf(stderr, "sucessfully attached a door in zone: %s\n", zonename);
  }
#endif//DEBUG

  return res;
}

int
gate_destroy(const char *zonename)
{
  zdoor_handle_t *zdoor_id;
  int res;

  res = nvlist_lookup_uint64(gate_nvl, zonename, (uint64_t *)&zdoor_id);

  // if we have a zdoor associated with this zonename
  if (res == 0) {
#ifdef  DEBUG
    fprintf(stderr, "destroying door in zone: %s\n", zonename);
#endif//DEBUG

    // XXX: libzdoor handles the removal on it's own
    // destroy and free up the handle
    // zdoor_handle_destroy(*zdoor_id);

    free(zdoor_id);
  }

  // remove the zdoor handle from the list
  return nvlist_remove_all(gate_nvl, zonename);
}
