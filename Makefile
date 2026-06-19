CXX = g++
CXXFLAGS = -O2 -Wall
LIBS = -lhidapi-hidraw
TARGET = expertbook-led

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET) $(LIBS)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	@echo 'KERNEL=="hidraw*", ATTRS{idVendor}=="0b05", ATTRS{idProduct}=="0124", MODE="0666"' > /etc/udev/rules.d/99-asus-alexa-bar.rules
	@udevadm control --reload-rules && udevadm trigger
	@echo "Installation complete. Tool placed in /usr/local/bin/. Udev rules applied (Sudo-free execution allowed)."

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -f /etc/udev/rules.d/99-asus-alexa-bar.rules
	@udevadm control --reload-rules && udevadm trigger
	@echo "Uninstalled successfully."

clean:
	rm -f $(TARGET)
