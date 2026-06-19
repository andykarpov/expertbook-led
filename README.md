# ASUS ExpertBook B9450 LED Bar Control (`expertbook-led`)

A lightweight, native C++ utility to control the proprietary front panel LED strip (historically known as the *Alexa Light Bar*) on the **ASUS ExpertBook B9450** laptop running Linux. 

This project interacts directly with the embedded **ITE IT8232FN** micro-controller using the native `HID-over-I2C` kernel subsystem (`/dev/hidraw`), providing a fast, zero-dependency solution without bloated background daemons.

## Features
- Trigger 20 built-in hardware animation presets.
- Control global maximum brightness via a 32-step hardware PWM scale.
- Adjust animation loop counts or unlock endless loops.
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

## Desktop Notifications Integration (D-Bus Bridge)

You can turn the Alexa Light Bar into a physical notification center for your Linux desktop environment (GNOME, KDE, XFCE, etc.). By intercepting the `org.freedesktop.Notifications` interface via D-Bus, the LED bar can flash whenever a system app, messenger (Telegram, Slack), or browser triggers a popup notification.

### 1. Notification Listener Script
Create a script named `expertbook-notifier.sh`:

```bash
#!/bin/bash
LED_UTIL="/usr/local/bin/expertbook-led"

stdbuf -oL dbus-monitor "interface='org.freedesktop.Notifications',member='Notify'" | while read -r line; do
    if echo "\$line" | grep -q "member=Notify"; then
        URGENCY=1
        PARAM_BLOCK=\$(timeout 0.05 cat)
        
        if echo "\$PARAM_BLOCK" | grep -A1 '"urgency"' | grep -q "byte 2"; then
            URGENCY=2 # Critical
        elif echo "\$PARAM_BLOCK" | grep -A1 '"urgency"' | grep -q "byte 0"; then
            URGENCY=0 # Low
        fi

        case "\$URGENCY" in
            2) \$LED_UTIL --effect 8 ;; # Critical
            0) \$LED_UTIL --effect 1  ;; # Low
            *) \$LED_UTIL --effect 5  ;; # Normal
        esac
    fi
done
```
Make it executable: `chmod +x expertbook-notifier.sh`

### 2. Auto-start on Login (Systemd User Service)
To run this listener automatically in the background when your graphical desktop session loads, configure it as a systemd user service.

Create the service file inside your user directory at `~/.config/systemd/user/expertbook-notifier.service`:

```ini
[Unit]
Description=ASUS ExpertBook LED Notification Bridge
After=graphical-session.target

[Service]
Type=simple
ExecStart=/home/YOUR_USERNAME/.local/bin/expertbook-notifier.sh
Restart=always
RestartSec=3

[Install]
WantedBy=default.target
```
*(Make sure to change `YOUR_USERNAME` and the file path to point to your exact script location).*

Enable and start the service (no root/sudo privileges required):
```bash
systemctl --user enable --now expertbook-notifier.service
```
Now, your front panel hardware LED bar will subtly pulse whenever your system delivers any desktop event!

---

## Technical Protocol Mapping

All operations construct targeted 33-byte `HID Feature Reports` aiming at **VID: `0x0B05` / PID: `0x0124`** using Report ID `32` (`0x20`).

| Byte 0 (Report ID) | Byte 1 (Sub-Command) | Byte 2 (Parameter) | Remaining Bytes (3-32) | Functional Description |
|---|---|---|---|---|
| `32` | `1` | `1-20` | `0...` | Fires an internal hardware animation routine. |
| `32` | `2` | `2` | `0...` | Modifies playback cycle loop state to infinity. |
| `32` | `3` | `1-255` | `0...` | Saves discrete animation loop counts to controller registers. |
| `32` | `4` | `1-32` | `0...` | Compresses hardware LED brightness matrix (5-bit PWM). |


## License
MIT License. Feel free to copy, modify, and integrate.

