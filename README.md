# ASUS ExpertBook B9450 LED Bar Control (`expertbook-led`)

A lightweight, native C++ utility to control the proprietary front panel LED strip (historically known as the *Alexa Light Bar*) on the **ASUS ExpertBook B9450** laptop under Linux. 

This project interacts directly with the embedded **ITE IT8232FN** micro-controller via native `HID-over-I2C` kernel interface (`/dev/hidraw`), avoiding bloated scripts, daemons, or Python wrappers.

## Features
- Trigger 20 built-in hardware animation effects.
- Set custom static RGB colors across the entire bar.
- Control global maximum brightness.
- Adjust animation loop counts or enable infinite loops.
- Lock/unlock between Direct Matrix Drive and Command modes seamlessly.
- Includes automated hardware permission setup (run without `sudo`).

## Prerequisites
You need a modern Linux kernel (5.15+) and the `hidapi` development library.

On Debian/Ubuntu based distributions:
```bash
sudo apt install libhidapi-dev g++ make
```

## Installation

1. Clone the repository and compile the binary:
   ```bash
   make
   ```
2. System-wide installation (places the executable into `/usr/local/bin` and deploys necessary `udev` rules for sudo-free access):
   ```bash
   sudo make install
   ```

## Usage Examples

Once installed, you can trigger the LED bar from terminal, cronjobs, or systemd hooks:

* **Play an animation effect (e.g., effect #5):**
  ```bash
  expertbook-led --effect 5
  ```
* **Set solid customized color (e.g., Cyan for battery full):**
  ```bash
  expertbook-led --static 0 255 255
  ```
* **Dim the maximum brightness to 20% (useful at night):**
  ```bash
  expertbook-led --brightness 20
  ```
* **Configure animations to repeat 5 times globally, then trigger effect #12:**
  ```bash
  expertbook-led --repeat 5
  expertbook-led --effect 12
  ```
* **Enable endless animation loops:**
  ```bash
  expertbook-led --infinite
  ```
* **Turn off the LED bar completely:**
  ```bash
  expertbook-led --off
  ```

---

## Technical Protocol Documentation

The tool crafts specific 33-byte `HID Feature Reports` targeting **VID: `0x0B05` / PID: `0x0124`** on Report ID `32` (`0x20`).

### Protocol Mapping (Feature Report 32)

| Byte 0 (Report ID) | Byte 1 (Sub-Command) | Byte 2 (Parameter) | Remaining Bytes (3-32) | Description |
|---|---|---|---|---|
| `32` | `1` | `1-20` | `0...` | Plays a factory hardcoded animation pre-set. |
| `32` | `2` | `2` | `0...` | Sets animation playback loop to infinite. |
| `32` | `3` | `1-255` | `0...` | Saves animation loop limit state in chip memory. |
| `32` | `4` | `0-255` | `0...` | Sets global LED brightness attenuation. |
| `32` | `6` | `2` | `0...` | **Command Mode Reset:** Releases the state machine back to effect listening state. |
| `32` | `6` | `R` | `G, B, R, G, B...` | **Direct Drive Mode:** Feeds raw streaming matrix arrays to individual LED diodes. |

*Note: Entering Direct Drive Mode locks the chip's internal state machine. Calling `--effect` safely triggers a `32, 6, 2` packet first to properly restore execution.*

## License
MIT License. Feel free to use, modify, and distribute.

