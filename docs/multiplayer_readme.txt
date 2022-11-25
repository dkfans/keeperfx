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

Multiplayer over TCP/IP
------------------------------

To host a TCP/IP game:
Make sure the port 5555 is open for traffic and is forwarded to port 5555 on
your computer. When you have started the game, click Multiplayer -> TCP/IP ->
Create Game.

To join a TCP/IP game:
Specify a command line option -sessions [ip_address]:5555 when starting game.
For instance, if the host's IP address is 55.83.54.187, the appropriate command
line option is -sessions 55.83.54.187:5555
The launcher can be used to set this.

Several sessions can be added to command line by prepending a semicolon before
each extra address, e.g. -sessions 55.83.54.187:5555;214.43.45.67:5555

When you have started the game, click Multiplayer -> TCP/IP -> [select IP
address in list] -> Join Game.

Multiplayer over ENET/UDP
------------------------------

Hosting or joining a game over ENET/UDP works the same as over TCP/IP, with a
key difference. Open port 5556 instead and select ENET/UDP from the multiplayer
menu.

+ A faster protocol so could accept a bit more latency for the same lag.
- Brand new, so totally untested. Go for it and tell us where we're wrong.
