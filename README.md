# ASUS ExpertBook B9450 LED Bar Control (`expertbook-led`)

A lightweight, native C++ utility to control the proprietary front panel LED strip (historically known as the *Alexa Light Bar*) on the **ASUS ExpertBook B9450** laptop running Linux. 

This project interacts directly with the embedded **ITE IT8232FN** micro-controller using the native `HID-over-I2C` kernel subsystem (`/dev/hidraw`), providing a fast, zero-dependency solution without bloated background daemons.

## Features
- Trigger 20 built-in hardware animation presets.
- Control global maximum brightness via a 32-step hardware PWM scale.
- Adjust animation loop counts or unlock endless loops.
- Freeze/Pause current animation frame and safely unfreeze the chip's internal state machine.
- Integrated `udev` setup allowing secure execution from standard non-root user space.

## Prerequisites
Ensure you have a modern Linux kernel and the `hidapi` compilation libraries.

On Debian/Ubuntu based systems:
```bash
sudo apt install libhidapi-dev g++ make
```

## Installation

1. Build the binary from source:
   ```bash
   make
   ```
2. System-wide installation (installs the binary to `/usr/local/bin` and automatically configures safe `udev` hardware permissions):
   ```bash
   sudo make install
   ```

## Usage Examples

Once installed, the utility can be seamlessly integrated into user shell environments, event hooks, or cron timers:

* **Play a specific built-in hardware animation (e.g., effect #5):**
  ```bash
  expertbook-led --effect 5
  ```
* **Dim the bar's maximum intensity down to night-friendly level (values 1 to 32):**
  ```bash
  expertbook-led --brightness 8
  ```
* **Set animations to loop exactly 3 times globally, then play effect #12:**
  ```bash
  expertbook-led --repeat 3
  expertbook-led --effect 12
  ```
* **Allow hardware effects to run endlessly:**
  ```bash
  expertbook-led --infinite
  ```
* **Freeze the current layout configuration / Pause the animation frame:**
  ```bash
  expertbook-led --freeze
  ```
* **Unfreeze and restore the micro-controller back to command listening state:**
  ```bash
  expertbook-led --unfreeze
  ```

---

## Technical Protocol Mapping

All operations construct targeted 33-byte `HID Feature Reports` aiming at **VID: `0x0B05` / PID: `0x0124`** using Report ID `32` (`0x20`).

| Byte 0 (Report ID) | Byte 1 (Sub-Command) | Byte 2 (Parameter) | Remaining Bytes (3-32) | Functional Description |
|---|---|---|---|---|
| `32` | `1` | `1-20` | `0...` | Fires an internal hardware animation routine. |
| `32` | `2` | `2` | `0...` | Modifies playback cycle loop state to infinity. |
| `32` | `3` | `1-255` | `0...` | Saves discrete animation loop counts to controller registers. |
| `32` | `4` | `1-32` | `0...` | Compresses hardware LED brightness matrix (5-bit PWM). |
| `32` | `6` | `0` | `0...` | **Freeze State:** Latches and pauses the current lighting frame. |
| `32` | `6` | `2` | `0...` | **State Reset:** Recovers the state machine back to standard operational readiness. |

*Implementation Note: Executing an animation through `--effect` automatically sends an unfreeze (`32, 6, 2`) frame first to prevent hardware lockups from previous pause states.*

## License
MIT License. Feel free to copy, modify, and integrate.

