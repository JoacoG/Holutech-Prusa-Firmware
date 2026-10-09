#pragma once

namespace PowerPanic {
enum PowerPanicFlag : uint8_t {
    NO_PENDING_RECOVERY = 0,
    PENDING_RECOVERY = 1, // First power panic, print state is saved in EEPROM
    PENDING_RECOVERY_RETRY = 2, // Power outage occured during recovery, print is still saved in EEPROM
};

// Types of printjobs possible when power panic is triggered
enum PrintType : uint8_t {
    PRINT_TYPE_SD = 0,
    PRINT_TYPE_HOST = 1,
    PRINT_TYPE_NONE = 2,
};

// How a pending print is restored after a power panic.
enum RecoveryMode : uint8_t {
    RECOVERY_AUTO = 0, // Short outage, bed still hot: recover without user interaction.
    RECOVERY_MANUAL_PURGE = 1, // Long outage, cold nozzle: user triggered, purge to re-prime the nozzle.
    RECOVERY_MANUAL_CLEAN = 2, // Short outage, user chose manual to clean the nozzle: no purge.
};
} // namespace PowerPanic

// Seconds to hold an automatic recovery so a present user can switch it to manual
// recovery and clean the oozed filament off the nozzle before it lands on the print.
// If nobody presses the knob within this window, the print still recovers automatically,
// so unattended prints are never jeopardized. 0 disables the window.
#ifndef UVLO_AUTO_RECOVERY_OVERRIDE_WINDOW
#define UVLO_AUTO_RECOVERY_OVERRIDE_WINDOW 120
#endif

void uvlo_();
void recover_print(PowerPanic::RecoveryMode mode);
void setup_uvlo_interrupt();
void check_uvlo_adc();

extern bool uvlo_wait_for_knob;
extern uint8_t uvlo_trigger_source;
