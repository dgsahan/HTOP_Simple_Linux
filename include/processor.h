#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
    float Utilization();  // See src/processor.cpp

  // Declare any necessary private members
 private:
    long prev_total_ = 0; // Previous CPU total usage
    long prev_idle_ = 0;  // Previous CPU idle time
};

#endif