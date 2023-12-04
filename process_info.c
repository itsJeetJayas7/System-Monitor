#include "process_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>



/*/proc/pid/smaps*/
void display_proc (int pid, long int cpu_time, char process_name[24], int program_size, char state) {
  long int cpu_usage = cpu_time;
  printf("Displaying Process(condensed)\n");
  printf("_______________________________________________\n");
  printf("Process Name\t Status\t CPU usage\t ID\t\t Memory\t\n");
  printf("%s\t\t %c\t %ld\t\t %d\t\t %d\t\n", process_name, state, cpu_usage, pid, program_size);
  printf("_______________________________________________\n\n");
}

void detailed_view (char process_name[24], char state, int memory, int virtual_memory, int resident_memory, int shared_memory, long int cpu_time, long long unsigned start_time, long int nice, long int priority, int pid) {
  
  printf("Detailed View\n");
  printf("_______________________________________________\n");
  printf(
    "Process Name:\t\t %s\n"
    "User:\t\t\t Unkown\n" // TODO: get user
    "Status:\t\t\t %c\n"
    "Memory:\t\t\t UNKNOWN%d\n" // TODO: get physcial memory usage
    "Vitual Memory:\t\t %d\n"
    "Resident Memory:\t %d\n"
    "Shared Memory:\t\t %d\n"
    "CPU time:\t\t %ld\n"
    "time started:\t\t %llu\n"
    "Nice:\t\t\t %ld\n"
    "Priority:\t\t %ld\n"
    "ID:\t\t\t %d\n",
    process_name, state, memory, virtual_memory, resident_memory, shared_memory, cpu_time, start_time, nice, priority, pid
  );
  printf("_______________________________________________\n\n");
}

void print_stat(procinfo_t * info) {
  printf("Stats Printed\n");
  printf("_______________________________________________\n");
  printf( 
    "pid: %d\n"
    "file name: %s\n"
    "state: %c\n"
    "ppid: %d\n"
    "pgrp: %d\n" 
    "session ID: %d\n"
    "tty_nr: %d\n"
    "tpgid: %d\n"
    "flags: %u\n"
    "minflt: %lu\n" 
    "cminflt: %lu\n"
    "majflt: %lu\n" 
    "cmajflt: %lu\n" 
    "utime: %lu\n"
    "stime: %lu\n"
    "cutime: %ld\n"
    "cstime: %ld\n"
    "priority: %ld\n"
    "nice: %ld\n"
    "numthreads: %ld\n"
    "itrealvalue: %lu\n"
    "starttime: %llu\n",
    info->pid, info->process_name, info->state, info->ppid, info->pgrp, 
    info->sid, info->tty_nr, info->tty_pgrp, info->flags, 
    info->min_flt, info->cmin_flt, info->maj_flt, info->cmaj_flt, 
    info->utime, info->stime, info->cutime, info->cstime, 
    info->prio, info->nice, info->num_threads, info->itrealvalue, 
    info->starttime
  );
  printf("_______________________________________________\n\n");
}

void print_smem (procinfo_t * info) {
  printf("Memory Stats Printed\n");
  printf("_______________________________________________\n");
  printf(
    "Memory: %d\n"
    "resident: %d\n"
    "shared mem size: %d\n",
    info->program_size, info->resident_size, info->shared_mem
  );
  printf("_______________________________________________\n\n");
}

void * create_proc_info (procinfo_t * self_info, char * path) {
  //open stat file path
  char * stat_path = "/proc/self/stat";
  FILE * stat_file = fopen(stat_path, "r");
  
  //scan relevent information
  fscanf(stat_file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %lu %llu", 
  &self_info->pid, self_info->process_name, &self_info->state, &self_info->ppid, &self_info->pgrp, &self_info->sid, 
  &self_info->tty_nr, &self_info->tty_pgrp, &self_info->flags, &self_info->min_flt, &self_info->cmin_flt, 
  &self_info->maj_flt, &self_info->cmaj_flt, &self_info->utime, &self_info->stime, &self_info->cutime, 
  &self_info->cstime, &self_info->prio, &self_info->nice, &self_info->num_threads, &self_info->itrealvalue, 
  &self_info->starttime);

  //open statm file path
  char * statm_path = "/proc/self/statm";
  FILE * statm_file = fopen(statm_path, "r");
  
  //scan relevent information 
  fscanf(statm_file, "%d %d %d ", 
  &self_info->program_size, &self_info->resident_size, 
  &self_info->shared_mem);
}

void creat_mem_info() {
  //open statm file path
  char * map_path = "/proc/self/smaps";
  FILE * smap_file = fopen(map_path, "r");
  char VM[32];
  char flags[32];
  char offset[32];
  char dev[32];
  char inode[32];
  char file_name[1024];
  int size;
  int kernel_page_size;
  int mmu_page_size;
  int rss;
  int pss;
  int shared_clean;
  int shared_dirty;
  int private_clean;
  int private_dirty;
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
  

  fscanf(smap_file, "%s %s %s %s %s %s\n", VM, flags, offset, dev, inode, file_name);
  fscanf(smap_file,
    "Size: %d kB\n"
    "KernelPageSize: %d kB\n"
    "MMUPageSize: %d kB\n"
    "Rss: %d kB\n"
    "Pss: %d kB\n"
    "Shared_Clean: %d kB\n"
    "Shared_Dirty: %d kB\n"
    "Private_Clean: %d kB\n"
    "Private_Dirty: %d kB\n"
    "Referenced: %d kB\n"
    "Anonymous: %d kB\n"
    "LazyFree: %d kB\n"
    "AnonHugePages: %d kB\n"
    "ShmemPmdMapped: %d kB\n"
    "FilePmdMapped: %d kB\n"
    "Shared_Hugetlb: %d kB\n"
    "Private_Hugetlb: %d kB\n"
    "Swap: %d kB\n"
    "SwapPss: %d kB\n"
    "Locked: %d kB\n", 
   &size, &kernel_page_size, &mmu_page_size, &rss, &pss, 
   &shared_clean, &shared_dirty, &private_clean, &private_dirty, 
   &referenced, &anonymous);
   printf(
    "Vm-stuff:%s\n" 
    "flags: %s\n"
    "offset: %s\n"
    "dev: %s\n"
    "inode: %s\n"
    "file_name %s\n"
    "SIZE: %d\n"
    , VM, flags, offset, dev, inode, file_name, size);
    
  
  
}



int * get_pid() {
  DIR * dir = opendir("/proc/");
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    // Count the number of entries in the directory
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (atoi(entry->d_name) > 0) {
            count++;
        }
    }

    // Allocate memory based on the count
    int* pids = malloc(count * sizeof(int));
    if (pids == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Reset the directory stream
    rewinddir(dir);

    // Read the process IDs
    count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (atoi(entry->d_name) > 0) {
            pids[count] = atoi(entry->d_name);
            count++;
        }
    }
    closedir(dir);
    return pids;
}


int main(int argc, char**argv) {
  //TODO phys mem
  int physical_memory_ph = 0;

  procinfo_t * self_info = (procinfo_t *) malloc(sizeof(procinfo_t));
  //mem_map_info_t * mem_map = (mem_map_info_t *) malloc(sizeof(mem_map_info_t));


  // creats proc info stuct
  create_proc_info(self_info, "/proc/self/");
  

  creat_mem_info();
  
  /*
  printf("-----------------------------------------------\n\n");
  
  // prints the smem data
  print_smem(self_info);
  
  
  printf("-----------------------------------------------\n\n");
  
  //prints stat data with labels
  print_stat(self_info);
  
  
  printf("-----------------------------------------------\n\n");
  long int cpu_time = self_info->cutime + self_info->cstime;
  
  //formatted display of the proc
  display_proc(self_info->pid, cpu_time, self_info->process_name, self_info->program_size, self_info->state);
  
  printf("-----------------------------------------------\n\n");
  
  //detailed veiw of proc 
  detailed_view(self_info->process_name, self_info->state, physical_memory_ph, self_info->program_size, self_info->resident_size, 
  self_info->shared_mem, cpu_time, self_info->starttime, self_info->nice, self_info->prio, self_info->pid);
  
l  */
  
}
