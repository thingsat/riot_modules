# GNSS PPS test for `nucleo-l432kc-inisat` board

The GNSS PPS pin of the miniPCIe SX130x module is routed on `PB7` pin of the `nucleo-l432kc` board.

The GNSS PPS pin of the Mikrobus GNSS XA1110 module is routed on `PB7` pin of the `nucleo-l432kc` board.

> NB: the `JP5` jumper must set on `PCI` for selecting miniPCIe and on `MK` selecting Mikrobus.

```bash
cd ~/github/RIOT-OS/RIOT/tests/periph/gpio
gmake BOARD=nucleo-l432kc flash
```

```
> init_int
usage: init_int <port> <pin> <flank> [pull_config]
	flank:
	0: falling
	1: rising
	2: both
	pull_config:
	0: no pull resistor (default)
	1: pull up
	2: pull down
> init_int 0 7 1
GPIO_PIN(0, 7) successfully initialized as ext int
> INT: external interrupt from pin 7
INT: external interrupt from pin 7
```
