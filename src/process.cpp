#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

//---------------------------------------------------------------
// Return this process's ID
int Process::Pid() 
{ 
    return Pid_;
}

void Process::setPid(int id)
{
  Pid_ = id;
}

//---------------------------------------------------------------
// Return this process's CPU utilization
float Process::CpuUtilization()
{ 
    return Cpu_; 
}

// Set Cpu utilization
void Process::setCpuUtilization(float cpu)
{
    Cpu_ = cpu;
}

//---------------------------------------------------------------
// Return the command that generated this process
string Process::Command()
{ 
    return Command_; 
}

void Process::setCommand(const std::string& sCommand)
{
    Command_ = sCommand.substr(0,40) + "...";
}

//---------------------------------------------------------------
// Return this process's memory utilization
string Process::Ram()
{ 

    return Ram_; 
}

void Process::setRam(const std::string& sRam)
{
    Ram_ = sRam;
}

//---------------------------------------------------------------
// Return the user (name) that generated this process
string Process::User() 
{ 
    return User_; 
}

void Process::setUser(const std::string& sUserName) 
{ 
    User_ = sUserName;
}

//---------------------------------------------------------------
// Return the age of this process (in seconds)
long int Process::UpTime() 
{ 
    return Uptime_; 
}

void Process::setUptime(long int uptime)
{
    Uptime_ = uptime;
}

//---------------------------------------------------------------
// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const 
{ 
    return this->Cpu_ > a.Cpu_;
}

//---------------------------------------------------------------