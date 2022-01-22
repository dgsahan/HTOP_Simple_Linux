#include "processor.h"
#include <vector>
#include <string>
#include <iostream>

#include "linux_parser.h"
using namespace LinuxParser;

// Return the System CPU utilization
float Processor::Utilization()
{ 
    float SystemCpuUsage;

    long idleJiffies = LinuxParser::IdleJiffies(); // Get number of idle jiffies
    long totalJiffies = LinuxParser::Jiffies(); // Getof number of total jiffies

   
    long deltaTotal = totalJiffies - prevTotalJiffies_;
    long deltaIdle = idleJiffies - prevIdleJiffies_;

    SystemCpuUsage = (static_cast<float> (deltaTotal - deltaIdle) / static_cast<float>(deltaTotal)); // System CPU usage

    // Preserve CPU data 
    prevTotalJiffies_ = totalJiffies;
    prevIdleJiffies_ = idleJiffies;

    return SystemCpuUsage;
}