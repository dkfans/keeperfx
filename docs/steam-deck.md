# KeeperFX on a Steam Deck — remote dev-kit workflow

Two commands once it's set up:

```bash
scripts/build-deck-bundle.sh                              # produces out/keeperfx-deck/
scripts/deploy-to-deck.sh --assets "<your game dir>" --run
```

---

## 1. Pair the Deck (once)

On the Deck (Desktop Mode, Konsole):

1. Set a `deck` password if needed: `passwd`.
2. Enable SSH: `sudo systemctl enable --now sshd`.
3. From your PC, install a key so logins are password-less:
   `ssh-copy-id deck@<deck-ip>`.
4. Add an SSH alias so everything can say `steamdeck`. In `~/.ssh/config`
   (Windows: `%USERPROFILE%\.ssh\config`):

   ```
   Host steamdeck
       HostName 192.168.1.xx     # your Deck's LAN IP
       User deck
   ```

Test: `ssh steamdeck echo ok`.

## 2. Build the bundle

```bash
scripts/build-deck-bundle.sh
```

## 3. Deploy + run
assets is where the full data of keeperfx is on your machine.

```bash
scripts/deploy-to-deck.sh --assets "C:/path/to/keeperfx game dir" --run
```

The first deploy will push everything, the rest just what changes

## 4. Debug — three ways

`scripts/deploy-to-deck.sh --gdb` deploys and starts `gdbserver :2345` on the Deck
(SteamOS ships `gdbserver`; it also falls back to a `kfx` distrobox if present).

### A. VS Code — cross debug (build on PC, gdbserver on Deck)
1. `scripts/deploy-to-deck.sh --gdb`.
2. Run the **"Attach to Steam Deck (gdbserver)"** config (from
   [`.vscode/defaultlaunch`](../.vscode/defaultlaunch)); it attaches to
   `steamdeck:2345`, reading symbols from your local build.
3. Or use the **"Deploy + Debug on Steam Deck"** task to do the deploy + server
   launch first.

Point `program` at your local `out/keeperfx-deck/keeperfx` (same binary, with
symbols); `sourceFileMap` binds breakpoints to your working copy.

### B. VS Code — Remote-SSH (edit/build/run *on* the Deck)
Install **Remote - SSH** (recommended in
[`.vscode/extensions.json`](../.vscode/extensions.json)), connect to `steamdeck`,
and work deck-side with a plain local `gdb`. Tightest inner loop; the Deck compiles.

### C. Visual Studio (full) — remote gdb
1. `Tools → Options → Cross Platform → Connection Manager` → add the Deck.
2. `scripts/deploy-to-deck.sh --gdb`.
3. Pick the **"keeperfx on Steam Deck (gdbserver)"** profile in
   [`launch.vs.json`](../launch.vs.json); adjust `miDebuggerServerAddress` /
   `miDebuggerPath` to your setup.