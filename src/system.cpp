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

using namespace std;

using std::set;
using std::size_t;
using std::string;
/*using std::vector;*/

// KB to MB conversion
#define KB_TO_MB 0.001

// Return the system's CPU
Processor& System::Cpu()
{
    return cpu_;
}
//---------------------------------------------------------------

// Return a container composed of the system's processes
vector<Process>& System::Processes()
{ 
    processes_.clear(); // Clear the vector at each run
    
    vector<int> pids = LinuxParser::Pids(); // Get the running process Ids
    if (pids.empty())
    {
        throw invalid_argument("Cannot find Pids");
    }

    // Loop through all process runing in the system
    for(auto id : pids)
    {
        Process proc;

        // Set the proces Id
        proc.setPid(id);

        const string uid = LinuxParser::Uid(id);
        int (*predicate)(int) = isdigit;
         if(!all_of(uid.begin(), uid.end(), predicate))
         {
            throw invalid_argument("Uid is not a number");
         }

        // Set user of the process
        const string user = LinuxParser::User(stoi(uid));
        if(user.empty()) 
        {
            throw invalid_argument("User is not found for the given Uid");
            
        }
        proc.setUser(user);

        // Set the command of the process
        const string command = LinuxParser::Command(id);
        proc.setCommand(command);

        // Set the process uptime
        // Process up time =  System up time - time process started
        long processUpTime =  LinuxParser::UpTime() - (LinuxParser::processStartTime(id)/sysconf(_SC_CLK_TCK));
        proc.setUptime(processUpTime);

        // Set the RAM of the process
        string ramString = LinuxParser::Ram(id);
        if(ramString.empty())
        {
            throw invalid_argument("Ram of the Pid is nou found");
        }
        long ramLong = KB_TO_MB * stol(ramString);
        proc.setRam(to_string(ramLong));
        
        // Set process CPU usage
        float processorStartTime = static_cast<float>(UpTime());
        
        long totalJiffies = LinuxParser::ActiveJiffies(id); // Get active Process CPU jiffies
        float totalTime = static_cast<float>(totalJiffies) / static_cast<float>(sysconf(_SC_CLK_TCK));
        
        long startJiffies = LinuxParser::StartTimeJiffies(id); // Process uptime
        float startTime = static_cast<float>(startJiffies)  /static_cast<float>(sysconf(_SC_CLK_TCK));

        float cpuUsage = (totalTime / (processorStartTime - startTime) ); // Calculate the process CPU usage
        proc.setCpuUtilization(cpuUsage);

        processes_.push_back(proc);
    }

    // Reorder the processes in descending order based on CPU
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

        reverse(temp.begin(),temp.end());
        processes_ = temp;
    }

    return processes_; 
}
//---------------------------------------------------------------

// Return the system's kernel identifier (string)
string System::Kernel()
{ 
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
string System::OperatingSystem()
{ 
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
//---------------------------------------------------------------