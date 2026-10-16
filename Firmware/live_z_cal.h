//! @file
//! @brief Live-Z first layer calibration print (75mm filled square).

#ifndef FIRMWARE_LIVE_Z_CAL_H_
#define FIRMWARE_LIVE_Z_CAL_H_

//! @brief Enqueue the setup commands before the Live-Z pattern starts.
//!
//! Zeroes E, selects absolute positioning/relative extrusion, retracts, lifts
//! the nozzle and sets the acceleration used by the pattern.
void live_z_setup();

//! @brief Enqueue the Live-Z pattern in batches.
//!
//! Each call enqueues a limited number of commands so the command buffer is not
//! overflowed. Must be called repeatedly (each time the command buffer empties)
//! until it returns false. The pattern is a 75mm filled square (outer + inner
//! perimeter and solid infill), generated like the stock first layer square.
//!
//! @return true while more commands remain to be enqueued
bool live_z_print_step(float layer_height, float extrusion_width);

#endif /* FIRMWARE_LIVE_Z_CAL_H_ */