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

// requires a higher version of g++
# if __has_include(<filesystem>)
    #include <filesystem>  
    namespace fs = std::filesystem;  
#else
    // works for virtual machine version ==> requires target_link_libraries(... stdc++fs) in 13:CMakeLists.txt
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
# endif

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
  float memTotal = 0.0f, memFree = 0.0f;
  bool memTotalFound =false;
  bool memFreeFound =false;
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
          memTotal = stof(value);
          memTotalFound = true;
        }
        // Free memeory
        else if(key == "MemFree:")
        {
          memFree = stof(value);
          memFreeFound = true;
        }

        // Require data found and exit the loop
        if(memTotalFound && memFreeFound)
        {
          break;
        }
      }
    }
  }
  
  // Return the percentage of memory utilization
  if (memFreeFound && memTotalFound)
  {
    return (memTotal - memFree)/ memTotal;
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
  
  return std::stol(uptime);
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
  long totalJiffies = 0;
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
        totalJiffies += std::stol(item);;
      }
      i++;
    }
  }
  return totalJiffies;
}

//--------------------------------------------------------
// Read and return the number of idle jiffies for the system
long LinuxParser::StartTimeJiffies(int pid) 
{ 
  long startTimeJiffies;
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
        startTimeJiffies = std::stol(item);
        goto end;
      }
      i++;
    }
    end: ;
  }
  return startTimeJiffies;
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
  int totalProcesses = 0;
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
          totalProcesses = stof(value);
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
    return totalProcesses;
  }
  
  cout << "Total processes are not found \n";
  return 0; // not valid
}

//--------------------------------------------------------
// Read and return the number of running processes
int LinuxParser::RunningProcesses()
{ 
  int runningProcesses = 0;
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
          runningProcesses = stof(value);
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
    return runningProcesses;
  }
  
  cout << "Running processes are not found \n";
  return 0; // not valid
}

//--------------------------------------------------------
// Read and return the command associated with a process
string LinuxParser::Command(int pid) 
{ 
  string sPid = std::to_string(pid);
  string line; // Initialize to empty string
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
long LinuxParser::processStartTime(int pid) 
{ 
  long int processStartTime = 0; // Time at process start after boot
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
          processStartTime = std::stol(value);
          break;
        }
        i++;
      }
  }
  
  return processStartTime; 
}
//--------------------------------------------------------
