/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

#include <time.h>
#include <curl/curl.h>

#include "gate.h"

#define LOGIN_EVENT_FMT "{\"zonename\":\"%s\",\"when\":%llu,\"event\":\"login\",\"username\":\"%s\",\"uid\":%u,\"fingerprint\":\"%s\"}"
#define STATE_EVENT_FMT "{\"zonename\":\"%s\",\"when\":%llu,\"event\":\"state\",\"oldstate\":\"%s\",\"newstate\":\"%s\"}"

uint64_t
_microtime()
{
  struct timeval tv;

  gettimeofday(&tv,NULL);

  return tv.tv_sec * 1000000ull + tv.tv_usec;
}

int
_request_post_data(char *data)
{
  CURL *curl;
  CURLcode res;
  long response_code = 0;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, remote_url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "request_send_event() failed: %s\n",
              curl_easy_strerror(res));

      curl_easy_cleanup(curl);

      return -1;
    }

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (res != CURLE_OK) {
      fprintf(stderr, "request_send_event() failed: %s\n",
              curl_easy_strerror(res));

      curl_easy_cleanup(curl);

      return -2;
    }

    curl_easy_cleanup(curl);
  }

  return (int)(response_code & 0xffff);
}

int
request_send_login_event(char *zonename, char *data)
{
#ifdef  DEBUG
  fprintf(stdout, "login event: zone=%s, data=%s\n", zonename, data);
#endif//DEBUG

  char *buf;
  char *datap, *data_usernamep, *data_idp, *data_fingerprintp;
  int res;

  // tokenize the dumb way (we destroy the input buffer)
  data_usernamep = data;
  data_idp = NULL;
  data_fingerprintp = NULL;

  for (datap = data; *datap != 0; datap++) {
    if (*datap == ' ') {
      *datap = 0;
      datap++;

      if (data_idp == NULL) {
        data_idp = datap;
      } else if (data_fingerprintp == NULL) {
        data_fingerprintp = datap;

        // exit the thing as we don't have any more args to split
        break;
      }
    }
  }

  asprintf(&buf, LOGIN_EVENT_FMT, zonename, _microtime(),
           data_usernamep, atol(data_idp), data_fingerprintp);

  res = _request_post_data(buf);

  free(buf);

  return res;
}

int
request_send_state_event(char *zonename, char *oldstate, char *newstate)
{
#ifdef  DEBUG
  fprintf(stdout, "state changed: zone=%s, old=%s, new=%s\n", zonename, oldstate, newstate);
#endif//DEBUG

  char *buf;
  int res;

  asprintf(&buf, STATE_EVENT_FMT, zonename, _microtime(), oldstate, newstate);

  res = _request_post_data(buf);

  free(buf);

  return res;
}
