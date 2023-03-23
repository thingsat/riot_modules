# LwIP Test


## Makefile

Add the following lines into `$(RIOTBASE)/pkg/lwip/Makefile`

```makefile
## ADD LWIP_APPS                       
LWIP_MODULES +=        lwip_httpclient lwip_altcp_tls
LWIP_MODULES +=        lwip_mqtt

...

## ADD LWIP_APPS
lwip_httpclient:
	mkdir -p $(PKG_SOURCE_DIR)/src/apps/httpclient
	cp $(PKG_SOURCE_DIR)/src/apps/http/http_client.c $(PKG_SOURCE_DIR)/src/apps/httpclient
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/apps/httpclient)

lwip_altcp_tls:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/apps/altcp_tls)
	
lwip_mqtt:
	$(call make_module,$@,$(PKG_SOURCE_DIR)/src/apps/mqtt)
```


## Build

```bash
gmake -j 16 flash term
```

## Network

```bash
> help
help
Command              Description
---------------------------------------
ip                   Send IP packets and listen for packets of certain type
tcp                  Send TCP messages and listen for messages on TCP port
udp                  Send UDP messages and listen for messages on UDP port
ifconfig             List network interfaces
pm                   interact with layered PM subsystem
ps                   Prints information about running threads.
reboot               Reboot the node
version              Prints current RIOT_VERSION

> ifconfig
ifconfig
Iface ET0 HWaddr: a6:31:d6:20:b4:f5 Link: up State: up
       Link type: wired
        inet addr: 0.0.0.0 mask: 0.0.0.0 gw: 0.0.0.0
```
After sharing internet :

```bash
> ifconfig
Iface ET0 HWaddr: a6:31:d6:20:b4:f5 Link: up State: up
        Link type: wired
        inet addr: 192.168.2.9 mask: 255.255.255.0 gw: 192.168.2.1
```

> ps
	pid | name                 | state    Q | pri | stack  ( used) ( free) | base addr  | current     
	  - | isr_stack            | -        - |   - |    512 (  240) (  272) | 0x20000000 | 0x200001c8
	  2 | lwip_netdev_mux      | bl rx    _ |   3 |   1024 (  508) (  516) | 0x2000682c | 0x20006b7c 
	  3 | tcpip_thread         | bl mbox  _ |   1 |   1024 (  604) (  420) | 0x20004a90 | 0x20004da4 
	    | SUM                  |            |     |   4096 ( 2236) ( 1860)


## TCP test

Host
```bash
(cd public_html; python -m http.server 8000)
```

http://localhost:8000

RIOT

```bash
> tcp disconnect
tcp disconnect

> tcp connect 192.168.2.1:8000
tcp connect 192.168.2.1:8000

> tcp send "47 45 54 20 2F 20 48 54 54 50 2F 31 2E 31 0d 0a 68 6F 73 74 3A 20 31 37 32 2E 32 30 2E 31 30 2E 36 0d 0a 0d 0a"
tcp send "47 45 54 20 2F 20 48 54 54 50 2F 31 2E 31 0d 0a 68 6F 73 74 3A 20 31 37 32 2E 32 30 2E 31 30 2E 36 0d 0a 0d 0a"
Success: send 37 byte over TCP to server
Received TCP data from "server"
00000000  48  54  54  50  2F  31  2E  31  20  32  30  30  20  4F  4B  0D
00000010  0A  64  61  74  65  3A  20  46  72  69  2C  20  31  30  20  4D
00000020  61  72  20  32  30  32  33  20  31  33  3A  34  32  3A  35  32
00000030  20  47  4D  54  0D  0A  73  65  72  76  65  72  3A  20  75  76
00000040  69  63  6F  72  6E  0D  0A  63  6F  6E  74  65  6E  74  2D  6C
00000050  65  6E  67  74  68  3A  20  34  0D  0A  63  6F  6E  74  65  6E
00000060  74  2D  74  79  70  65  3A  20  61  70  70  6C  69  63  61  74
00000070  69  6F  6E  2F  6A  73  6F  6E  0D  0A  0D  0A  31  34  30  37

# answer of the server :

# date: Fri, 10 Mar 2023 13:42:52 GMT
# server: uvicorn
# content-length: 4
# content-type: application/json

#1407
```


## UDP Test

Server
```bash
nc -ul 1700
```

RIOT
```
udp send 192.168.2.1:1700 48454c4c4f20574f524c440d0a 10 10000
```

`48454c4c4f20574f524c440d0a` is `HELLO WORLD`


## HTTP Test
TODO

## MQTT Test
TODO



## Tested boards

* [x] nucleo-f429zi
* [x] stm32f746g-disco


## Others
* http://inet.haw-hamburg.de/thesis/completed/simon-brummer

