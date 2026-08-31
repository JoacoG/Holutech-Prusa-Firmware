//! @file
//! @brief Custom first layer calibration print.
//!
//! Prints a large filled square (75mm x 75mm, centered on the bed) so the user
//! can tune the Live-Z offset across a wide area. The pattern is generated
//! programmatically (outer perimeter, inner perimeter and solid infill) to keep
//! the flash footprint minimal, following the same approach as the stock
//! meander/square in first_lay_cal.cpp.

#include "custom_first_layer_cal.h"
#include "Marlin.h"
#include "messages.h"
#include "cmdqueue.h"
#include <avr/pgmspace.h>
#include <math.h>

// Square geometry matching the Live-Z_Small object (75mm square centered on bed).
static constexpr float X_MIN = 87.5f;
static constexpr float Y_MIN = 67.5f;
static constexpr float X_MAX = 162.5f;
static constexpr float Y_MAX = 142.5f;

// Number of infill lines enqueued per batch. Keeps the command buffer from
// overflowing (each line is two commands: one horizontal + one vertical step).
static constexpr uint8_t INFILL_BATCH_LINES = 8;

// Batch state, reset in custom_lay1cal_before_print().
static uint16_t s_infill_line = 0;
static uint16_t s_infill_line_count = 0;
static bool s_infill_started = false;

//! @brief Count extrude length (same formula as first_lay_cal.cpp).
static constexpr float __attribute__((noinline)) count_e(float layer_height, float extrusion_width, float extrusion_length, float filament_diameter = 1.75f)
{
    return (extrusion_length * ((M_PI * pow(layer_height, 2)) / 4 + layer_height * (extrusion_width - layer_height))) / ((M_PI * pow(filament_diameter, 2)) / 4);
}

//! @brief Extrusion spacing (same formula as first_lay_cal.cpp).
static constexpr float spacing(float layer_height, float extrusion_width, float overlap_factor = 1.f)
{
    return extrusion_width - layer_height * (overlap_factor - M_PI / 4);
}

static const char extrude_fmt_X[] PROGMEM = "G1X%.4fE%.4f";
static const char extrude_fmt_Y[] PROGMEM = "G1Y%.4fE%.4f";

void custom_lay1cal_before_print()
{
    static const char cmd_zero[] PROGMEM = "G92E0";
    static const char cmd_abs[] PROGMEM = "G90";
    static const char cmd_retract[] PROGMEM = "G1E-1.5F2100";
    static const char cmd_lift[] PROGMEM = "G1Z5F7200";
    static const char cmd_accel[] PROGMEM = "M204S800";

    // Reset the batch state for a fresh print.
    s_infill_line = 0;
    s_infill_line_count = 0;
    s_infill_started = false;

    enquecommand_P(cmd_zero);
    enquecommand_P(cmd_abs);
    enquecommand_P(MSG_M83); // use relative distances for extrusion
    enquecommand_P(cmd_retract);
    enquecommand_P(cmd_lift);
    enquecommand_P(cmd_accel);
}

//! @brief Enqueue start position and perimeters, then initialize infill state.
static void custom_lay1cal_enqueue_header(float layer_height, float extrusion_width)
{
    const float w = extrusion_width;
    const float width = X_MAX - X_MIN;
    const float height = Y_MAX - Y_MIN;
    const float line_spacing = spacing(layer_height, extrusion_width);

    // Move to start position (absolute), set Z and feedrate, then relative mode.
    enquecommandf_P(PSTR("G1X%.3fY%.3f"), X_MIN, Y_MAX);
    enquecommandf_P(PSTR("G1Z%.2f"), layer_height);
    enquecommand_P(PSTR("G1F1200"));
    enquecommand_P(PSTR("G91")); // enable relative XYZ

    // Outer perimeter (clockwise from top-left).
    enquecommandf_P(extrude_fmt_X, width, count_e(layer_height, w, width));
    enquecommandf_P(extrude_fmt_Y, -height, count_e(layer_height, w, height));
    enquecommandf_P(extrude_fmt_X, -width, count_e(layer_height, w, width));
    enquecommandf_P(extrude_fmt_Y, height, count_e(layer_height, w, height));

    // Step diagonally to the inner perimeter start (inset by one extrusion width).
    enquecommandf_P(extrude_fmt_X, w, count_e(layer_height, w, w));
    enquecommandf_P(extrude_fmt_Y, -w, count_e(layer_height, w, w));

    // Inner perimeter (counter-clockwise).
    const float iw = width - 2 * w;
    const float ih = height - 2 * w;
    enquecommandf_P(extrude_fmt_X, iw, count_e(layer_height, w, iw));
    enquecommandf_P(extrude_fmt_Y, -ih, count_e(layer_height, w, ih));
    enquecommandf_P(extrude_fmt_X, -iw, count_e(layer_height, w, iw));
    enquecommandf_P(extrude_fmt_Y, ih, count_e(layer_height, w, ih));

    // Initialize infill state.
    s_infill_line = 0;
    s_infill_line_count = (uint16_t)(ih / line_spacing);
    s_infill_started = true;
}

//! @brief Enqueue one batch of solid infill lines.
//! @return true if more infill lines remain
static bool custom_lay1cal_enqueue_infill(float layer_height, float extrusion_width)
{
    const float w = extrusion_width;
    const float width = X_MAX - X_MIN;
    const float line_spacing = spacing(layer_height, extrusion_width);
    const float fill_width = width - 2 * w;

    for (uint8_t i = 0; i < INFILL_BATCH_LINES && s_infill_line < s_infill_line_count; ++i)
    {
        // Direction alternates: even lines go +X, odd lines go -X.
        const float xdir = (s_infill_line % 2 == 0) ? fill_width : -fill_width;
        enquecommandf_P(extrude_fmt_X, xdir, count_e(layer_height, w, fill_width));
        enquecommandf_P(extrude_fmt_Y, -line_spacing, count_e(layer_height, w, line_spacing));
        ++s_infill_line;
    }

    return s_infill_line < s_infill_line_count;
}

bool custom_lay1cal_print_step(float layer_height, float extrusion_width)
{
    if (!s_infill_started)
    {
        custom_lay1cal_enqueue_header(layer_height, extrusion_width);
        return true;
    }
    return custom_lay1cal_enqueue_infill(layer_height, extrusion_width);
}
