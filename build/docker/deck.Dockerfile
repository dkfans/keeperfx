# ---------------------------------------------------------------------------
# Pinned build image for the Steam Deck bundle (see docs/steam-deck.md).
# ---------------------------------------------------------------------------
# Debian trixie == glibc 2.41, matching current SteamOS.
#
# SDL3 is built from source (trixie has no sdl3-mixer dev package).
# Include the other shit we need for building the game, so we can build a bundle on the Deck itself.
## 1. Pair the Deck (once)

#On the Deck (Desktop Mode, Konsole):

#1. Set a `deck` password if needed: `passwd`.
#2. Enable SSH: `sudo systemctl enable --now sshd`.
#3. From your PC, install a key so logins are password-less:
#   `ssh-copy-id deck@<deck-ip>`.
#4. Add an SSH alias so everything can say `steamdeck`. In `~/.ssh/config`
#   (Windows: `%USERPROFILE%\.ssh\config`):
# ---------------------------------------------------------------------------
FROM debian:trixie
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq && apt-get install -y -qq \
    build-essential cmake ninja-build pkg-config git curl ca-certificates \
    patchelf rsync \
    libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
    libopenal-dev libluajit-5.1-dev libspng-dev libminizip-dev zlib1g-dev \
    libminiupnpc-dev libnatpmp-dev libssl-dev libzstd-dev \
    wayland-protocols libwayland-dev libdecor-0-dev \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev \
    libxss-dev libxtst-dev libxkbcommon-dev libgl1-mesa-dev libegl1-mesa-dev \
    libdrm-dev libgbm-dev libasound2-dev libpulse-dev libdbus-1-dev libudev-dev \
    libpng-dev libjpeg-dev libogg-dev libvorbis-dev libflac-dev libmpg123-dev libopusfile-dev \
  && rm -rf /var/lib/apt/lists/*
