#ifndef PROCESSOR_H
#define PROCESSOR_H

/*
   The Processor class caclulate and return the system CPU
   usage.
   
   Reference: https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
*/
class Processor {
 public:
    float Utilization();  // See src/processor.cpp

 private:
    long prevTotalJiffies_ = 0; // Previous CPU total Jiffies
    long prevIdleJiffies_ = 0;  // Previous CPU idle Jiffies
};

#endif