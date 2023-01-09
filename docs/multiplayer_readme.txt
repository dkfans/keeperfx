Multiplayer in KeeperFX
------------------------------

KeeperFX supports multiplayer over TCP/IP or ENET/UDP. Serial, Modem and IPX are
not supported. 1Player allows you to practice multiplayer maps alone.

At the moment of writing, KeeperFX, like Dungeon Keeper, requires a LAN-grade
connection to work properly. Low latency is what's important, not high speeds.
Latency is always high when playing over very high distances, so look for local
friends to play with.

* High-latency connections will make both games run very slow.
* Limited to two players.
* Desyncs and crashes are very possible, report issues and they may be fixed.

Multiplayer over ENET/UDP
------------------------------

To host a ENET/UDP game:
Make sure the port 5556 is open for traffic and is forwarded to port 5556 on
your computer. When you have started the game, click Multiplayer -> ENET/UDP ->
Create Game.

To join a ENET/UDP game:
Specify a command line option -sessions [ip_address]:5556 when starting game.
For instance, if the host's IP address is 55.83.54.187, the appropriate command
line option is -sessions 55.83.54.187:5556
The launcher can be used to set this.

Several sessions can be added to command line by prepending a semicolon before
each extra address, e.g. -sessions 55.83.54.187:5555;214.43.45.67:5556

When you have started the game, click Multiplayer -> ENET/UDP -> [select IP
address in list] -> Join Game.

Multiplayer over TCP/IP
------------------------------

Hosting or joining a game over TCP/IP works the same as over ENET/UDP, with a
key difference. Open port 5555 instead and select TCP/IP from the multiplayer
menu.

TCP/IP is less suitable for multiplayer as a protocol, and there's no known 
reason to use this over ENET/UDP as you will have significantly more lag.
However, this protocal has been around for longer and is no longer being 
developed or tested on, there's a chance that new bugs on ENET/UDP have
not ended up in this protocol.