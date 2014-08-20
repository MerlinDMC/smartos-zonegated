/*
 * Copyright (c) 2014 Daniel Malon <daniel.malon@me.com>
 * See LICENSE file for copyright and license details.
 */

/*
 * This API service will listen on 127.0.0.1:1337 for http requests.
 * The used path for the endpoint is irrelevant.
 *
 * If you want to run that in the SmartOS global zone use the platform node:
 * /usr/node/bin/node
 */

var http = require('http');

http.createServer(function (req, res) {
  var data = '';

  req.on('data', function(chunk) {
    data += chunk.toString();
  });

  req.on('end', function() {
    var resultCode = 200;

    var parsed_data = JSON.parse(data);

    if (parsed_data.event) {
      switch (parsed_data.event) {
        // sshd requests login verification
        case 'login':
          // set a bad response code to disallow all logins by default
          resultCode = 401;

          // if we have the needed fields in the posted data
          if (parsed_data.username && parsed_data.fingerprint) {
            // and it matches our super-duper user-pubkey repository
            if (parsed_data.username === 'root' &&
                parsed_data.fingerprint ===
                  '3c:18:2f:3a:68:ed:44:87:b6:51:dd:53:3b:c0:41:e0') {
              // allow the login by returning a good http response code
              resultCode = 200;
            }
          }
        break;
        // zone state transition
        case 'state':
        break;
      }
    }

    // some cool stuff on the console as we like it.
    console.log({
      request: {
        method: req.method,
        url: req.url
      },
      event: parsed_data
    });

    // be informative and send something back!
    // (only the http status code really matters)
    res.writeHead(resultCode, {'Content-Type': 'text/plain'});
    res.end('');
  });
}).listen(1337, '127.0.0.1');
