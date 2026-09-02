//! @file
//! @brief Custom first layer calibration print (programmatically generated square)

#ifndef FIRMWARE_CUSTOM_FIRST_LAYER_CAL_H_
#define FIRMWARE_CUSTOM_FIRST_LAYER_CAL_H_

//! @brief Enqueue the custom short intro line (matches the Live-Z G-code).
void custom_lay1cal_intro_line();

//! @brief Enqueue setup commands before printing the custom pattern.
//! Resets the internal batch state for a fresh print.
void custom_lay1cal_before_print();

//! @brief Enqueue the custom square print in batches.
//!
//! Each call enqueues a limited number of commands so the command buffer is not
//! overflowed. Must be called repeatedly (each time the command buffer empties)
//! until it returns false.
//!
//! @return true while more commands remain to be enqueued
bool custom_lay1cal_print_step(float layer_height, float extrusion_width);

#endif /* FIRMWARE_CUSTOM_FIRST_LAYER_CAL_H_ */
