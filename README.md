# zonegated

the butler that holds doors open and asks for permission to let visitors pass

## What

zonegated is a daemon program meant to be run in the global zone that will manage door RPC interfaces for all running zones and intercept libsmartsshd plugin pubkey verification request.

It will use an external API service to verify those pubkey fingerprints and give the result back via the door to the libsmartsshd plugin.

To keep the list of open doors updated it does listen for zone state transitions and opens door RPC interfaces for all zones transitioning to running state.
Those events get forwarded to the API service also.

## API Service

The API service is a simple http service that accepts data to a given URL sent via **POST** method.
All captured events get sent as JSON formatted strings to the API.

Each event has the **zonename** for the zone it occurred and a timestamp in microseconds for **when** the event was served.
The field **event** decides which type of event was triggered.

For now we have two different event types:

### Event: state

Is fired for each state transition that changes the zone state.

```json
{
	"zonename": "a22a731c-a237-4f30-ad86-d89ddaefdc21",
	"when":     "1408557940547451",
	"event":    "state",
	"oldstate": "ready",
	"newstate": "running"
}
```

> *The event does not need a response from the API.*

### Event: login

Is fired when a user tries to log in via ssh and it needs to verify the pubkey fingerprint.

```json
{
	"zonename":    "a22a731c-a237-4f30-ad86-d89ddaefdc21",
	"when":        "1408558270094340",
	"event":       "login",
	"username":    "root",
	"uid":         0,
	"fingerprint": "3c:18:2f:3a:68:ed:44:87:b6:51:dd:53:3b:c0:41:e0"
}
```

The event does wait for a response which will determine if the actual login request will be allowed or blocked.

If the API service response with a statusCode of 2xx the login will be allowed. Every other statusCode will block the user from logging in.

A working example for this API Service is available in `example/node_api_server.js`.

## Building

To build in a smartos zone use a base64 image and install the build-essential package.

	$ git clone git://github.com/MerlinDMC/smartos-zonegated.git zonegated
	$ cd zonegated
	$ make

This will fetch curl and build a minimal static libcurl and create the **zonegated** executable.

> The **zonegated** executable is meant to be run in the SmartOS global zone.  
> It should not need any extra pkgsrc packages installed in the global zone to run.

## Running

	$ zonegated http://127.0.0.1:1337/zonegate

This will run the zonegated service and attach doors to all running zones and bind to zone transitioning events.
> It will not daemonize as it is meant to be run by a companion application (or SMF as a child).

> The service will run even if the API service is not available - if so it will by default disallow all login requests.

Run the provided example API service to have a complete setup:

	$ /usr/node/bin/node example/node_api_server.js

The node API service should now show transitioning events on all zones on the system. And it should get requests if someone tries to authenticate through ssh pubkeys.
