#!/bin/bash
workspace_folder="$1"
game_directory="$workspace_folder/.vscode/game"
compile_settings_file="$workspace_folder/.vscode/compile_settings.cfg"

echo "Source Code directory: $workspace_folder"

if [ ! -d "$game_directory" ]; then
    read -rp 'Game directory: ' game_target
    if [ ! -d "$game_target" ]; then
        echo "Game directory '$game_target' does not exist."
        exit 1
    fi
    if ! ln -sfnT "$game_target" "$game_directory"; then
        echo "Path '$game_directory' exists but is not a game directory link."
        exit 1
    fi
fi

echo "Game directory: $game_directory"

if [ ! -f "$compile_settings_file" ]; then
    printf 'DEBUG=1\nMAKE_JOBS=0\nHEAVYLOG=0\n' > "$compile_settings_file"
fi

compile_setting=$(tr '\n' ' ' < "$compile_settings_file")
make_target=all
make_jobs=$(nproc)
if [[ $compile_setting =~ (^|[[:space:]])HEAVYLOG=1($|[[:space:]]) ]]; then
    make_target=heavylog
fi
if [[ $compile_setting =~ (^|[[:space:]])MAKE_JOBS=([0-9]+)($|[[:space:]]) ]] && (( BASH_REMATCH[2] > 0 )); then
    make_jobs=${BASH_REMATCH[2]}
fi
if ! make "$make_target" -j"$make_jobs" $compile_setting; then
    echo 'Compilation failed!'
    exit 1
fi

echo 'Compilation successful!'
cp "$workspace_folder"/bin/* "$game_directory"
