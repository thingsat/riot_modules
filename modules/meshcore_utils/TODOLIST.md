# TODOLIST


Priority list

* [ ] decode advert packet https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#L26
* [ ] print advert packet (debug)
* [ ] add/update a list of ContactInfo from an advert packet
* [ ] decode group datagram https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#L239
* [ ] print group datagram (debug)
* [ ] decode group datagram using a given list of ChannelDetails for decryphering the message
* [ ] print group datagram (debug)
* [ ] encode a group datagram using a ChannelDetails
* [ ] encode a direct packet using a ContactInfo + private key with flooding https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#L177
* [ ] encode a direct packet using a ContactInfo + private key with path for routing
* [ ] encode a advert packet using a ContactInfo + private key
* [ ] decode a DISCOVER_REQ https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#L263
* [ ] print a DISCOVER_REQ https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#L263
* [ ] encode a DISCOVER_REQ https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#L263
* [ ] decode a DISCOVER_RESP https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#272
* [ ] print a DISCOVER_RESP https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#272
* [ ] encode a DISCOVER_RESP https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1#272
* [ ] add loop detection (see the parameters for [set/get loop.detect](https://github.com/meshcore-dev/MeshCore/blob/main/docs/cli_commands.md?plain=1#L467) CLI)

## loop detection
**Parameters:**
- `state`: 
  - `off`: no loop detection is performed
  - `minimal`: packets are dropped if repeater's ID/hash appears 4 or more times (1-byte), 2 or more (2-byte), 1 or more (3-byte)
  - `moderate`: packets are dropped if repeater's ID/hash appears 2 or more times (1-byte), 1 or more (2-byte), 1 or more (3-byte)
  - `strict`: packets are dropped if repeater's ID/hash appears 1 or more times (1-byte), 1 or more (2-byte), 1 or more (3-byte)
  
**Default:** `off`

**Note:** When it is enabled, repeaters will now reject flood packets which look like they are in a loop. This has been happening recently in some meshes when there is just a single 'bad' repeater firmware out there (probably some forked or custom firmware). If the payload is messed with, then forwarded, the same packet ends up causing a packet storm, repeated up to the max 64 hops. This feature was added in firmware 1.14

**Example:** If preference is `loop.detect minimal`, and a 1-byte path size packet is received, the repeater will see if its own ID/hash is already in the path. If it's already encoded 4 times, it will reject the packet.  If the packet uses 2-byte path size, and repeater's own ID/hash is already encoded 2 times, it rejects. If the packet uses 3-byte path size, and the repeater's own ID/hash is already encoded 1 time, it rejects. 


* https://github.com/meshcore-dev/MeshCore/blob/main/docs/packet_format.md
* https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1
