# mini_ssh (minimal SSH-like protocol)

This folder contains a tiny educational implementation of a local-only SSH-like protocol and a simple protocol description.

Files
- `PROTOCOL.md` — protocol specification
- `server.cpp` — TCP server that listens on 127.0.0.1:2222 and authenticates using `check_key_username_pair`
- `client.cpp` — client that connects, authenticates, and proxies a local terminal to the server shell

Build

These are example commands; adjust include/library paths for your environment. The server links the project `db` code which depends on libpqxx.

Compile server (requires libpqxx/libpq):

```bash
g++ -std=c++17 -I.. -I../.. mini_ssh/server.cpp db/db.cpp -lpqxx -lpq -o mini_ssh_server
```

Compile client:

```bash
g++ -std=c++17 mini_ssh/client.cpp -o mini_ssh_client
```

Run

1. Make sure your PostgreSQL DB is running and the `db` code is configured correctly (see `db/db.cpp` for connection string).
2. Start the server:

```bash
./mini_ssh_server
```

3. From another terminal, run the client with a username and a key (as stored in the DB):

```bash
./mini_ssh_client alice somekeyvalue
```

If authentication succeeds, you'll get an interactive `/bin/sh` shell over the TCP connection.

Security reminder
---------------
This is unencrypted and intended only for local testing. Do not expose the server to untrusted networks.
