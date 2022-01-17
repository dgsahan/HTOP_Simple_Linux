#include "processor.h"
#include <vector>
#include <string>
#include <iostream>

#include "linux_parser.h"
using namespace LinuxParser;

// Return the CPU utilization
float Processor::Utilization()
{ 
    float cpu_usage; // System CPU usage

    // Calculate the reqired CPU states from the extracted parameters
    // Reference: https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
    
    long idle = LinuxParser::IdleJiffies(); // Number of idle jiffies
    long total = LinuxParser::Jiffies(); // Number of total jiffies

   
    long delta_total = total - prev_total_;
    long delta_idle = idle - prev_idle_;

    cpu_usage = ((float)(delta_total -delta_idle)) / ((float)delta_total); // System CPU usage

    // Preserve CPU usage data 
    prev_total_ = total;
    prev_idle_ = idle;

    return cpu_usage;
}