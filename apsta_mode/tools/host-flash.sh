#!/usr/bin/env bash
# ============================================================================
# host-flash.sh – Flash ESP32-S3 depuis macOS (hôte)
# ============================================================================
# Utilisation :
#
#   1. Build dans VS Code (container) : Ctrl+Shift+B
#   2. Depuis le terminal macOS :
#        ./tools/host-flash.sh
#
# Le script utilise esptool directement sur le port série du Mac.
# Le binaire build/zephyr/zephyr.bin est accessible car le workspace
# est monté depuis l'hôte dans le container.
# ============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

# ── Détection du port série ESP32-S3 ──
PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)

if [ -z "$PORT" ]; then
  echo "❌ Aucun port ESP32 trouvé (cherché /dev/cu.usbmodem*)"
  echo "   Branche la carte et vérifie avec : ls /dev/cu.usbmodem*"
  exit 1
fi

echo "🔌 Port détecté : $PORT"

# ── Vérifier/installer esptool ──
if ! python3 -c "import esptool" 2>/dev/null; then
  echo "📦 Installation de esptool…"
  pip3 install esptool
fi

# ── Binaire à flasher ──
BIN="$SCRIPT_DIR/build/zephyr/zephyr.bin"

if [ ! -f "$BIN" ]; then
  echo "❌ Fichier binaire introuvable : $BIN"
  echo "   Fais d'abord 🔨 Build dans VS Code (Ctrl+Shift+B)"
  exit 1
fi

echo "⚡ Flash de $BIN …"
echo "   Assure-toi de maintenir BOOT, taper RESET, relâcher BOOT."
esptool --chip esp32s3 --port "$PORT" --baud 921600 \
  --before default-reset --after hard-reset \
  write-flash 0x0 "$BIN"
