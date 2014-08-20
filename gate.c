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
gate_attach(const char *zonename)
{
  zdoor_handle_t zdoor_id;
  int res;

  // init the zdoor handle
  zdoor_id = zdoor_handle_init();

  // and create the door for the sshd service
  res = zdoor_open(zdoor_id, zonename, (char*)gate_service_name,
                   NULL, zdoor_gate_handler);
#ifdef  DEBUG
  if (res == 0) {
    fprintf(stderr, "sucessfully attached a door in zone: %s\n", zonename);
  }
#endif//DEBUG

  return res;
}
