#!/usr/bin/env bash
set -euo pipefail

APP_NAME="${APP_NAME:-KeeperFX.app}"
APP_BASENAME="${APP_NAME%.app}"
OUTPUT_DIR="${OUTPUT_DIR:-dist}"
BIN_PATH="${BIN_PATH:-bin/keeperfx}"

APP_DIR="${OUTPUT_DIR}/${APP_NAME}"
MACOS_DIR="${APP_DIR}/Contents/MacOS"
RES_DIR="${APP_DIR}/Contents/Resources"
FW_DIR="${APP_DIR}/Contents/Frameworks"

rm -rf "${APP_DIR}"
mkdir -p "${MACOS_DIR}" "${RES_DIR}" "${FW_DIR}"

cp "${BIN_PATH}" "${MACOS_DIR}/keeperfx"
chmod +x "${MACOS_DIR}/keeperfx"

cat > "${APP_DIR}/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key>
  <string>${APP_BASENAME}</string>
  <key>CFBundleIdentifier</key>
  <string>org.keeperfx.${APP_BASENAME}</string>
  <key>CFBundleVersion</key>
  <string>1.0</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0</string>
  <key>CFBundleExecutable</key>
  <string>keeperfx</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>LSMinimumSystemVersion</key>
  <string>12.0</string>
</dict>
</plist>
EOF

install_name_tool -add_rpath "@executable_path/../Frameworks" "${MACOS_DIR}/keeperfx" 2>/dev/null || true

declare -a queue
queue=("${MACOS_DIR}/keeperfx")

seen_file=$(mktemp)
trap 'rm -f "${seen_file}"' EXIT

while [ ${#queue[@]} -gt 0 ]; do
  file="${queue[0]}"
  queue=("${queue[@]:1}")

  if grep -Fxq "${file}" "${seen_file}"; then
    continue
  fi
  echo "${file}" >> "${seen_file}"

  deps=$(otool -L "${file}" | tail -n +2 | awk '{print $1}')
  for dep in ${deps}; do
    case "${dep}" in
      /opt/homebrew/*|/usr/local/*)
        libname=$(basename "${dep}")
        target="${FW_DIR}/${libname}"
        if [ ! -f "${target}" ]; then
          cp "${dep}" "${target}"
          chmod 644 "${target}" || true
          queue+=("${target}")
        fi
        if [ "${file}" = "${MACOS_DIR}/keeperfx" ]; then
          install_name_tool -change "${dep}" "@rpath/${libname}" "${file}" || true
        else
          install_name_tool -change "${dep}" "@loader_path/../Frameworks/${libname}" "${file}" || true
        fi
        ;;
    esac
  done

done

for dylib in "${FW_DIR}"/*.dylib; do
  if [ -f "${dylib}" ]; then
    libname=$(basename "${dylib}")
    install_name_tool -id "@rpath/${libname}" "${dylib}" || true
    install_name_tool -add_rpath "@loader_path/../Frameworks" "${dylib}" 2>/dev/null || true
  fi
done

echo "App bundle created at ${APP_DIR}"
