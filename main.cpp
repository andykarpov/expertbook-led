#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <hidapi/hidapi.h>

const unsigned short ASUS_VID = 0x0B05;
const unsigned short ASUS_PID = 0x0124;
const size_t REPORT_SIZE = 33;

void print_usage() {
    std::cout << "Usage: expertbook-led [options]\n"
              << "Options:\n"
              << "  --effect [1-20]        Play built-in hardware animation effect\n"
              << "  --brightness [1-32]    Set global PWM brightness level (1-32 steps)\n"
              << "  --repeat [1-255]       Set global animation repeat loop count\n"
              << "  --infinite             Enable endless animation playback loop\n"
              << "  --freeze               Freeze/Pause the animation layout at current frame\n"
              << "  --unfreeze             Unfreeze the animation and restore command state\n";
}

bool send_feature_report(const std::vector<unsigned char>& data) {
    if (hid_init() < 0) return false;

    hid_device* handle = hid_open(ASUS_VID, ASUS_PID, NULL);
    if (!handle) {
        std::cerr << "Error: ASUS Alexa Bar not found or permission denied (try running with sudo).\n";
        hid_exit();
        return false;
    }

    unsigned char buffer[REPORT_SIZE];
    std::memset(buffer, 0, REPORT_SIZE);
    
    for (size_t i = 0; i < data.size() && i < REPORT_SIZE; ++i) {
        buffer[i] = data[i];
    }

    int res = hid_send_feature_report(handle, buffer, REPORT_SIZE);
    
    hid_close(handle);
    hid_exit();
    
    return (res >= 0);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string arg = argv[1];
    std::vector<unsigned char> report_data = {32}; // Report ID: 0x20

    if (arg == "--effect" && argc == 3) {
        int effect = std::stoi(argv[2]);
        if (effect < 1 || effect > 20) { std::cerr << "Invalid effect number (1-20)\n"; return 1; }
        
        // Always unfreeze state machine before pushing a new effect
        send_feature_report({32, 6, 2});
        
        report_data.push_back(1); 
        report_data.push_back(effect);
    } 
    else if (arg == "--infinite") {
        report_data.push_back(2);
        report_data.push_back(2);
    } 
    else if (arg == "--repeat" && argc == 3) {
        int repeat = std::stoi(argv[2]);
        if (repeat < 1 || repeat > 255) { std::cerr << "Invalid repeat count (1-255)\n"; return 1; }
        report_data.push_back(3);
        report_data.push_back(repeat);
    } 
    else if (arg == "--brightness" && argc == 3) {
        int b = std::stoi(argv[2]);
        if (b < 1 || b > 32) { std::cerr << "Brightness must be 1-32\n"; return 1; }
        report_data.push_back(4);
        report_data.push_back(b);
    } 
    else if (arg == "--freeze") {
        report_data.push_back(6);
        report_data.push_back(0); // Trigger latch freeze frame
    }
    else if (arg == "--unfreeze") {
        report_data.push_back(6);
        report_data.push_back(2); // Recover execution
    } 
    else {
        print_usage();
        return 1;
    }

    if (send_feature_report(report_data)) {
        std::cout << "Command accepted by ITE controller.\n";
    } else {
        std::cerr << "Hardware transmission failed.\n";
        return 1;
    }

    return 0;
}

