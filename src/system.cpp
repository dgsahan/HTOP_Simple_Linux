#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <unistd.h>
#include <ostream>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"
#include "iostream"

using std::set;
using std::size_t;
using std::string;
using std::vector;

#define KB_TO_MB 0.001

//---------------------------------------------------------------
// Return the system's CPU
Processor& System::Cpu()
{
    return cpu_;
}
//---------------------------------------------------------------

// Return a container composed of the system's processes
vector<Process>& System::Processes()
{ 
    processes_.clear();
    vector<int> pids = LinuxParser::Pids(); // Get the running process Ids

    if (pids.empty()) // Check for empty vector of Pids
    {
        throw std::invalid_argument("Cannot find a Pid");
    }

    for(auto id : pids)
    {
        Process proc; // Instantiate an empty process object
        proc.setPid(id); // Set the proces Id

        const std::string uid = LinuxParser::Uid(id);
         if(!std::all_of(uid.begin(), uid.end(), isdigit))
         {
            throw std::invalid_argument("Uid is not a number");
         }

        const std::string user = LinuxParser::User(std::stoi(uid)); // Get user name of Pid
        if(user.empty()) 
        {
            throw std::invalid_argument("User is not found for for the given Uid");
            
        }
        proc.setUser(user);

        const std::string command = LinuxParser::Command(id); // Get the process command
        if(command.empty())
        {
            throw std::invalid_argument("Command of the process is not found");
        }
        proc.setCommand(command);

        long int uptime = LinuxParser::UpTime(id); // Get the process uptime
        proc.setUptime(uptime/sysconf(_SC_CLK_TCK));

        std::string ram = LinuxParser::Ram(id); // Get the RAM of the process
        if(ram.empty())
        {
            throw std::invalid_argument("Ram of the Pid is nou found");
        }
        
        long iRam = std::stol(ram);
        iRam *= KB_TO_MB;
        ram = std::to_string(iRam);
        proc.setRam(ram);
        
        float cpu_usage;
        float processor_starttime = (float)UpTime();
        
        long total_jiffies = LinuxParser::ActiveJiffies(id); // Get active Process CPU jiffies
        float total_time = (float)total_jiffies / (float)sysconf(_SC_CLK_TCK);
        
        long start_jiffies = LinuxParser::StartTimeJiffies(id); // Process uptime
        float start_time = (float)start_jiffies  / (float)sysconf(_SC_CLK_TCK);

        cpu_usage = (total_time / (processor_starttime - start_time) ); // Calculate the process CPU usage
        proc.setCpuUtilization(cpu_usage);

        processes_.push_back(proc);
    }

    // Reorder the processes in ascending order based on CPU
    const int size =  processes_.size();
    if(size > 0)
    {
        vector<int> pos(size,0);
        vector<Process> temp(size);
        for(int i = 0 ; i < size - 1 ; i++)
        {
            Process a = processes_[i];
            
            for(int j = i+1 ; j < size ; j++)
            {
                Process b = processes_[j];

                if (a.operator<(b))
                {
                    pos[i] +=1;
                }
                else
                {
                    pos[j] +=1;
                }
            }
        }

        for(int k =  0; k < size ; k++)
        {
            temp[pos[k]] = processes_[k];
        }

        std::reverse(temp.begin(),temp.end());
        processes_ = temp;
    }

    return processes_; 
}
//---------------------------------------------------------------

// Return the system's kernel identifier (string)
std::string System::Kernel()
{ 
    // Return the system kernam identifier
    return LinuxParser::Kernel(); 
}
//---------------------------------------------------------------

// Return the system's memory utilization
float System::MemoryUtilization()
{ 
    return LinuxParser::MemoryUtilization(); 
}
//---------------------------------------------------------------

// Return the operating system name
std::string System::OperatingSystem()
{ 
    // Return the name of the operating system
    return LinuxParser::OperatingSystem();
}
//---------------------------------------------------------------

// Return the number of processes actively running on the system
int System::RunningProcesses()
{ 
    return LinuxParser::RunningProcesses(); 
}
//---------------------------------------------------------------

// Return the total number of processes on the system
int System::TotalProcesses()
{ 
    return LinuxParser::TotalProcesses(); 
}
//---------------------------------------------------------------

// Return the number of seconds since the system started running
long System::UpTime()
{ 
    return LinuxParser::UpTime();
}