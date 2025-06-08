# Test JC42 compatible temperature sensor (MCP9808)

[Thingsat @ INISAT](https://github.com/csu-grenoble/flatsat/tree/main/Hardware/Thingsat_INISAT) is equipped of a MCP9808 sensor which is a JC42 compatible temperature sensor.


```bash
cd ~/github/RIOT-OS/RIOT/tests/drivers/jc42
gmake BOARD=nucleo-l432kc -j 8 flash term
```

```bash
tio -b 115200 -m INLCRNL /dev/tty.usbmodem1xxxx
```


```
main(): This is RIOT! (Version: 2025.07-devel-385-g42a38)
JC42 temperature sensor test application

Initializing sensor...[Failed]

```
