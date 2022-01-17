#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  int Pid();                               // See src/process.cpp
  std::string User();                      // See src/process.cpp
  std::string Command();                   // See src/process.cpp
  float CpuUtilization();                  // See src/process.cpp
  std::string Ram();                       // See src/process.cpp
  long int UpTime();                       // See src/process.cpp
  bool operator<(Process const& a) const;  // See src/process.cpp
  
  // Mutable to set the private members
  void setPid(int id);
  void setCommand(const std::string& sCommand);
  void setUptime(long int uptime);
  void setRam(std::string& ram);
  void setUser(const std::string& user_name);
  void setCpuUtilization(float cpu);

  // Declare any necessary private members
 private:
    int Pid_;
    std::string User_;
    std::string Command_;
    float Cpu_;
    std::string Ram_;
    long int Uptime_;
};

#endif