#!/usr/bin/env bash
#
# deploy-to-deck.sh - push a KeeperFX build to a Steam Deck (or any remote Linux
# box) over SSH, and optionally run it or start a gdbserver for remote debugging.
#
# The dev-kit loop is:  build (scripts/build-deck-bundle.sh) -> deploy here ->
# run / gdbserver -> attach from your IDE. See docs/steam-deck.md.
#
# Usage:
#   scripts/deploy-to-deck.sh [options]
#
# Options:
#   -H, --host HOST     SSH host of the Deck        (default: $DECK_HOST or "steamdeck")
#   -B, --bundle DIR    self-contained bundle dir   (default: out/keeperfx-deck)
#   -b, --binary PATH   single binary instead of a bundle dir
#   -a, --assets DIR    game data dir to sync alongside the binary
#   -d, --dest DIR      remote install dir          (default: ~/keeperfx)
#   -r, --run           run it on the Deck after deploying
#   -g, --gdb           run under gdbserver after deploying (attach from your IDE)
#   -p, --port PORT     gdbserver port              (default: 2345)
#       --args "..."    args passed to keeperfx     (default: level 1, keeporig)
#   -h, --help          show this help
#
# --run / --gdb auto-detect the Deck's graphical session (DISPLAY / WAYLAND_DISPLAY
# / XAUTHORITY) from the running compositor, so the window appears on the Deck.
#
set -euo pipefail

HOST="${DECK_HOST:-steamdeck}"
BUNDLE="out/keeperfx-deck"
BINARY=""
ASSETS=""
DEST="${DECK_DEST:-~/keeperfx}"
RUN=0; GDB=0; PORT=2345
GAME_ARGS="${DECK_ARGS:--level 00001 -campaign keeporig -nointro}"

die() { echo "deploy-to-deck: $*" >&2; exit 1; }

while [ $# -gt 0 ]; do
    case "$1" in
        -H|--host)   HOST="$2"; shift 2 ;;
        -B|--bundle) BUNDLE="$2"; BINARY=""; shift 2 ;;
        -b|--binary) BINARY="$2"; BUNDLE=""; shift 2 ;;
        -a|--assets) ASSETS="$2"; shift 2 ;;
        -d|--dest)   DEST="$2"; shift 2 ;;
        -r|--run)    RUN=1; shift ;;
        -g|--gdb)    GDB=1; shift ;;
        -p|--port)   PORT="$2"; shift 2 ;;
        --args)      GAME_ARGS="$2"; shift 2 ;;
        -h|--help)   sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) die "unknown option: $1 (try --help)" ;;
    esac
done

# --- transfer helper: rsync when both ends have it, else tar-over-ssh ---------
have_rsync=0
if command -v rsync >/dev/null 2>&1 && ssh "$HOST" 'command -v rsync' >/dev/null 2>&1; then
    have_rsync=1
fi

push_dir() {   # push_dir <local_dir> <remote_dir>
    local src="$1" dst="$2"
    ssh "$HOST" "mkdir -p \"$dst\""
    if [ "$have_rsync" -eq 1 ]; then
        rsync -a --info=progress2 "$src"/ "$HOST:$dst"/
    else
        tar cf - -C "$src" . | ssh "$HOST" "tar xf - -C \"$dst\""
    fi
}

echo ">> target: $HOST:$DEST   (transfer: $([ $have_rsync -eq 1 ] && echo rsync || echo tar-over-ssh))"

if [ -n "$BUNDLE" ]; then
    [ -d "$BUNDLE" ] || die "bundle dir not found: $BUNDLE (build it: scripts/build-deck-bundle.sh)"
    [ -x "$BUNDLE/keeperfx" ] || die "no keeperfx binary in $BUNDLE"
    echo ">> deploying bundle: $BUNDLE -> $DEST/ (binary + lib/)"
    push_dir "$BUNDLE" "$DEST"
else
    [ -f "$BINARY" ] || die "binary not found: $BINARY"
    echo ">> deploying binary: $BINARY -> $DEST/keeperfx"
    ssh "$HOST" "mkdir -p \"$DEST\""
    if [ "$have_rsync" -eq 1 ]; then rsync -a "$BINARY" "$HOST:$DEST/keeperfx";
    else tar cf - -C "$(dirname "$BINARY")" "$(basename "$BINARY")" | ssh "$HOST" "tar xf - -C \"$DEST\" && mv \"$DEST/$(basename "$BINARY")\" \"$DEST/keeperfx\""; fi
fi
ssh "$HOST" "chmod +x \"$DEST/keeperfx\""

if [ -n "$ASSETS" ]; then
    [ -d "$ASSETS" ] || die "assets dir not found: $ASSETS"
    echo ">> syncing game data: $ASSETS -> $DEST/"
    push_dir "$ASSETS" "$DEST"
fi

# --- remote launch prelude: adopt the Deck's graphical session env -----------
# shellcheck disable=SC2016
ENV_PRELUDE='for _p in plasmashell kwin_wayland gnome-shell weston sway gamescope; do
    _pid=$(pgrep -x "$_p" 2>/dev/null | head -1); [ -n "$_pid" ] && break; done
if [ -n "${_pid:-}" ] && [ -r /proc/$_pid/environ ]; then
    while IFS= read -r -d "" _kv; do case "$_kv" in
        DISPLAY=*|WAYLAND_DISPLAY=*|XAUTHORITY=*|XDG_RUNTIME_DIR=*) export "$_kv";; esac
    done < /proc/$_pid/environ
fi
: "${XDG_RUNTIME_DIR:=/run/user/$(id -u)}"; export XDG_RUNTIME_DIR'

if [ "$GDB" -eq 1 ]; then
    echo ">> starting gdbserver on $HOST:$PORT  (attach your IDE to $HOST:$PORT)"
    ssh -t "$HOST" "cd \"$DEST\" && $ENV_PRELUDE
        if command -v gdbserver >/dev/null; then gdbserver :$PORT ./keeperfx $GAME_ARGS;
        elif distrobox list 2>/dev/null | grep -q kfx; then distrobox-enter kfx -- gdbserver :$PORT ./keeperfx $GAME_ARGS;
        else echo 'no gdbserver on the Deck (install via distrobox)'; fi"
elif [ "$RUN" -eq 1 ]; then
    echo ">> launching on $HOST (window should appear on the Deck)"
    ssh -t "$HOST" "cd \"$DEST\" && $ENV_PRELUDE
        ./keeperfx $GAME_ARGS"
else
    echo ">> deployed. Play:  scripts/deploy-to-deck.sh --run     Debug:  scripts/deploy-to-deck.sh --gdb"
fi
