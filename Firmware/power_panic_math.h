#pragma once

#include <stdint.h>

// Computes the host-print resume g-code line number at power panic: the last
// acknowledged line minus the commands still buffered in the planner
// (planner_lines) and in the command queue (buffer_blocks).
// Kept pure and side-effect free so the recovery arithmetic can be exercised
// by host-side unit tests.
static inline uint32_t power_panic_host_resume_line(long gcode_last_n,
                                                    uint8_t planner_lines,
                                                    int buffer_blocks)
{
    uint32_t resume_line = (uint32_t)gcode_last_n;
    resume_line -= planner_lines;
    resume_line -= buffer_blocks;
    return resume_line;
}