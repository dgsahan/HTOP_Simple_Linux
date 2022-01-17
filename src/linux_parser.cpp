#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <experimental/filesystem>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::cout;

namespace fs = std::experimental::filesystem;

//--------------------------------------------------------
// Return OS string
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

//--------------------------------------------------------
// Return Kernal of the system
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

//--------------------------------------------------------
vector<int> LinuxParser::Pids()
{
  vector<int> pids;
  //string path = kProcDirectory;
  const fs::path path{kProcDirectory};
  for(const auto & entry : fs::directory_iterator(path))
  {
    if(fs::is_directory(entry.symlink_status()))
    {
      const string filename = entry.path().filename().string();
      if (std::all_of(filename.begin(), filename.end(), isdigit))
      {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

//--------------------------------------------------------
// Read and return the system memory utilization
float LinuxParser::MemoryUtilization()
{ 
  float mem_total = 0.0f, mem_free = 0.0f;
  bool mem_totalFound =false;
  bool mem_freeFound =false;
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      std::istringstream linestream(line);
      while (linestream >> key >> value)
      {
        
        // Total memeory
        if (key == "MemTotal:")
        {
          mem_total = stof(value);
          mem_totalFound = true;
        }
        // Free memeory
        else if(key == "MemFree:")
        {
          mem_free = stof(value);
          mem_freeFound = true;
        }

        // Require data found and exit the loop
        if(mem_totalFound && mem_freeFound)
        {
          break;
        }
      }
    }
  }
  
  // Return the percentage of memory utilization
  if (mem_freeFound && mem_totalFound)
  {
    return (mem_total - mem_free)/ mem_total;
  }
  
  return 0.0f; // not valid
}

//--------------------------------------------------------
// Read and return the system uptime
long LinuxParser::UpTime() 
{
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return (std::stol(uptime));
}

//--------------------------------------------------------
// Read and return the number of jiffies for the system
long LinuxParser::Jiffies()
{ 
  long nonIdle = ActiveJiffies(); // Number of total active jiffies
  long idle  = IdleJiffies(); // nNmber of idle jiffies

  return nonIdle + idle; // Total number of jiffies in the system
}

//--------------------------------------------------------
// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid)
{ 
  long total_jiffies = 0;
  string sPid = std::to_string(pid);
  string line;
  string item;
  std::ifstream filestream(kProcDirectory + "/"+ sPid + kStatFilename);
  
  if (filestream.is_open())
  {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    
    int i = 1;
    while (linestream >> item)
    {
      // utime + stime + cutime + cstime
      if ((i == 14) || (i == 15) || (i == 16) || (i == 17))
      {
        total_jiffies += std::stol(item);;
      }
      i++;
    }
  }
  return total_jiffies;
}

//--------------------------------------------------------
// Read and return the number of idle jiffies for the system
long LinuxParser::StartTimeJiffies(int pid) 
{ 
  long startTime_jiffies;
  string sPid = std::to_string(pid);
  string line;
  string item;
  std::ifstream filestream(kProcDirectory + "/"+ sPid + kStatFilename);
  
  if (filestream.is_open())
  {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    
    int i = 1;
    while (linestream >> item)
    {
      // utime + stime + cutime + cstime
      if (i == 22)
      {
        startTime_jiffies = std::stol(item);
        goto end;
      }
      i++;
    }
    end: ;
  }
  return startTime_jiffies;
}

//--------------------------------------------------------
// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies()
{ 
  std::vector<std::string> sCpu;
  long lCpu = 0;
  sCpu = LinuxParser::CpuUtilization();
  
  if (sCpu.empty())
    {
        std::cout << "Cannot read processor data \n";
        return 0;
    }

    int j = kUser_;
    for(auto i : sCpu)
    {
      if( (j != kIdle_ || j != kIOwait_) && (j < kGuest_))
      {
        lCpu += std::stol(i);
      }
      
      j++;
    }

  return lCpu; // Total number active jiffies in the system
}

//--------------------------------------------------------
// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() 
{ 
  std::vector<std::string> sCpu;
  long lCpu = 0;
  sCpu = LinuxParser::CpuUtilization();
  
  if (sCpu.empty())
    {
        std::cout << "Cannot read processor data \n";
        return 0;
    }

    int j = kUser_;
    for(auto i : sCpu)
    {
      if(j == kIdle_ || j == kIOwait_)
      {
        lCpu += std::stol(i);
      }
      
      j++;
    }

  return lCpu; // Total number idle jiffies in the system
  
}

//--------------------------------------------------------
// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization()
{ 
  vector<string> sCpu;
  bool found = false;
  string line;
  string item;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      if(found)
      {
        break;
      }

      std::istringstream linestream(line);
      while (linestream >> item)
      {
        if(found)
        {
          sCpu.push_back(item);
        }

        if(item == "cpu")
        {
          found = true;
        }

      }
    }
  }
  
  // Return the percentage of memory utilization
  if (found)
  {
    return sCpu;
  }
  
  cout << "Total processes are not found \n";
  return sCpu; // not valid
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses()
{ 
  int total_processes = 0;
  bool found = false;
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      std::istringstream linestream(line);
      while (linestream >> key >> value)
      {
        // Total memeory
        if (key == "processes")
        {
          total_processes = stof(value);
          found = true;
        }
        // Require data found and exit the loop
        if(found)
        {
          break;
        }
      }
    }
  }
  
  // Return the percentage of memory utilization
  if (found)
  {
    return total_processes;
  }
  
  cout << "Total processes are not found \n";
  return 0; // not valid
}

//--------------------------------------------------------
// Read and return the number of running processes
int LinuxParser::RunningProcesses()
{ 
  int running_processes = 0;
  bool found = false;
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      std::istringstream linestream(line);
      while (linestream >> key >> value)
      {
        // Total memeory
        if (key == "procs_running")
        {
          running_processes = stof(value);
          found = true;
        }
        // Require data found and exit the loop
        if(found)
        {
          break;
        }
      }
    }
  }
  
  // Return the percentage of memory utilization
  if (found)
  {
    return running_processes;
  }
  
  cout << "Running processes are not found \n";
  return 0; // not valid
}

//--------------------------------------------------------
// Read and return the command associated with a process
string LinuxParser::Command(int pid) 
{ 
  string sPid = std::to_string(pid);
  string line;
  std::ifstream stream(kProcDirectory + "/" + sPid + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

//--------------------------------------------------------
// Read and return the memory used by a process
string LinuxParser::Ram(int pid)
{ 
  string ramSize;
  string sPid = std::to_string(pid);
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + "/" + sPid + kStatusFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      std::istringstream linestream(line);
      while (linestream >> key >> value)
      {
        // Total memeory
        if (key == "VmSize:")
        {
          ramSize = value;
          goto end;
        }
      }
    }
    end: ;
  }
  return ramSize;
}

//--------------------------------------------------------
// Read and return the user ID associated with a process
string LinuxParser::Uid(const int pid)
{ 
  string uid;
  string sPid = std::to_string(pid);
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + "/" + sPid + kStatusFilename);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      std::istringstream linestream(line);
      while (linestream >> key >> value)
      {
        // Total memeory
        if (key == "Uid:")
        {
          uid = value;
          goto end;
        }
      }
    }
    end: ;
  }
  return uid; 
}

//--------------------------------------------------------
// Read and return the user associated with a process
string LinuxParser::User(const int uid)
{ 
  string user;
  string sPid = std::to_string(uid);
  string line;
  string username;
  string token;
  vector<string> output;
  string processId;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      std::istringstream linestream(line);
      
      output.clear();

      for(int i = 0 ; i < 3 ; i++)
      {
        std::getline(linestream, token,':');
        output.push_back(token);
      }

      if ((output[1] == "x") && (output[2] == sPid))
      {
          user = output[0];
          goto end;
      }
      
    }
    end: ;
  }

  return user; 
}

//--------------------------------------------------------
// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) 
{ 
  long int uptime = 0;
  string sPid = std::to_string(pid);
  string line;
  string value;
  std::ifstream stream(kProcDirectory + "/" + sPid + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    int i = 1;
    while (linestream >> value)
      {
        // Total memeory
        if (i == 22)
        {
          uptime = std::stol(value);
          break;
        }
        i++;
      }
  }
  
  return uptime; 
}
//--------------------------------------------------------
