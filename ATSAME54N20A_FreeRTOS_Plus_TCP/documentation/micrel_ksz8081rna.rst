================================================
Micrel KSZ8081RNA IEEE 802.3 Ethernet PHY Driver
================================================

This software component supply a Generic IEEE802.3 Ethernet PHY driver.
The PHY chip should compliant IEEE 802.3 Ethernet Standard that
support MDC/MDIO management interface for PHY register configuration.

The Micrel KSZ8081RNA is a single-chip 10Bast-T/100Bast-TX Ethernet
Physical layer transceiver for transmission and reception of data over
standard CAT-5 unshielded twisted pair (UTP) cable.

The KSZ8081RNA features a Reduced Media Independent Interface (RMII) for
direct connection with RMII-compliant Ethernet MAC processors and switches.

Features
--------

* Initialization the Ethernet PHY driver with Ethernet MAC communication
* Setting PHY address
* Reading/Writing register from PHY device
* Setting/Clearing register bit from PHY device
* Enabling/Disabling Power Down
* Restart Auto Negotiation
* Enabling/Disabling Loop Back
* Getting Link Status
* Reset PHY device

Dependencies
------------

* An instance of the Ethernet MAC driver is used by this driver.

Limitations
-----------

N/A
