#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H


typedef struct procinfo {
  char state_str[32];
  char cpu_perc_str[32];
  char pid_str[32];
  char phys_mem_str[32];
  char virtual_mem_str[32];
  char resident_mem_str[32];
  char shared_mem_str[32];
  char cpu_time_str[32];
  char nice_str[32];
  char prio_str[32];

  
  
  //fields of stat
  int pid; 
  char process_name[32];
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
  long unsigned vsize; //virtual memory size
  long unsigned rss; //physical memory size
  double cpu_percent; //cpu percentage
  
  //fields of smem
  int program_size; //program size in virtual memory
  int resident_size; //resident size in vitual memory 
  int shared_mem; //shared memory size
} procinfo_t;

typedef struct proc_list {
  procinfo_t ** info_list;
  int count;
} proc_list_t;

typedef struct mem_map_info {
  char file_name[1024];
  char VM[32];
  char size_str[32];
  char flags[32];
  char offset[32];
  char private_clean_str[32];
  char private_dirt_str[32];
  char shared_clean_str[32];
  char shared_dirty_str[32];
  
  char vm_start[32];
  char vm_end[32];

  int size;
  int private_clean;
  int private_dirty;
  int shared_clean;
  int shared_dirty;

  char dev[32];
  char inode[32];
  int kernel_page_size;
  int mmu_page_size;
  int rss;
  int pss;
  int referenced;
  int anonymous;
  int lazy_free;
  int anon_huge_pages;
  int shmem_pmd_mapped;
  int file_pmd_mapped;
  int shared_hugetlb;
  int private_hugetlb;
  int swap;
  int swap_pss;
  int locked;
  int th_peligible;
  int protection_key;
}mem_map_info_t;

typedef struct mem_map_list {
  mem_map_info_t ** maps_list;
  int count;
} mem_map_list_t;

typedef struct fd_info {
  int fd;
  char type[1024];
  char object[1024];
  char fd_str[16];
} fd_info_t;

typedef struct fd_list {
  fd_info_t ** fd_array;
  int count;
} fd_list_t;

typedef struct mount {
  char device[128];
  char directory[128];
  char type[32];
  char total[32];
  char available[32];
  char used[32];
  char percent[32];
} mount_t;


#endif
