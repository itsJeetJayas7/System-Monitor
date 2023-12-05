#include "process_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


/* Displays process information of a signal PID */
void display_proc (long int cpu_time, procinfo_t * info) {
  long int cpu_usage = cpu_time;
  printf("Displaying Process(condensed)\n");
  printf("_______________________________________________\n");
  printf("Process Name\t Status\t CPU usage\t ID\t\t Memory\t\n");
  printf("%s\t\t %c\t %ld\t\t %d\t\t %d\t\n", info->process_name, info->state, cpu_usage, info->pid, info->program_size);
  printf("_______________________________________________\n\n");
}

void print_mem_map(mem_map_info_t * mem_map) {
  
   printf(
    "Vm-stuff: %s\n" 
    "flags: %s\n"
    "offset: %s\n"
    "dev: %s\n"
    "inode: %s\n"
    "file_name: %s\n"
    "SIZE: %d kB\n"
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
   mem_map->referenced, mem_map->anonymous, mem_map->lazy_free);
}

/* Displays detailed information of a single process */
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

/* prints out detailed informaton obtained form the /proc/PID/stat file */
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

/* prints infomation from the /proc/PID/smem file */
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
  FILE * stat_file;
  if ((stat_file = fopen(stat_path, "r"))) {

    //scan relevent information
    fscanf(stat_file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %lu %llu %lu %ld", 
      &self_info->pid, self_info->process_name, &self_info->state, &self_info->ppid, &self_info->pgrp, &self_info->sid, 
      &self_info->tty_nr, &self_info->tty_pgrp, &self_info->flags, &self_info->min_flt, &self_info->cmin_flt, 
      &self_info->maj_flt, &self_info->cmaj_flt, &self_info->utime, &self_info->stime, &self_info->cutime, 
      &self_info->cstime, &self_info->prio, &self_info->nice, &self_info->num_threads, &self_info->itrealvalue, 
      &self_info->starttime, &self_info->vsize, &self_info->rss);
      fclose(stat_file);

      //get cpu uptime for cpu % calcualtion 
      FILE * uptime_file = fopen("/proc/uptime", "r");
      double uptime_sec;
      fscanf(uptime_file, "%lf ", &uptime_sec);
      fclose(uptime_file);

      //calcualte the cpu%
      double utime_sec = self_info->utime / 100;
      double stime_sec = self_info->stime / 100;
      double start_time_sec = self_info->starttime / 100;
      double proc_elapse_sec = uptime_sec - start_time_sec;
      double proc_usage_sec = utime_sec + stime_sec;
      double proc_usage_percent = proc_usage_sec  * 100 / proc_elapse_sec;
      self_info->cpu_percent = proc_usage_percent;

  } else {
    //return 0 on faliure to open file
     return 0;
  }
   
  //open statm file path
  char statm_path[1024];
  sprintf(statm_path,"/proc/%d/statm", path);
  FILE * statm_file = fopen(statm_path, "r");

  //open proc file and scan for the relevent information
  if ((statm_file = fopen(statm_path, "r"))) {
    //scan relevent information 
    fscanf(statm_file, "%d %d %d", 
    &self_info->program_size, &self_info->resident_size, 
    &self_info->shared_mem);
  
    //print_smem(self_info);
    fclose(statm_file);
    return 1;
  }
  else {
    //return 0 on failiure 
    return 0;
  }
  
}

void create_empty_map(mem_map_info_t * map_info) {
  strcpy(map_info->file_name, "None");
  strcpy(map_info->VM, "None");
  strcpy(map_info->size_str, "None");
  strcpy(map_info->flags, "None");
  strcpy(map_info->private_clean_str, "None");
  strcpy(map_info->private_dirt_str, "None");
  strcpy(map_info->shared_clean_str, "None");
  strcpy(map_info->shared_dirty_str, "None");
  strcpy(map_info->vm_start, "None");
  strcpy(map_info->vm_end, "None");
  map_info->size = 0;
  map_info->private_clean = 0;
  map_info->private_dirty = 0;
  map_info->shared_clean = 0;
  map_info->shared_dirty = 0;
  strcpy(map_info->dev, "None");
  strcpy(map_info->inode, "None");
  map_info->kernel_page_size = 0;
  map_info->mmu_page_size = 0;
  map_info->rss = 0;
  map_info->pss = 0;
  map_info->referenced = 0;
  map_info->anonymous = 0;
  map_info->lazy_free = 0;
  map_info->anon_huge_pages = 0;
  map_info->shmem_pmd_mapped = 0;
  map_info->file_pmd_mapped = 0;
  map_info->shared_hugetlb = 0;
  map_info->private_hugetlb = 0;
  map_info->swap = 0;
  map_info->swap_pss = 0;
  map_info->locked = 0;
  map_info->th_peligible = 0;
  map_info->protection_key = 0;
}

void format_mem_map(mem_map_info_t * map_info) {
  
  // Use strtok to split the string
  char *token = strtok(map_info->VM, "-");
  strcpy(map_info->vm_start, token);
  token = strtok(NULL, "-");
  strcpy(map_info->vm_end, token);
  
  sprintf(map_info->size_str, "%d", map_info->size);
  sprintf(map_info->private_clean_str, "%d", map_info->private_clean);
  sprintf(map_info->private_dirt_str, "%d", map_info->private_dirty);
  sprintf(map_info->shared_clean_str, "%d", map_info->shared_clean);
  sprintf(map_info->shared_dirty_str, "%d", map_info->shared_dirty);
  
}

mem_map_list_t * create_mem_info(int pid) {
  //open statm file path
  char map_path[1024];
  sprintf(map_path,"/proc/%d/smaps", pid);
  FILE * smap_file;
  if((smap_file = fopen(map_path, "r"))) {
    //line buffer
    char line_ph[1024];
    int mem_map_flag = 0;
    
    mem_map_list_t * smap_list = (mem_map_list_t *) malloc(sizeof(mem_map_list_t));
    smap_list->count = 0;
    smap_list->maps_list =( mem_map_info_t **) malloc(((smap_list->count + 10) * sizeof(mem_map_info_t *)));
    

    while (mem_map_flag == 0) {
      
      mem_map_info_t * mem_map = (mem_map_info_t *) malloc(sizeof(mem_map_info_t));
      create_empty_map(mem_map);

      smap_list->maps_list = (mem_map_info_t **) realloc(smap_list->maps_list, ((smap_list->count + 10) * sizeof(mem_map_info_t *)));
      smap_list->maps_list[smap_list->count] = mem_map;

      if (fgets(line_ph, sizeof(line_ph), smap_file) != NULL) {
        sscanf(line_ph, "%s %s %s %s %s %s\n", mem_map->VM, mem_map->flags, mem_map->offset, 
          mem_map->dev, mem_map->inode, mem_map->file_name);
      }
      else {
        mem_map_flag = 1;
        return smap_list;
      }
      
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

      
      for (int i = 0; i < 11; ++i) {
        if (fgets(line_ph, sizeof(line_ph), smap_file) != NULL) {
          continue;
        }
        else {
          mem_map_flag = 1;
          
        }
      }
      //print_mem_map(smap_list->maps_list[count]);
      format_mem_map(mem_map);
      
      smap_list->count++;
    }
    
  }
  else {
    return NULL;
  }
  
  
   
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
  strcpy(info->process_name, "");
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



fd_list_t * open_file_list(int pid) {
  
  fd_list_t * fd_list = (fd_list_t *) malloc(sizeof(fd_list_t));

  char fd_dir_path[1024];
  sprintf(fd_dir_path,"/proc/%d/fd", pid);

  DIR * fd_dir = opendir(fd_dir_path);
  struct dirent * dir_struc;

  fd_list->count = 0;
  fd_list->fd_array =(fd_info_t **) malloc(((fd_list->count + 5) * sizeof(fd_info_t *)));

  while((dir_struc = readdir(fd_dir)) != NULL) {
    
    if (!strcmp(dir_struc->d_name, ".")) {
      
      continue;
    }
    if (!strcmp(dir_struc->d_name, "..") != 0) {
      
      continue;
    }

    fd_list->fd_array =(fd_info_t **)  realloc(fd_list->fd_array, ((fd_list->count + 5) * sizeof(fd_info_t *)));
    fd_info_t * fd_information = (fd_info_t *) malloc(sizeof(fd_info_t));

    char fd_path[1024];
    sprintf(fd_path, "/proc/%d/fd/%s", pid, dir_struc->d_name);
    char link_path[1024];
    int link_len = readlink(fd_path, link_path, sizeof(link_path));
    link_path[link_len] = '\0';

    fd_information->fd = atoi(dir_struc->d_name);
    sprintf(fd_information->object, "%s", link_path);
    
    struct stat sb;
    int ret_val = lstat(link_path, &sb);
    switch (sb.st_mode & S_IFMT) {
      case S_IFDIR:  
        sprintf(fd_information->type, "%s", "directory");
        break;
      case S_IFIFO:
        sprintf(fd_information->type, "%s", "pipe");
        break;
      case S_IFLNK:
        sprintf(fd_information->type, "%s", "symlink");
        break;
      case S_IFREG:
        sprintf(fd_information->type, "%s", "file");
        break;
      case S_IFSOCK:
        sprintf(fd_information->type, "%s", "socket");
        break;
      default:
        sprintf(fd_information->type, "%s", "file");
        break;
    }

    fd_list->fd_array[fd_list->count] = fd_information;
    fd_list->count++;
  }

  return fd_list;

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
  fd_list_t * fd_list = open_file_list(1047644);
  //procinfo_t * self_info = (procinfo_t *) malloc(sizeof(procinfo_t));
  mem_map_info_t * mem_map = (mem_map_info_t *) malloc(sizeof(mem_map_info_t));

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
    //printf("%s %s %s %s %s %s %s\n", mount->device, mount->directory, mount->type, mount->total, mount->available, mount->used, mount->percent);
    count++;
  }

  int * pids = get_pid();
  proc_list_t * proc_list = list_view(pids);

  mem_map_list_t * mmap_list = create_mem_info(1047644);
  /*printf("%d\n", mmap_list->count);
  for (int i = 0; i < mmap_list->count; i++) {
    printf("vm_start %s\n", mmap_list->maps_list[i]->vm_start);
  }*/
 /*
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
 */

}
