ATSAME54N20A_FreeRTOS_Plus_TCP
==============================

This is a simple demo of an in progress FreeRTOS+TCP driver for the ATSAME54N20A (SAM E54), which should also work for any SAM E5X.
The test board consists of a ATSAME54N20A and a KSZ8081RNAIA, both powered by a +3V3 LDO regulator and fed by a 25 MHz clock.
The PCB might be published later, when the rest of the hardware is fully validated.

The project should be imported in Microchip Studio 7.0.

The driver uses Atmel Start ASF4 generated MAC and PHY drivers, along with the code in [NetworkInterface.c](ATSAME54N20A_FreeRTOS_Plus_TCP/FreeRTOS/FreeRTOS-Plus-TCP/portable/NetworkInterface/ATSAME5/NetworkInterface.c).
The demo uses many helper functions located in the [Application](ATSAME54N20A_FreeRTOS_Plus_TCP/Application) folder

Features
--------
- DHCP
- ICMP Echo (Ping) send and receive
- LLMNR
- Hardware CRC offloading (except for ICMP, which is done in software)
- Hardware frame filtering


Log
---

2021-05-18:
- Implement LLMNR and unique chip ID


2021-05-17:
- Added ICMP Echo (Ping) send and receive support with software CRC check (MAC offloading supports TCP, UDP and IP only)


2021-05-16: 
- Migrated to FreeRTOSv202104.00, which includes coreMQTT-Agent
- Added support for PING (sending untested)
- DHCP now working properly
- Added MAC frames filtering
- Moved MAC and PHY initialization to NetworkInterface.c


2021-05-15: 
- Working DHCP, but only on 2nd try.