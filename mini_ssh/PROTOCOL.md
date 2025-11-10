mini_ssh protocol
=================

Overview
--------
mini_ssh is a very small, local-only SSH-like protocol intended for testing and educational use. It runs over TCP on localhost and provides a simple username+key authentication. On successful auth the server spawns a shell bound to the TCP socket.

Security note
-------------
- This protocol is NOT secure for production. It's unencrypted and vulnerable to eavesdropping and replay attacks.
- Use only on trusted local machines or inside secure testing environments.

Wire format (binary, big-endian where specified)
------------------------------------------------

Client -> Server: AUTH packet
- 1 byte: version (0x01)
- 1 byte: auth type (0x01 = username+key)
- 2 bytes: username length (uint16)
- 2 bytes: key length (uint16)
- N bytes: username (UTF-8)
- M bytes: key (UTF-8)

Server -> Client: AUTH-REPLY
- 1 byte: status (0x00 = fail, 0x01 = success)
- If success, the server immediately starts an interactive shell on the same TCP connection.

Examples
--------
- Client connects to 127.0.0.1:2222
- Client sends an AUTH packet (version=1, auth-type=1) containing username and key from the database
- Server validates using the existing database function `check_key_username_pair(username,key)`
- Server replies with 0x01 (success) or 0x00 (fail)
- On success, client switches to raw mode and forwards stdin/stdout to the socket to provide an interactive shell

Notes for implementers
----------------------
- Keep packets small; username/key lengths limited to 65535 bytes (uint16)
- Always validate lengths before reading to avoid resource exhaustion
- Use localhost binding (127.0.0.1) unless you explicitly want remote access
