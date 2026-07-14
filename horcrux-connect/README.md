# Horcrux-connect

## WARNING This is a work in progress. The hardware and software are not yet ready for production use.

## Quick Start - flash ESP32

### Via ESPWEBTOOL


**Requirements** : 

Board's compatibility :
* ESP32-S3-DevKitC-1

Latest firmware release : 
* [last release](https://github.com/ficaud/horcrux_box/releases)

**How to flash the firmware** : 

1. Go to [ESPWEBTOOL](https://esptool.spacehuhn.com/) and connect to your board by selecting the correct serial port.

2. Reset the board in bootloader mode by pressing the BOOT button while pressing the EN/RESET button, then release the EN button and finally release the BOOT button.

3. Remove all sectors keeping only one sector for the firmware. The sector size is 4MB, so you can keep the first sector (0x00000) and remove the rest.

4. Select the firmware binary file and flash it by clicking on the "PROGRAM" button.

### Via ESPTOOL (Python)

**Requirements** :

- Python 3.6+
- [esptool](https://github.com/espressif/esptool) Python package
- USB cable (data-capable) to connect the board to your computer

Board's compatibility :
* ESP32-S3-DevKitC-1

**1. Install esptool**

```bash
pip install esptool
```

Verify the installation:

```bash
python3 -c "import esptool; print(esptool.__version__)"
```

**2. Connect the board and identify the serial port**

- **macOS**: `/dev/cu.usbmodem*` or `/dev/tty.usbmodem*`
- **Linux**: `/dev/ttyACM0` or `/dev/ttyUSB0`
- **Windows**: `COM1`, `COM2`, etc.

List available ports:

```bash
# macOS / Linux
ls /dev/cu.usbmodem* /dev/ttyACM* /dev/ttyUSB* 2>/dev/null

# Windows (PowerShell)
python -m esptool chip_id
```

**3. Put the board in bootloader (download) mode**

1. Press and hold the **BOOT** button.
2. While holding **BOOT**, press and release the **EN/RESET** button.
3. Release the **BOOT** button.

The board is now ready to receive firmware.

**4. Flash the firmware**

Erase the flash first (optional but recommended for a clean slate):

Note, if you're on macOS, don't forget to activate your virtual environment if you have one.

```bash
esptool.py --chip esp32s3 --port <YOUR_PORT> erase_flash
```

Write the firmware binary to address `0x0`:

```bash
esptool.py --chip esp32s3 --port <YOUR_PORT> --baud 921600 \
  --before default-reset --after hard-reset \
  write_flash 0x0 path/to/firmware.bin
```

Replace `<YOUR_PORT>` with the port detected in step 2, and `path/to/firmware.bin` with the path to your compiled binary (e.g., `build/zephyr/zephyr.bin`).

## Quick Start - contribution

Below are the basic steps to build, flash, and monitor the serial output of the Horcrux-Connect firmware on the ESP32.

### Build and Flash

In the development environment, you can build and program the Horcrux ESP32 using `tasks.json` in VSCode:

```bash
Ctrl + Shift + P → Tasks: Run Task → Pristine Build (esp32s3_devkitc/procpu) or Build (esp32s3_devkitc/procpu)
```

Then, from the host where the ESP32 is connected, flash the firmware as follows:

```bash
./host-flash.sh
```

### Monitor the Serial Output

**Note:** You need `west` installed in your host environment to monitor the serial output. Install it using pip:

```bash
pip install west
```

From the host where the ESP32 is connected, monitor the serial output as follows:

```bash
source ~/zephyrproject/.venv/bin/activate
west espressif monitor
```