# Utility functions for Meshtastic LoRa mesh networks

## Context

Meshtastic is an off-grid messaging system using [affortable LoRa endpoints](https://meshtastic.org/docs/hardware/devices) for building dynamic mesh networks over kilometers. LoRa endpoints (aka nodes) [forward messages to the next node](https://meshtastic.org/blog/why-meshtastic-uses-managed-flood-routing/) to distribute them over the network. Messages can be encrypted using [AES256-CTR](https://meshtastic.org/docs/overview/encryption/) expect from radio-amateur bands. Internet-connected relay nodes enable the conversation to move online too using MQTT brokers.

Meshtastic can be used for operating disaster networks.

https://meshtastic.org

## Description

This module provides preliminary functions for encoding/decoding Meshtastic messages.

## ToDoList

* [x] decode row messages
* [ ] encode row messages
* [ ] encrypt/decrypt protobuf payloads ([AES256-CTR](https://doc.riot-os.org/ctr_8h.html#details))
* [ ] marshall/unmarshall [protobuf payloads](https://github.com/meshtastic/protobufs/tree/master/meshtastic) [docs](https://buf.build/meshtastic/protobufs/docs/main:meshtastic#)

## References

* [Why Meshtastic Uses Managed Flood Routing](https://meshtastic.org/blog/why-meshtastic-uses-managed-flood-routing/)
* [Supported Devices](https://meshtastic.org/docs/hardware/devices/)
* [Protobufs](https://github.com/meshtastic/protobufs/tree/master/meshtastic) [docs](https://buf.build/meshtastic/protobufs/docs/main:meshtastic#)
