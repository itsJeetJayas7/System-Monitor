#include "process_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>



/*/proc/pid/smaps*/
void display_proc (long int cpu_time, procinfo_t * info) {
  long int cpu_usage = cpu_time;
  printf("Displaying Process(condensed)\n");
  printf("_______________________________________________\n");
  printf("Process Name\t Status\t CPU usage\t ID\t\t Memory\t\n");
  printf("%s\t\t %c\t %ld\t\t %d\t\t %d\t\n", info->process_name, info->state, cpu_usage, info->pid, info->program_size);
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

int create_proc_info (procinfo_t * self_info, int path) {
  //open stat file path
  char stat_path[1024];
  sprintf(stat_path,"/proc/%d/stat", path);
  //printf("%s\n", stat_path);
  FILE * stat_file;
  if ((stat_file = fopen(stat_path, "r"))) {
    fscanf(stat_file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %lu %llu", 
      &self_info->pid, self_info->process_name, &self_info->state, &self_info->ppid, &self_info->pgrp, &self_info->sid, 
      &self_info->tty_nr, &self_info->tty_pgrp, &self_info->flags, &self_info->min_flt, &self_info->cmin_flt, 
      &self_info->maj_flt, &self_info->cmaj_flt, &self_info->utime, &self_info->stime, &self_info->cutime, 
      &self_info->cstime, &self_info->prio, &self_info->nice, &self_info->num_threads, &self_info->itrealvalue, 
      &self_info->starttime);
      fclose(stat_file);
  } else {
     return 0;
  }
   
  //scan relevent information
  
  //print_stat(self_info);
  //printf("%s\n", self_info->process_name);
  //open statm file path
  char statm_path[1024];
  sprintf(statm_path,"/proc/%d/statm", path);
  FILE * statm_file = fopen(statm_path, "r");
  if ((statm_file = fopen(statm_path, "r"))) {

    //printf("%s\n", statm_path);
    //scan relevent information 
    fscanf(statm_file, "%d %d %d", 
    &self_info->program_size, &self_info->resident_size, 
    &self_info->shared_mem);
  
    //print_smem(self_info);
    fclose(statm_file);
    return 1;
  }
  else {
    return 0;
  }
  
}

void creat_mem_info(mem_map_info_t * mem_map) {
  //open statm file path
  char * map_path = "/proc/self/smaps";
  FILE * smap_file = fopen(map_path, "r");
  

  fscanf(smap_file, "%s %s %s %s %s %s\n", mem_map->VM, mem_map->flags, mem_map->offset, mem_map->dev, mem_map->inode, mem_map->file_name);
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
    "AnonHugePages:  kB\n"
    "ShmemPmdMapped:  kB\n"
    "FilePmdMapped:  kB\n"
    "Shared_Hugetlb: kB\n"
    "Private_Hugetlb: kB\n"
    "Swap: kB\n"
    "SwapPss: kB\n"
    "Locked: kB\n", 
   &mem_map->size, &mem_map->kernel_page_size, &mem_map->mmu_page_size, &mem_map->rss, &mem_map->pss, 
   &mem_map->shared_clean, &mem_map->shared_dirty, &mem_map->private_clean, &mem_map->private_dirty, 
   &mem_map->referenced, &mem_map->anonymous, &mem_map->lazy_free);
   
   /*printf(
    "Vm-stuff: %s\n" 
    "flags: %s\n"
    "offset: %s\n"
    "dev: %s\n"
    "inode: %s\n"
    "file_name %s\n"
    "SIZE: %d\n"
    "Rss: %d kB\n"
    "Pss: %d kB\n"
    "Shared_Clean: %d kB\n"
    "Shared_Dirty: %d kB\n"
    "Private_Clean: %d kB\n"
    "Private_Dirty: %d kB\n"
    "Referenced: %d kB\n"
    "Anonymous: %d kB\n"
    "LazyFree: %d kB\n"
    , mem_map->VM, mem_map->flags, mem_map->offset, mem_map->dev, mem_map->inode, mem_map->file_name, mem_map->size, mem_map->rss, mem_map->pss, 
   mem_map->shared_clean, mem_map->shared_dirty, mem_map->private_clean, mem_map->private_dirty, 
   mem_map->referenced, mem_map->anonymous, mem_map->lazy_free);*/
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

void create_single_process_info( procinfo_t * info) {
  info->pid = -999; 
  strcpy(info->process_name, "");;
  info->state = '~'; // R = Running; S = Sleeping, Z = Zombie, I = Idle
  info->ppid = 0; // partent pid
  info->pgrp = 0; // process group id
  info->sid = 0; // session id
  info->tty_nr = 0; // contoling terminal process
  info->tty_pgrp = 0; // controling termianl forground process
  info->flags  = 0; // flags 
  info->min_flt  = 0; // minor fault
  info->cmin_flt = 0; // minor faults children
  info->maj_flt = 0; // major faults
  info->cmaj_flt = 0; // major faults in children
  info->utime = 0;
  info->stime = 0;
  info->cutime = 0; // cpu time in user land measured in clock ticks
  info->cstime = 0; // cpu time in kearnal mode measured in clock ticks
  info->prio = 0; //  priority value 
  info->nice = 0; // nice value
  info->num_threads = 0; //number of threads
  info->itrealvalue = 0; //realvalue
  info->starttime = 0; //start time inticks
  
  //fields of smem
  info->program_size = 0; //program size in virtual memory
  info->resident_size = 0; //resident size in vitual memory 
  info->shared_mem = 0; //shared memory size
}

proc_list_t * list_view(int * pids) {
  int count = 0;
  proc_list_t * proc_list = (proc_list_t *) malloc(sizeof(proc_list_t));
  while (pids[count] != 0) {
    count++;
  }
  //printf("count: %d\n", count);
  proc_list->info_list =(procinfo_t **) malloc((count * sizeof(procinfo_t *)));
  proc_list->count = count;
  for (int i = 0; i < count; i++) {
    procinfo_t * info = (procinfo_t *) malloc(sizeof(procinfo_t));
    create_single_process_info(info);
    proc_list->info_list[i] = info;
    int flag = create_proc_info(proc_list->info_list[i], pids[i]);
    if (flag == 0) {
      free(proc_list->info_list[i]);
      proc_list->info_list[i] = NULL;
    } 
  }
  return proc_list;
}

void create_mount(mount_t * mount, char * path) {
  sscanf(path, "%s %s %s %s %s %s", mount->device, mount->total, mount->used, mount->available, mount->percent, mount->directory);
  FILE *file;
  char line [256];
  file = fopen("/proc/mounts", "r");
  while (fgets(line, sizeof(line), file) != NULL) {
    if (strstr(line, mount->device) != NULL) {
      sscanf(line, "%*s %*s %s", mount->type);
      break;
    }
  }
  fclose(file);
}

int main(int argc, char**argv) {
  //TODO phys mem
  
  FILE *fp;
  char path[1035];
  fp = popen("df -h", "r");
  mount_t ** mount_list = (mount_t **) malloc(100* sizeof(mount_t*));
  int count = 0;
  fgets(path, sizeof(path)-1, fp);
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    mount_t * mount = (mount_t *) malloc(sizeof(mount_t));
    create_mount(mount, path);
    mount_list[count] = mount;
    printf("%s %s %s %s %s %s %s\n", mount->device, mount->directory, mount->type, mount->total, mount->available, mount->used, mount->percent);
    count++;
  }

  int physical_memory_ph = 0;

  //procinfo_t * self_info = (procinfo_t *) malloc(sizeof(procinfo_t));
  
  mem_map_info_t * mem_map = (mem_map_info_t *) malloc(sizeof(mem_map_info_t));

  int * pids = get_pid();
  proc_list_t * proc_list = list_view(pids);
/*
  for (int i = 0; i < proc_list->count; i++) {
    if (proc_list->info_list[i] != NULL) {
      display_proc(0, proc_list->info_list[i]);
    }
  }
*/
 /*
  // creats proc info stuct
  create_proc_info(self_info, "/proc/self/");
  

  creat_mem_info(mem_map);
  
  
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
  */
 for (int i = 0; pids[i] != 0; i++) {
    if (proc_list->info_list[i] != NULL) {
      free(proc_list->info_list[i]);
      proc_list->info_list[i] == NULL;
    }
 }
 free(proc_list->info_list);
 free(proc_list);
 proc_list->info_list = NULL;
 proc_list = NULL;
 free(pids);
 pids = NULL;
  
}
