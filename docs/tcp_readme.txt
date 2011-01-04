TCP/IP Multiplayer in KeeperFX
------------------------------

TCP/IP support is currently in its infancy and may be 'slightly' buggy.

To host a TCP/IP game:
Make sure the port 5555 is open for traffic and is forwarded to port 5555 on
your computer. When you have started the game, click Multiplayer -> TCP/IP ->
Create Game.

To join a TCP/IP game:
Specify a command line option -sessions [ip_address]:5555 when starting game.
For instance, if the host's IP address is 55.83.54.187, the appropriate command
line option is -sessions 55.83.54.187:5555

Several sessions can be added to command line by prepending a semicolon before
each extra address, e.g. -sessions 55.83.54.187:5555;214.43.45.67:5555

When you have started the game, click Multiplayer -> TCP/IP -> [select IP
address in list] -> Join Game.

Known issues with TCP/IP:
------------------------------
- May go out of sync. (Please remember what you did before this and check log,
    then report it.)
- When it goes out of sync, something bad will generally happen. Re-
    synchronization *might* work.
- Everyone else crashes if host exits.
- Host is auto-computered if anyone leaves. (WTF!?)
- Having a player leave without anyone rejoining creates a 'hole' in player
    list, which will make the game unplayable.
- Session player list does not update when a session is selected.
- Will be laggy like heck on anything but a LAN or near LAN-grade connection
    probably.
- Only old four byte IP-addresses are supported at the moment, as far as I
    know.

  
These issues shall hopefully be resolved one by one in coming versions. I
apologize for them, but I don't have a good test environment nor is
the game code particularly easy to work with. Help me by reporting reasons for
out-of-sync in particular, or other issues not on the above list.

- Petter H, aka P Hansson ;-)