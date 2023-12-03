#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

typedef struct procinfo {
  
  //fields of stat
  int pid; 
  char process_name[24];
  char state; // R = Running; S = Sleeping, Z = Zombie, I = Idle
  int ppid; // partent pid
  int pgrp; // process group id
  int sid; // session id
  int tty_nr; // contoling terminal process
  int tty_pgrp; // controling termianl forground process
  unsigned flags; // flags 
  long unsigned min_flt; // minor fault
  long unsigned cmin_flt; // minor faults children
  long unsigned maj_flt; // major faults
  long unsigned cmaj_flt; // major faults in children
  long unsigned utime;
  long unsigned stime;
  long int cutime; // cpu time in user land measured in clock ticks
  long int cstime; // cpu time in kearnal mode measured in clock ticks
  long int prio; //  priority value 
  long int nice; // nice value
  long int num_threads; //number of threads
  long int itrealvalue; //realvalue
  long long unsigned starttime; //start time inticks
  
  //fields of smem
  int program_size; //program size in virtual memory
  int resident_size; //resident size in vitual memory 
  int shared_mem; //shared memory size
} procinfo_t;

#endif