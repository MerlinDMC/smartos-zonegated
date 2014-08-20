# Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
# See LICENSE file for copyright and license details.

CURL_VERSION = 7.37.1

CC = gcc

PWD := $(shell pwd)

CFILES = main.c request.c gate.c gate_init.c events.c
OBJS = $(CFILES:%.c=%.o)

PROGS = zonegated
SLIBS = $(PWD)/curl-$(CURL_VERSION)/lib/.libs/libcurl.a

CFLAGS = -I$(PWD) -I$(PWD)/curl-$(CURL_VERSION)/include -Wall -Wextra -Werror -g -O2 -DCURL_STATICLIB
LDFLAGS = -lnvpair -lzdoor -lsocket -lnsl -lsysevent

%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $<

zonegated:	$(SLIBS) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SLIBS)

.PHONY:	clean
clean:
	rm -f $(PROGS) $(OBJS)
	rm -rf $(PWD)/curl-$(CURL_VERSION)

.PHONY:	all
all:	$(PROGS)

$(SLIBS): $(PWD)/curl-$(CURL_VERSION)
	cd $(PWD)/curl-$(CURL_VERSION); ./configure --without-zlib --enable-http --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-manual --disable-libcurl-option --disable-threaded-resolver --without-libssh2 --without-ssl --without-gnutls --disable-verbose --disable-shared --without-libidn
	make -C $(PWD)/curl-$(CURL_VERSION) -j8

$(PWD)/curl-$(CURL_VERSION):
	wget -c -O- http://curl.haxx.se/download/curl-$(CURL_VERSION).tar.gz | tar -xzp -C $(PWD)
