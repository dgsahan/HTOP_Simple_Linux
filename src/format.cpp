#include <string>

#include "format.h"

using std::string;

#define SEC_FOR_HOUR 3600
#define SEC_FOR_MIN 60

/*
    This function takes time in seconds and output
    the time in HH:MM:SS format. 
    
    The formatting could be done using date::format()
    method as well.
*/
    // INPUT: Long int measuring seconds
    // OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds)
{ 
    int hour = seconds / SEC_FOR_HOUR;
    int mins = (seconds % SEC_FOR_HOUR) / SEC_FOR_MIN;
    int secs =  seconds % SEC_FOR_MIN;

    string hh = hour < 10 ? "0" + std::to_string(hour) : std::to_string(hour);
    string mm = mins < 10 ? "0" + std::to_string(mins) : std::to_string(mins);
    string ss = secs < 10 ? "0" + std::to_string(secs) : std::to_string(secs);

    return hh + ":" + mm + ":" + ss; 
}