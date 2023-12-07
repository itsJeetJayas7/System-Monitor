#include <stdio.h>    // Standard Input/Output functions
#include <stdlib.h>   // General Utilities Library   // Variable arguments
#include <string.h>   // String handling functions
#include <math.h>     // Mathematical functions
#include <ctype.h>    // Character handling functions
#include <signal.h>   // Signal handling
#include <unistd.h>   // POSIX operating system API
#include <gtk/gtk.h>
#include <sys/types.h>
//#include <sys/sysctl.h>
#include <sys/mount.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <gtk/gtkx.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include "process_info.h"


#define MAX_LINE_LENGTH 1000
#define MAX_ENTRIES 1000

GtkTreeStore * treeStore;
GtkTreeView * treeView;
GtkTreeViewColumn * c1;
GtkTreeViewColumn * c2;
GtkTreeViewColumn * c3;
GtkTreeViewColumn * c4;
GtkTreeViewColumn * c5;
GtkCellRenderer * cr1;
GtkCellRenderer * cr2;
GtkCellRenderer * cr3;
GtkCellRenderer * cr4;
GtkCellRenderer * cr5;
GtkTreeSelection * selection;

int create_proc_info (procinfo_t * self_info, int path) {
  //open stat file path
  char stat_path[1024];
  sprintf(stat_path,"/proc/%d/stat", path);
  //printf("%s\n", stat_path);
  FILE * stat_file;
  if ((stat_file = fopen(stat_path, "r"))) {
    fscanf(stat_file, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %lu %llu %lu %ld", 
      &self_info->pid, self_info->process_name, &self_info->state, &self_info->ppid, &self_info->pgrp, &self_info->sid, 
      &self_info->tty_nr, &self_info->tty_pgrp, &self_info->flags, &self_info->min_flt, &self_info->cmin_flt, 
      &self_info->maj_flt, &self_info->cmaj_flt, &self_info->utime, &self_info->stime, &self_info->cutime, 
      &self_info->cstime, &self_info->prio, &self_info->nice, &self_info->num_threads, &self_info->itrealvalue, 
      &self_info->starttime, &self_info->vsize, &self_info->rss);
      fclose(stat_file);

      FILE * uptime_file = fopen("/proc/uptime", "r");
      double uptime_sec;
      double utime_sec = self_info->utime / 100;
      double stime_sec = self_info->stime / 100;
      double start_time_sec = self_info->starttime / 100;
      fscanf(uptime_file, "%lf ", &uptime_sec);
      fclose(uptime_file);
      double proc_elapse_sec = uptime_sec - start_time_sec;
      double proc_usage_sec = utime_sec + stime_sec;
      double proc_usage_percent = proc_usage_sec  * 100 / proc_elapse_sec;
      self_info->cpu_percent = proc_usage_percent;
  } else {
     return 0;
  }

  //scan relevent information

  //print_stat(self_info);
  //printf("%s\n", self_info->process_name);
  //open statm file path
  char statm_path[1024];
  sprintf(statm_path,"/proc/%d/statm", path);
  FILE * statm_file;
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
    format_single_process_info(info);
  }
  return proc_list;
}

fd_list_t * open_file_list(int pid) {
  
  fd_list_t * fd_list = (fd_list_t *) malloc(sizeof(fd_list_t));

  char fd_dir_path[1024];
  sprintf(fd_dir_path,"/proc/%d/fd", pid);

  DIR * fd_dir = opendir(fd_dir_path);
  if (fd_dir == NULL) {
    g_print("IT is NULL Actually!!");
    fd_list->count = 1;
    fd_list->fd_array =(fd_info_t **) malloc(((fd_list->count + 5) * sizeof(fd_info_t *)));
    fd_info_t * fd_information = (fd_info_t *) malloc(sizeof(fd_info_t));

    strcpy(fd_information->type, "Permission Denied");
    strcpy(fd_information->object, "Permission Denied");
    strcpy(fd_information->fd_str, "-1");
    fd_list->fd_array[0] = fd_information;
    return fd_list;
  }
  struct dirent * dir_struc;

  fd_list->count = 0;
  fd_list->fd_array =(fd_info_t **) malloc(((fd_list->count + 5) * sizeof(fd_info_t *)));

  while((dir_struc = readdir(fd_dir)) != NULL) {

    if (dir_struc == NULL && errno != 0) {
        perror("readdir");
        continue;  // Exit the loop on error
    }
    
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

    strcpy(fd_information->fd_str, dir_struc->d_name);


    fd_information->fd = atoi(dir_struc->d_name);
    sprintf(fd_information->object, "%s", link_path);
    
    switch (link_path[0]) {
      case 'p':
        sprintf(fd_information->type, "%s", "pipe");
        break;
      case '/':
        sprintf(fd_information->type, "%s", "file");
        break;
      case 's':
        sprintf(fd_information->type, "%s", "socket");
        break;
      case 'a':
        sprintf(fd_information->type, "%s", "inode");
        break;
      default:
        sprintf(fd_information->type, "%s", "other");
        break;
    }

    fd_list->fd_array[fd_list->count] = fd_information;
    fd_list->count++;
  }

  closedir(fd_dir);

  return fd_list;

}

void create_empty_map(mem_map_info_t * map_info) {
  strcpy(map_info->file_name, "None");
  strcpy(map_info->VM, "None-PremDenied");
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

void format_single_process_info(procinfo_t * info) {
  sprintf(info->pid_str, "%d", info->pid);
  sprintf(info->cpu_perc_str, "%.2f", info->cpu_percent);
  double rss_mb = (info->rss * 4) / 1024.0;
  sprintf(info->phys_mem_str, "%.1f", rss_mb);
  double virt_mem_mb = (info->program_size * 4) / 1024.0;
  sprintf(info->virtual_mem_str, "%.1f", virt_mem_mb);

  double res_mem_mb = (info->resident_size * 4) / 1024.0;
  sprintf(info->resident_mem_str, "%.1f", res_mem_mb);

  double shared_mem_mb = (info->shared_mem * 4) / 1024.0;
  sprintf(info->shared_mem_str, "%.1f", shared_mem_mb);

  double cpu_time = info->utime + info->stime;
  cpu_time = cpu_time / 100;

  int hr = cpu_time / (60 * 60);
  int min = ((cpu_time - (hr * 60 * 60)) / 60);

  sprintf(info->cpu_time_str, "%d:%d", hr, min);
  sprintf(info->nice_str, "%ld", info->nice);
  sprintf(info->prio_str, "%ld", info->prio);

  

  if (info->state == 'R') {
    strcpy(info->state_str, "Running");
  }
  else if (info->state == 'S') {
     strcpy(info->state_str, "Sleeping");
  }
  else if (info->state == 'Z') {
     strcpy(info->state_str, "Zombie");
  }
  else if (info->state == 't') {
     strcpy(info->state_str, "Trace Pause");
  }
  else if (info->state == 'X') {
     strcpy(info->state_str, "Dead");
  }
  else if (info->state == 'I') {
     strcpy(info->state_str, "Idle");
  }
  else {
    sprintf(info->state_str, "%c", info->state);
  }
  
  

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
        fclose(smap_file);
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

      
      for (int i = 0; i < 10; ++i) {
        if (fgets(line_ph, sizeof(line_ph), smap_file) != NULL) {
          continue;
        }
        else {
          mem_map_flag = 1;
          
        }
      }
      format_mem_map(mem_map);
      
      smap_list->count++;
    }
    
  }
  else {
    
    mem_map_list_t * smap_list = (mem_map_list_t *) malloc(sizeof(mem_map_list_t));
    smap_list->count = 0;
    g_print("THE COUNT IS FUCIKN ZERO!!!!\n\n\n\n");
    smap_list->maps_list =( mem_map_info_t **) malloc(((smap_list->count + 10) * sizeof(mem_map_info_t *)));
    mem_map_info_t * mem_map = (mem_map_info_t *) malloc(sizeof(mem_map_info_t));
    create_empty_map(mem_map);

    smap_list->maps_list = (mem_map_info_t **) realloc(smap_list->maps_list, ((smap_list->count + 10) * sizeof(mem_map_info_t *)));
    smap_list->maps_list[smap_list->count] = mem_map;
    format_mem_map(mem_map);
    return smap_list;
    return NULL;
  }
  
  
   
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

GtkWidget * window;
GtkWidget * button1;
GtkWidget * label_1;
GtkWidget * grid1;
GtkBuilder *builder;
GtkWidget * window2;
GtkBuilder *builder_2;

GtkWidget * label_osName;
GtkWidget * label_version;
GtkWidget * label_kernel;
GtkWidget * label_memory;
GtkWidget * label_processor;
GtkWidget * label_disk;

char osName[256];
char osVersion[256];
char kernelVersion[256];
char memorySize[256];
char cpuType[256];
char diskInfo[256];

void getSystemInfo() {
    // Get OS name using sw_vers command 
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        snprintf(osName, sizeof(osName), "%s", unameData.sysname);
        snprintf(osVersion, sizeof(osVersion), "%s %s", unameData.release, unameData.version);
    } else {
        printf("Failed to retrieve OS information\n");
    }

    // Get kernel version
    char kvers[256];
    FILE *kernelFile = fopen("/proc/version", "r");
    if (kernelFile != NULL) {
        if (fgets(kvers, sizeof(kernelVersion), kernelFile) != NULL) {
            // Remove newline character from the end
            kvers[strcspn(kvers, "\n")] = '\0';
        }
        fclose(kernelFile);
    } else {
        printf("Failed to retrieve kernel version\n");
    }
    printf("\n\n|%s|\n\n", kvers);
    char * temp2 = "Kernel Version: ";
    kvers[strcspn(kvers, "(")] = '\0';
    strcat(kernelVersion, temp2);
    strcat(kernelVersion, kvers);

    // Get amount of memory
    struct sysinfo sysInfo;
    if (sysinfo(&sysInfo) == 0) {
        snprintf(memorySize, sizeof(memorySize), "Memory Size: %.2f GiB", (double)sysInfo.totalram / (1 << 30));
    } else {
        printf("Failed to retrieve memory information\n");
    }


    // Get processor version
    FILE *cpuInfoFile = fopen("/proc/cpuinfo", "r");
    if (cpuInfoFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), cpuInfoFile) != NULL) {
            if (strstr(line, "model name") != NULL) {
                char *pos = strchr(line, ':');
                if (pos != NULL) {
                    snprintf(cpuType, sizeof(cpuType), "Processor Version: %s", pos + 2);
                    break;
                }
            }
        }
        fclose(cpuInfoFile);
    } else {
        printf("Failed to retrieve CPU information\n");
    }

    
    //char * temp = "Processor Version: ";
    //strcat(cpuType, temp);
    //strcat(cpuType, cput);

    // Get disk storage
    struct statvfs diskStats;
    if (statvfs("/", &diskStats) == 0) {
        uint64_t totalDiskSpace = (uint64_t)diskStats.f_blocks * diskStats.f_frsize;
        snprintf(diskInfo, sizeof(diskInfo), "Total Disk Space: %.2f GiB", (double)totalDiskSpace / (1 << 30));
    } else {
        printf("Failed to retrieve disk information\n");
    }
}


void on_window_temp_destroy(GtkWidget *widget, gpointer user_data) {
    // Disconnect the "destroy" signal to avoid closing the main window
    g_signal_handlers_disconnect_by_func(widget, G_CALLBACK(on_window_temp_destroy), NULL);

    // Your cleanup code, if any

    // Destroy the window explicitly if needed
    gtk_widget_destroy(widget);
}

void on_button1_clicked(GtkButton * button) {
    //gtk_label_set_text(GTK_LABEL(label_1), (const gchar *) "Button Works!");
    GtkWidget * windowTemp;
    windowTemp = GTK_WIDGET(gtk_builder_get_object(builder, "open_files_window"));
    //g_signal_connect(windowTemp, "destroy", G_CALLBACK(on_window_temp_destroy), NULL);
    gtk_widget_show_all(windowTemp);
    printf("\nTried to open a new Window\n");
}

int pidIndex;
int ind;

char* parse_ps_line(char* line, int* level, int* pid) {
    
    char * temp = strstr(line, "\\_");
    int lev = 0;
    if (temp != NULL) {
    int slash = temp-line;
    int val = slash - ind;
    int i = 2;
    lev = 1;
    while(i != val) {
      i+=4;
      lev++;
    }
    } else {
	    lev = 0;
    }
    *level = lev;
    char* token = strtok(line, " ");
    int count = 0;
    while (token != NULL) {
        if (count == 1) {
            *pid = atoi(token);
        }
        token = strtok(NULL, " ");
        count++;
    }
    return line; // Return the original line for further processing if needed
}

int indexT;

char ** fauxData() {
    FILE* fp = popen("ps faux", "r");
    if (fp == NULL) {
        perror("Error opening pipe");
        return EXIT_FAILURE;
    }
    char line[MAX_LINE_LENGTH];
    char** result = (char **) malloc(7000 * sizeof(char*));
    indexT = 0;
    fgets(line, sizeof(line), fp);
    printf("%s", line);
    char * result2 = strstr(line, "PID");
    pidIndex = result2 - line;
    printf("int pid: %d", pidIndex);
    result2 = strstr(line, "COMMAND");
    ind = result2 - line;
    ind = ind - 1;
    printf("start ind: %d", ind);
    printf(" |%c %c| \n", line[pidIndex], line[ind+1]);
    while (fgets(line, sizeof(line), fp) != NULL && indexT < MAX_ENTRIES) {
        int level, pid;
        parse_ps_line(line, &level, &pid);
	char entry[MAX_LINE_LENGTH];
        sprintf(entry, "level %d PID %d", level, pid);
	result[indexT] = strdup(entry);
        indexT++;
    }
    
    pclose(fp);
    
    return result;

}



void on_kill_pro_clicked() {
    g_print("want to kill a process\n");
    GtkWidget * label_proc = GTK_WIDGET(gtk_builder_get_object(builder_2, "proc_labe"));
    const gchar *text = gtk_label_get_text(GTK_LABEL(label_proc));
    int pid_priv;
    sscanf(text, "%d", &pid_priv);
    g_print("The PID received: %d\n", pid_priv);
    if (kill(pid_priv, SIGTERM) == 0) {
        printf("Process with PID %d killed successfully.\n", pid_priv);
    } else {
        perror("Error killing process");
    }

    
}

void on_cont_proc_clicked() {
    g_print("want to continue a process\n");
    GtkWidget * label_proc = GTK_WIDGET(gtk_builder_get_object(builder_2, "proc_labe"));
    const gchar *text = gtk_label_get_text(GTK_LABEL(label_proc));
    int pid_priv;
    sscanf(text, "%d", &pid_priv);
    g_print("The PID received: %d\n", pid_priv);
    if (kill(pid_priv, SIGCONT) == 0) {
        printf("Process with PID %d continued.\n", pid_priv);
    } else {
        perror("Error continuing the process");
        return EXIT_FAILURE;
    }
}

void get_active_process_table() {
  GtkTreeStore *treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder, "tree_store"));
  GtkTreeView *treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_mai"));
  gtk_tree_store_clear(treeStore_mm);

  // setTable(1, NULL);

  GtkTreeViewColumn *cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col1"));
  GtkTreeViewColumn *cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col2"));
  GtkTreeViewColumn *cm3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col3"));
  GtkTreeViewColumn *cm4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col4"));
  GtkTreeViewColumn *cm5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col5"));

  GtkTreeIter iter;

  int *pids = get_pid();
  proc_list_t *proc_list = list_view(pids);

  for (int j = 0; j < proc_list->count; j++) {
    if (proc_list->info_list[j] == NULL) {
      continue;
    }
    if (proc_list->info_list[j]->state == 'R' || proc_list->info_list[j]->state == 'S') {
      char rss[20];
      double rssInMiB = (double)proc_list->info_list[j]->rss / (1024 * 1024);
      sprintf(rss, "%.2f MiB", rssInMiB);
      gtk_tree_store_append(treeStore_mm, &iter, NULL);
      gtk_tree_store_set(treeStore_mm, &iter, 0, proc_list->info_list[j]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 1, proc_list->info_list[j]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 2, proc_list->info_list[j]->cpu_perc_str, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 3, proc_list->info_list[j]->pid_str, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 4, rss, -1);
    }
  }    
  
  gtk_tree_view_set_model(treeView_mm, GTK_TREE_MODEL(treeStore_mm));
}

int pid_user_array[10000];
int pid_user_len;

void get_process_usernames() {
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Error opening /proc directory");
        exit(EXIT_FAILURE);
    }
    char * username = getenv("USER");
    struct dirent *entry;
    int i = 0;
    while ((entry = readdir(proc_dir)) != NULL) {
        // Check if the entry is a directory and represents a process
        if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);

            int fd = open(path, O_RDONLY);
            if (fd == -1) {
                perror("Error opening process status file");
                continue;
            }

            char buffer[1024];
            ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
            if (bytesRead > 0) {
                // Search for the "Uid:" line to extract the user ID
                char *uidPos = strstr(buffer, "Uid:");
                if (uidPos != NULL) {
                    uid_t uid;
                    sscanf(uidPos, "Uid:\t%d", &uid);

                    // Get the username from the user ID
                    struct passwd *user_info = getpwuid(uid);
                    if (user_info != NULL) {
                        if (username == NULL) {
                            printf("Lmao");
                        }
                         if(!strcmp(username, user_info->pw_name)) {
                        pid_user_array[i++] = atoi(entry->d_name);

                        //printf("Process %s is run by user: %s\n", entry->d_name, user_info->pw_name);
                         }
                        
                    } else {
                        //perror("Error getting user info");
                    }
                }
            }

            close(fd);
        }
    }
    pid_user_len = i;

    closedir(proc_dir);
}

void filter_table(int form) {
  GtkTreeStore *treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder, "tree_store"));
  GtkTreeView *treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_mai"));
  gtk_tree_store_clear(treeStore_mm);

  // setTable(1, NULL);

  GtkTreeViewColumn *cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col1"));
  GtkTreeViewColumn *cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col2"));
  GtkTreeViewColumn *cm3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col3"));
  GtkTreeViewColumn *cm4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col4"));
  GtkTreeViewColumn *cm5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col5"));

  GtkTreeIter iter;
  
  get_process_usernames();

  int *pids = get_pid();
  proc_list_t *proc_list = list_view(pids);

  for (int i = 0; i < pid_user_len; i++) {
    int index_pid;

    for (int j = 0; j < proc_list->count; j++) {
      if (proc_list->info_list[j] == NULL) {
        continue;
      }
      if (proc_list->info_list[j]->pid == pid_user_array[i]) {
        index_pid = j;
        break;
      }
    }
    char rss[20];
    double rssInMiB = (double)proc_list->info_list[index_pid]->rss / (1024 * 1024);
    sprintf(rss, "%.2f MiB", rssInMiB);
    gtk_tree_store_append(treeStore_mm, &iter, NULL);
    gtk_tree_store_set(treeStore_mm, &iter, 0, proc_list->info_list[index_pid]->process_name, -1);
    gtk_tree_store_set(treeStore_mm, &iter, 1, proc_list->info_list[index_pid]->state_str, -1);
    gtk_tree_store_set(treeStore_mm, &iter, 2, proc_list->info_list[index_pid]->cpu_perc_str, -1);
    gtk_tree_store_set(treeStore_mm, &iter, 3, proc_list->info_list[index_pid]->pid_str, -1);
    gtk_tree_store_set(treeStore_mm, &iter, 4, rss, -1);

  }
  gtk_tree_view_set_model(treeView_mm, GTK_TREE_MODEL(treeStore_mm));

}

void on_stop_pro_clicked() {

    g_print("want to stop a process\n");
    GtkWidget * label_proc = GTK_WIDGET(gtk_builder_get_object(builder_2, "proc_labe"));
    const gchar *text = gtk_label_get_text(GTK_LABEL(label_proc));
    int pid_priv;
    sscanf(text, "%d", &pid_priv);
    g_print("The PID received: %d\n", pid_priv);
    if (kill(pid_priv, SIGSTOP) == 0) {
        printf("Process with PID %d stopped.\n", pid_priv);
    } else {
        perror("Error stopping the process");
        return EXIT_FAILURE;
    }
}

void on_combo_box_changed(GtkComboBox * c) {
  printf("combo box chaged\n\n");
}

void on_combo_vals_changed(GtkEntry * e) {
  char tmp[128];
  sprintf(tmp, "%s", gtk_entry_get_text(e));
  g_print("GOT THE ENTRY: |%s|\n\n", tmp);
  
  if(!strcmp(tmp, "Active Processes")) {
    get_active_process_table();
  } else if (!strcmp(tmp, "My Processes")) {
    printf("User chose to see his own process");
    filter_table(1);
  } else {
    on_refresh_clicked();
  }

}


void make_detail_table(int pid_priv) {
    GtkTreeStore * treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder_2, "tree_store3"));
    GtkTreeView * treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder_2, "tree_detail"));
    GtkTreeViewColumn * cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "col1"));
    GtkTreeViewColumn * cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "col2"));

    GtkCellRenderer * cmr1 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr2 = gtk_cell_renderer_text_new();

    GtkTreeSelection * selection_mm = GTK_TREE_SELECTION(gtk_builder_get_object(builder_2, "select_detail"));

    gtk_tree_view_column_pack_start(cm1, cmr1, TRUE);
	gtk_tree_view_column_pack_start(cm2, cmr2, TRUE);

    gtk_tree_view_column_add_attribute(cm1, cmr1, "text", 0);
	gtk_tree_view_column_add_attribute(cm2, cmr2, "text", 1);

    GtkTreeIter iter;


  procinfo_t * self_info = (procinfo_t *) malloc(sizeof(procinfo_t));
  create_single_process_info(self_info);
  create_proc_info(self_info, pid_priv);
  format_single_process_info(self_info);
  
  // gtk_tree_store_append(treeStore_mm, &iter, NULL);
	// gtk_tree_store_set(treeStore_mm, &iter, 0, "Process Name", -1);
	// gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->process_name, -1);
  
  gtk_tree_view_column_set_title(cm2, self_info->process_name);

  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "User", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, "USer ID", -1);
  
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Status", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->state_str, -1);
  
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Memory (MB)", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->phys_mem_str, -1);
  
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Virtual Memory (MB)", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->virtual_mem_str, -1);
  
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Resident Memory (MB)", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->resident_mem_str, -1);

  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Shared Memory (MB)", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->shared_mem_str, -1);
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "CPU %", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->cpu_perc_str, -1);
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "CPU Time", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->cpu_time_str, -1);
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Nice", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->nice_str, -1);
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "Priority", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->prio_str, -1);
  gtk_tree_store_append(treeStore_mm, &iter, NULL);
	gtk_tree_store_set(treeStore_mm, &iter, 0, "PID", -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, self_info->pid_str, -1);

    selection_mm = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView_mm));
}

void make_op_table(int pid_priv) {
    GtkTreeStore * treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder_2, "tree_store2"));
    GtkTreeView * treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder_2, "tree_open"));
    GtkTreeViewColumn * cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cr1"));
    GtkTreeViewColumn * cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cr2"));
    GtkTreeViewColumn * cm3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cr3"));

    GtkCellRenderer * cmr1 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr2 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr3 = gtk_cell_renderer_text_new();

    GtkTreeSelection * selection_mm = GTK_TREE_SELECTION(gtk_builder_get_object(builder_2, "select_op"));

    gtk_tree_view_column_pack_start(cm1, cmr1, TRUE);
	gtk_tree_view_column_pack_start(cm2, cmr2, TRUE);
    gtk_tree_view_column_pack_start(cm3, cmr3, TRUE);

    gtk_tree_view_column_add_attribute(cm1, cmr1, "text", 0);
	gtk_tree_view_column_add_attribute(cm2, cmr2, "text", 1);
    gtk_tree_view_column_add_attribute(cm3, cmr3, "text", 2);

    GtkTreeIter iter;


    fd_list_t * fd_list = open_file_list(pid_priv);

    for (int i = 0; i < fd_list->count; i++) {

        gtk_tree_store_append(treeStore_mm, &iter, NULL);
	    gtk_tree_store_set(treeStore_mm, &iter, 0, fd_list->fd_array[i]->fd_str, -1);
	    gtk_tree_store_set(treeStore_mm, &iter, 1, fd_list->fd_array[i]->type, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 2, fd_list->fd_array[i]->object, -1);

    }

    selection_mm = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView_mm));
}

void make_mmaps_table(int pid_priv) {
    printf("Hellaaaao");
    GtkTreeStore * treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder_2, "tree_store1"));
    GtkTreeView * treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder_2, "tree_mm"));
    GtkTreeViewColumn * cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cmr1"));
    GtkTreeViewColumn * cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cmr2"));
    GtkTreeViewColumn * cm3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cmr3"));
    GtkTreeViewColumn * cm4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cmr4"));
    GtkTreeViewColumn * cm5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cmr5"));
    GtkTreeViewColumn * cm6 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cmr6"));
    GtkTreeViewColumn * cm7 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cm7"));
    GtkTreeViewColumn * cm8 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cm8"));
    GtkTreeViewColumn * cm9 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cm9"));
    GtkTreeViewColumn * cm10 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder_2, "cm10"));
    


    GtkCellRenderer * cmr1 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr2 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr3 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr4 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr5 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr6 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr7 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr8 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr9 = gtk_cell_renderer_text_new();
    GtkCellRenderer * cmr10 = gtk_cell_renderer_text_new();
    
    GtkTreeSelection * selection_mm = GTK_TREE_SELECTION(gtk_builder_get_object(builder_2, "select_mm"));

    if (cm1 != NULL) {
    g_print("cm1 is not NULL\n");
    // Do something with cm1
    }
    gtk_tree_view_column_pack_start(cm1, cmr1, TRUE);
	gtk_tree_view_column_pack_start(cm2, cmr2, TRUE);
    gtk_tree_view_column_pack_start(cm3, cmr3, TRUE);
    gtk_tree_view_column_pack_start(cm4, cmr4, TRUE);
    gtk_tree_view_column_pack_start(cm5, cmr5, TRUE);
	gtk_tree_view_column_pack_start(cm6, cmr6, TRUE);
	gtk_tree_view_column_pack_start(cm7, cmr7, TRUE);
    gtk_tree_view_column_pack_start(cm8, cmr8, TRUE);
    gtk_tree_view_column_pack_start(cm9, cmr9, TRUE);
    gtk_tree_view_column_pack_start(cm10, cmr10, TRUE);
	
	gtk_tree_view_column_add_attribute(cm1, cmr1, "text", 0);
	gtk_tree_view_column_add_attribute(cm2, cmr2, "text", 1);
    gtk_tree_view_column_add_attribute(cm3, cmr3, "text", 2);
    gtk_tree_view_column_add_attribute(cm4, cmr4, "text", 3);
    gtk_tree_view_column_add_attribute(cm5, cmr5, "text", 4);
    gtk_tree_view_column_add_attribute(cm6, cmr6, "text", 5);
	gtk_tree_view_column_add_attribute(cm7, cmr7, "text", 6);
    gtk_tree_view_column_add_attribute(cm8, cmr8, "text", 7);
    gtk_tree_view_column_add_attribute(cm9, cmr9, "text", 8);
    gtk_tree_view_column_add_attribute(cm10, cmr10, "text", 9);

    GtkTreeIter iter;
    
    mem_map_list_t * mmap_list = create_mem_info(pid_priv);
    
    if (mmap_list->count == 0) {
      gtk_tree_store_append(treeStore_mm, &iter, NULL);
      gtk_tree_store_set(treeStore_mm, &iter, 0, "Permission", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 1, "Denied", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 2, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 3, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 4, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 5, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 6, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 7, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 8, "None", -1);
      gtk_tree_store_set(treeStore_mm, &iter, 9, "None", -1);
    }

    for (int i = 0; i < mmap_list->count; i++) {
        gtk_tree_store_append(treeStore_mm, &iter, NULL);
        if(mmap_list->maps_list[i] == NULL) {
            g_print("WTF!=======\n");
        }
        g_print("==%s==\n", mmap_list->maps_list[i]->file_name);
        gtk_tree_store_set(treeStore_mm, &iter, 0, mmap_list->maps_list[i]->file_name, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 1, mmap_list->maps_list[i]->vm_start, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 2, mmap_list->maps_list[i]->vm_end, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 3, mmap_list->maps_list[i]->size_str, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 4, mmap_list->maps_list[i]->flags, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 5, mmap_list->maps_list[i]->offset, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 6, mmap_list->maps_list[i]->private_clean_str, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 7, mmap_list->maps_list[i]->private_dirt_str, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 8, mmap_list->maps_list[i]->shared_clean_str, -1);
        gtk_tree_store_set(treeStore_mm, &iter, 9, mmap_list->maps_list[i]->shared_dirty_str, -1);
    }

    selection_mm = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView_mm));
}

void set_fs_table() {
  GtkTreeStore * treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder, "fs_treeStore"));
  GtkTreeView * treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder, "fs_tree"));
  GtkTreeViewColumn * cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c1"));
  GtkTreeViewColumn * cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c2"));
  GtkTreeViewColumn * cm3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c3"));
  GtkTreeViewColumn * cm4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c4"));
  GtkTreeViewColumn * cm5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c5"));
  GtkTreeViewColumn * cm6 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c6"));
  GtkTreeViewColumn * cm7 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "c7"));
  
  


  GtkCellRenderer * cmr1 = gtk_cell_renderer_text_new();
  GtkCellRenderer * cmr2 = gtk_cell_renderer_text_new();
  GtkCellRenderer * cmr3 = gtk_cell_renderer_text_new();
  GtkCellRenderer * cmr4 = gtk_cell_renderer_text_new();
  GtkCellRenderer * cmr5 = gtk_cell_renderer_text_new();
  GtkCellRenderer * cmr6 = gtk_cell_renderer_text_new();
  GtkCellRenderer * cmr7 = gtk_cell_renderer_text_new();

  gtk_tree_view_column_pack_start(cm1, cmr1, TRUE);
	gtk_tree_view_column_pack_start(cm2, cmr2, TRUE);
    gtk_tree_view_column_pack_start(cm3, cmr3, TRUE);
    gtk_tree_view_column_pack_start(cm4, cmr4, TRUE);
    gtk_tree_view_column_pack_start(cm5, cmr5, TRUE);
	gtk_tree_view_column_pack_start(cm6, cmr6, TRUE);
	gtk_tree_view_column_pack_start(cm7, cmr7, TRUE);

  gtk_tree_view_column_add_attribute(cm1, cmr1, "text", 0);
	gtk_tree_view_column_add_attribute(cm2, cmr2, "text", 1);
    gtk_tree_view_column_add_attribute(cm3, cmr3, "text", 2);
    gtk_tree_view_column_add_attribute(cm4, cmr4, "text", 3);
    gtk_tree_view_column_add_attribute(cm5, cmr5, "text", 4);
    gtk_tree_view_column_add_attribute(cm6, cmr6, "text", 5);
	gtk_tree_view_column_add_attribute(cm7, cmr7, "text", 6);
  
  
  GtkTreeSelection * selection_mm = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "fs_select"));

  GtkTreeIter iter;

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
    gtk_tree_store_append(treeStore_mm, &iter, NULL);
    gtk_tree_store_set(treeStore_mm, &iter, 0, mount->device, -1);
	gtk_tree_store_set(treeStore_mm, &iter, 1, mount->directory, -1);
  gtk_tree_store_set(treeStore_mm, &iter, 2, mount->type, -1);
  gtk_tree_store_set(treeStore_mm, &iter, 3, mount->total, -1);
  int total_t;
  char gig;
  sscanf(mount->total, "%d%c", &total_t, &gig);
  int used_t;
  sscanf(mount->used, "%d%*c", &used_t);
  int free_t = total_t - used_t;
  char free_mem[20];
  sprintf(free_mem, "%d%c", free_t, gig);
  gtk_tree_store_set(treeStore_mm, &iter, 4, free_mem, -1);
  gtk_tree_store_set(treeStore_mm, &iter, 5, mount->available, -1);
  gtk_tree_store_set(treeStore_mm, &iter, 6, mount->used, -1);

    //printf("%s %s %s %s %s %s %s\n", mount->device, mount->directory, mount->type, mount->total, mount->available, mount->used, mount->percent);
    count++;
  }

  


  selection_mm = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView_mm));

}

void on_refresh_clicked() {
  GtkTreeStore *treeStore_mm = GTK_TREE_STORE(gtk_builder_get_object(builder, "tree_store"));
  GtkTreeView *treeView_mm = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_mai"));
  gtk_tree_store_clear(treeStore_mm);

  // setTable(1, NULL);

  GtkTreeViewColumn *cm1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col1"));
  GtkTreeViewColumn *cm2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col2"));
  GtkTreeViewColumn *cm3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col3"));
  GtkTreeViewColumn *cm4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col4"));
  GtkTreeViewColumn *cm5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col5"));

  GtkTreeIter iter;
  GtkTreeIter iterChild1;
  GtkTreeIter iterChild2;
  GtkTreeIter iterChild3;
  GtkTreeIter iterChild4;
  GtkTreeIter iterChild5;
  GtkTreeIter iterChild6;
  GtkTreeIter iterChild7;
  GtkTreeIter iterChild8;

  printf("Hello World!");
  char **result = fauxData();
  char *data = "To be Added";
  int *pids = get_pid();
  proc_list_t *proc_list = list_view(pids);

  for (int i = 0; i < indexT; i++) {
    int level;
    int pid_t;
    int check = sscanf(result[i], "level %d PID %d", &level, &pid_t);
    int index_pid;

    for (int j = 0; j < proc_list->count; j++) {
      if (proc_list->info_list[j] == NULL) {
        continue;
      }
      if (proc_list->info_list[j]->pid == pid_t) {
        index_pid = j;
        break;
      }
    }

    char state_t[2];
    state_t[0] = proc_list->info_list[index_pid]->state;
    state_t[1] = '\0';
    char cpu_t[7];
    sprintf(cpu_t, "%.2f", proc_list->info_list[index_pid]->cpu_percent);
    char pid_tt[10];
    sprintf(pid_tt, "%d", proc_list->info_list[index_pid]->pid);
    char rss[20];
    double rssInMiB = (double)proc_list->info_list[index_pid]->rss / (1024 * 1024);
    sprintf(rss, "%.2f MiB", rssInMiB);

    if (level == 0) {
      gtk_tree_store_append(treeStore_mm, &iter, NULL);
      gtk_tree_store_set(treeStore_mm, &iter, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iter, 4, rss, -1);

    } else if (level == 1) {
      gtk_tree_store_append(treeStore_mm, &iterChild1, &iter);
      gtk_tree_store_set(treeStore_mm, &iterChild1, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild1, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild1, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild1, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild1, 4, rss, -1);

    } else if (level == 2) {
      gtk_tree_store_append(treeStore_mm, &iterChild2, &iterChild1);
      gtk_tree_store_set(treeStore_mm, &iterChild2, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild2, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild2, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild2, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild2, 4, rss, -1);

    } else if (level == 3) {
      gtk_tree_store_append(treeStore_mm, &iterChild3, &iterChild2);
      gtk_tree_store_set(treeStore_mm, &iterChild3, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild3, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild3, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild3, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild3, 4, rss, -1);

    } else if (level == 4) {
      gtk_tree_store_append(treeStore_mm, &iterChild4, &iterChild3);
      gtk_tree_store_set(treeStore_mm, &iterChild4, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild4, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild4, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild4, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild4, 4, rss, -1);

    } else if (level == 5) {
      gtk_tree_store_append(treeStore_mm, &iterChild5, &iterChild4);
      gtk_tree_store_set(treeStore_mm, &iterChild5, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild5, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild5, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild5, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild5, 4, rss, -1);

    } else if (level == 6) {
      gtk_tree_store_append(treeStore_mm, &iterChild6, &iterChild5);
      gtk_tree_store_set(treeStore_mm, &iterChild6, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild6, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild6, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild6, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild6, 4, rss, -1);

    } else if (level == 7) {
      gtk_tree_store_append(treeStore_mm, &iterChild7, &iterChild6);
      gtk_tree_store_set(treeStore_mm, &iterChild7, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild7, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild7, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild7, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild7, 4, rss, -1);

    } else if (level == 8) {
      gtk_tree_store_append(treeStore_mm, &iterChild8, &iterChild7);
      gtk_tree_store_set(treeStore_mm, &iterChild8, 0, proc_list->info_list[index_pid]->process_name, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild8, 1, proc_list->info_list[index_pid]->state_str, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild8, 2, cpu_t, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild8, 3, pid_tt, -1);
      gtk_tree_store_set(treeStore_mm, &iterChild8, 4, rss, -1);
    }
  }

  printf("New was loaded alright!\n");

  gtk_tree_view_set_model(treeView_mm, GTK_TREE_MODEL(treeStore_mm));
}


void setTable(int number, char * type) {
	treeStore = GTK_TREE_STORE(gtk_builder_get_object(builder, "tree_store"));
	treeView = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_mai"));
	c1 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col1"));
	c2 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col2"));
    c3 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col3"));
    c4 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col4"));
    c5 = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "col5"));
	selection = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "select_main"));
	
  if(number == 0) {
	cr1 = gtk_cell_renderer_text_new();
	cr2 = gtk_cell_renderer_text_new();
    cr3 = gtk_cell_renderer_text_new();
    cr4 = gtk_cell_renderer_text_new();
    cr5 = gtk_cell_renderer_text_new();
  } 
	
	gtk_tree_view_column_pack_start(c1, cr1, TRUE);
	gtk_tree_view_column_pack_start(c2, cr2, TRUE);
    gtk_tree_view_column_pack_start(c3, cr3, TRUE);
    gtk_tree_view_column_pack_start(c4, cr4, TRUE);
    gtk_tree_view_column_pack_start(c5, cr5, TRUE);
	
	gtk_tree_view_column_add_attribute(c1, cr1, "text", 0);
	gtk_tree_view_column_add_attribute(c2, cr2, "text", 1);
    gtk_tree_view_column_add_attribute(c3, cr3, "text", 2);
    gtk_tree_view_column_add_attribute(c4, cr4, "text", 3);
    gtk_tree_view_column_add_attribute(c5, cr5, "text", 4);
  
	GtkTreeIter iter;
	GtkTreeIter iterChild1;
	GtkTreeIter iterChild2;
	GtkTreeIter iterChild3;
	GtkTreeIter iterChild4;
	GtkTreeIter iterChild5;
	GtkTreeIter iterChild6;
	GtkTreeIter iterChild7;
	GtkTreeIter iterChild8;
	GtkTreeIter iterChild9;
	GtkTreeIter iterChild10;
	GtkTreeIter iterChild11;

    printf("Hello World!");

	char ** result = fauxData();
	char * data = "To be Added";
	int * pids = get_pid();
        proc_list_t * proc_list = list_view(pids);

	for(int i = 0; i < indexT; i++){
		//char lev_to_convert[5];
		//lev_to_convert[0] = result[i][6];
		//lev_to_convert[1] = '\0';
		int level;
		int pid_t;
		int check = sscanf(result[i], "level %d PID %d", &level, &pid_t);	
 		int index_pid;
		for (int j = 0; j < proc_list->count; j++) {
			if (proc_list->info_list[j] == NULL) {
				continue;
			}
			if (proc_list->info_list[j]->pid == pid_t) {
				index_pid = j;
				break;
			}
		}

        char state_t[2];
        state_t[0] = proc_list->info_list[index_pid]->state;
        state_t[1] = '\0';
        char cpu_t[7];
        sprintf(cpu_t, "%.2f", proc_list->info_list[index_pid]->cpu_percent);
        char pid_tt[10];
        sprintf(pid_tt, "%d", proc_list->info_list[index_pid]->pid);
        char rss[20];
        double rssInMiB = (double) proc_list->info_list[index_pid]->rss / (1024 * 1024);
        sprintf(rss, "%.2f MiB", rssInMiB);
		if (level == 0) {
			gtk_tree_store_append(treeStore, &iter, NULL);
      		gtk_tree_store_set(treeStore, &iter, 0, proc_list->info_list[index_pid]->process_name, -1);
        	gtk_tree_store_set(treeStore, &iter, 1, proc_list->info_list[index_pid]->state_str, -1);
            gtk_tree_store_set(treeStore, &iter, 2, cpu_t, -1);
            gtk_tree_store_set(treeStore, &iter, 3, pid_tt, -1);
            gtk_tree_store_set(treeStore, &iter, 4, rss, -1);

		}
		if (level == 1) {
			gtk_tree_store_append(treeStore, &iterChild1, &iter);
        	gtk_tree_store_set(treeStore, &iterChild1, 0, proc_list->info_list[index_pid]->process_name, -1);
        	gtk_tree_store_set(treeStore, &iterChild1, 1, proc_list->info_list[index_pid]->state_str, -1);
            gtk_tree_store_set(treeStore, &iterChild1, 2, cpu_t, -1);
            gtk_tree_store_set(treeStore, &iterChild1, 3, pid_tt, -1);
            gtk_tree_store_set(treeStore, &iterChild1, 4, rss, -1);
		}
		if (level == 2) {
                        gtk_tree_store_append(treeStore, &iterChild2, &iterChild1);
                        gtk_tree_store_set(treeStore, &iterChild2, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild2, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild2, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild2, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild2, 4, rss, -1);
                }
		if (level == 3) {
                        gtk_tree_store_append(treeStore, &iterChild3, &iterChild2);
                        gtk_tree_store_set(treeStore, &iterChild3, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild3, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild3, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild3, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild3, 4, rss, -1);
                }
		if (level == 4) {
                        gtk_tree_store_append(treeStore, &iterChild4, &iterChild3);
                        gtk_tree_store_set(treeStore, &iterChild4, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild4, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild4, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild4, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild4, 4, rss, -1);
                }
		if (level == 5) {
                        gtk_tree_store_append(treeStore, &iterChild5, &iterChild4);
                        gtk_tree_store_set(treeStore, &iterChild5, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild5, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild5, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild5, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild5, 4, rss, -1);
                }
		if (level == 6) {
                        gtk_tree_store_append(treeStore, &iterChild6, &iterChild5);
                        gtk_tree_store_set(treeStore, &iterChild6, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild6, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild6, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild6, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild6, 4, rss, -1);
                }
		if (level == 7) {
                        gtk_tree_store_append(treeStore, &iterChild7, &iterChild6);
                        gtk_tree_store_set(treeStore, &iterChild7, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild7, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild7, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild7, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild7, 4, rss, -1);
                }
		if (level == 8) {
                        gtk_tree_store_append(treeStore, &iterChild8, &iterChild7);
                        gtk_tree_store_set(treeStore, &iterChild8, 0, proc_list->info_list[index_pid]->process_name, -1);
                        gtk_tree_store_set(treeStore, &iterChild8, 1, proc_list->info_list[index_pid]->state_str, -1);
                        gtk_tree_store_set(treeStore, &iterChild8, 2, cpu_t, -1);
                        gtk_tree_store_set(treeStore, &iterChild8, 3, pid_tt, -1);
                        gtk_tree_store_set(treeStore, &iterChild8, 4, rss, -1);
                }
	}
	
    printf("THis was loaded alright!\n");
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
}

typedef struct {
    GtkWidget *drawing_area;
    double data[60][8];  // Assuming 8 CPU cores
    int index;
} plot_data_st_t;


void draw_curve(GtkWidget *widget, cairo_t *cr, plot_data_st_t *plot_data) {
    GdkRectangle allocation;
    gtk_widget_get_allocation(widget, &allocation);

    double x_scale = (double)allocation.width / (60 - 1);
    double y_scale = (double)allocation.height / 100.0;
    for (int type = 0; type < 8; type++) {
        cairo_set_source_rgb(cr, rand()/100, rand()/100, rand()/100);
        cairo_set_line_width(cr, 2.0);
        cairo_move_to(cr, 0, allocation.height - plot_data->data[0][type] * y_scale);

        for (int i = 1; i < 60; i++) {
            cairo_line_to(cr, i * x_scale, allocation.height - plot_data->data[i][type] * y_scale);
        }

        cairo_stroke(cr);
    }
}

gboolean update_plot_callback(gpointer user_data) {
    plot_data_st_t *plot_data = (plot_data_st_t *)user_data;
    
    FILE* netdev_file;
    static unsigned long long prev_recv_bytes = 0, prev_send_bytes = 0;

    if ((netdev_file = fopen("/proc/net/dev", "r")) == NULL) {
        perror("Error opening /proc/net/dev");
        return;
    }

    char line[1024];
    unsigned long long recv_bytes, recv_packets, send_bytes, send_packets;

    // Skip the first two lines in /proc/net/dev
    fgets(line, sizeof(line), netdev_file);
    fgets(line, sizeof(line), netdev_file);
    long long unsigned recv_rate;
    long long unsigned  send_rate;
    int count = 0;
    while (fgets(line, sizeof(line), netdev_file) != NULL) {
        if (sscanf(line, "%*s %llu %*u %*u %*u %*u %*u %*u %*u %*u %llu %llu %*u %*u %*u %*u %*u %*u %*u %*u %*u %llu", 
        &recv_bytes, &send_bytes, &recv_packets) == 3) {
            // Calculate rates in kilobytes per second
            if (count == 1) {
                
                recv_rate = (recv_bytes) / (1024*1024*1024);
                send_rate = (send_bytes) / (1024*1024);
                plot_data->data[plot_data->index][0] = 20;
                plot_data->data[plot_data->index][1] = 30;
                
                printf("Received Rate: %f mb/s\nSent Rate: %f mb/s\n", (double)recv_rate, (double)send_rate);
                break;
            }
            count++;
            
            //printf("Received Rate: %llu mB/s\nSent Rate: %llu mB/s\n", recv_rate, send_rate);
        }
    }
    // Print network rates
    //printf("Received Rate: %llu mb/s\nSent Rate: %llu mb/s\n", recv_rate, send_rate);
    
    //plot_dat->data[plot_data->index][2] = send_enet;
    //plot_dat->data[plot_data->index][3] = rec_enet;
    fclose(netdev_file);

    plot_data->index = (plot_data->index + 1) % 60;

    gtk_widget_queue_draw(plot_data->drawing_area);
    return TRUE;
}
int argc_t;
char ** argv_t;


static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;

    getSystemInfo();
    gtk_init(&argc_t, &argv_t);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "gladeTesting.glade", NULL);

    // Connect signals, get widgets, etc.

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(builder, NULL);

    grid1 = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    button1 = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
    //label_1 = GTK_WIDGET(gtk_builder_get_object(builder, "label_1"));

    label_osName = GTK_WIDGET(gtk_builder_get_object(builder, "os_name"));
    gtk_label_set_text(GTK_LABEL(label_osName), (const gchar *) osName);

    label_version = GTK_WIDGET(gtk_builder_get_object(builder, "os_version"));
    gtk_label_set_text(GTK_LABEL(label_version), (const gchar *) osVersion);

    label_kernel = GTK_WIDGET(gtk_builder_get_object(builder, "kernel"));
    gtk_label_set_text(GTK_LABEL(label_kernel), (const gchar *) kernelVersion);

    label_memory = GTK_WIDGET(gtk_builder_get_object(builder, "memory"));
    gtk_label_set_text(GTK_LABEL(label_memory), (const gchar *) memorySize);

    label_processor = GTK_WIDGET(gtk_builder_get_object(builder, "processor"));
    gtk_label_set_text(GTK_LABEL(label_processor), (const gchar *) cpuType);

    label_disk = GTK_WIDGET(gtk_builder_get_object(builder, "disk"));
    gtk_label_set_text(GTK_LABEL(label_disk), (const gchar *) diskInfo);

    GdkColor color;
    color.red = 32767;   // Half intensity for the red component
    color.green = 49151; // Three-fourths intensity for the green component
    color.blue = 49151;
    gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &color);

    printf("OS Name: %s\n", osName);
    printf("OS Release Version: %s\n", osVersion);
    printf("Kernel Version: %s\n", kernelVersion);
    printf("Memory Size: %s\n", memorySize);
    printf("Processor Version: %s\n", cpuType);
    printf("Total Disk Space: %s\n", diskInfo);
    

    GtkWidget *grid;
    GtkWidget *drawing_area;

    window = GTK_WIDGET(gtk_builder_get_object(builder, "viewPortgraph1"));
    gtk_window_set_title(GTK_WINDOW(window), "Network Usage Graph");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    plot_data_st_t *plot_data = g_malloc(sizeof(plot_data_st_t));
    plot_data->index = 0;

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 600, 400);
    gtk_grid_attach(GTK_GRID(grid), drawing_area, 0, 0, 1, 1);

    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_curve), plot_data);

    plot_data->drawing_area = drawing_area;
    setTable(0, NULL);

    set_fs_table();

    g_timeout_add(1000, update_plot_callback, plot_data);
    gtk_widget_show(window);

    gtk_main();
    
    return 0;

    //gtk_widget_show_all(window);
}

// int main(int argc, char **argv) {
//     argc_t = argc;
//     argv_t = argv;
//     arg
//     GtkApplication *app;
//     int status;
//     //main2(argc, argv);
//     app = gtk_application_new("org.Netgraph", G_APPLICATION_FLAGS_NONE);
//     g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

//     status = g_application_run(G_APPLICATION(app), argc, argv);
//     g_object_unref(app);
    
// }
 

int main(int argc, char *argv[]) {
    
    getSystemInfo();
    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "gladeTesting.glade", NULL);

    // Connect signals, get widgets, etc.

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(builder, NULL);

    grid1 = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    button1 = GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
    //label_1 = GTK_WIDGET(gtk_builder_get_object(builder, "label_1"));

    label_osName = GTK_WIDGET(gtk_builder_get_object(builder, "os_name"));
    gtk_label_set_text(GTK_LABEL(label_osName), (const gchar *) osName);

    label_version = GTK_WIDGET(gtk_builder_get_object(builder, "os_version"));
    gtk_label_set_text(GTK_LABEL(label_version), (const gchar *) osVersion);

    label_kernel = GTK_WIDGET(gtk_builder_get_object(builder, "kernel"));
    gtk_label_set_text(GTK_LABEL(label_kernel), (const gchar *) kernelVersion);

    label_memory = GTK_WIDGET(gtk_builder_get_object(builder, "memory"));
    gtk_label_set_text(GTK_LABEL(label_memory), (const gchar *) memorySize);

    label_processor = GTK_WIDGET(gtk_builder_get_object(builder, "processor"));
    gtk_label_set_text(GTK_LABEL(label_processor), (const gchar *) cpuType);

    label_disk = GTK_WIDGET(gtk_builder_get_object(builder, "disk"));
    gtk_label_set_text(GTK_LABEL(label_disk), (const gchar *) diskInfo);

    GdkColor color;
    color.red = 32767;   // Half intensity for the red component
    color.green = 49151; // Three-fourths intensity for the green component
    color.blue = 49151;
    gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &color);

    printf("OS Name: %s\n", osName);
    printf("OS Release Version: %s\n", osVersion);
    printf("Kernel Version: %s\n", kernelVersion);
    printf("Memory Size: %s\n", memorySize);
    printf("Processor Version: %s\n", cpuType);
    printf("Total Disk Space: %s\n", diskInfo);
    
    setTable(0, NULL);

    set_fs_table();
    
    gtk_widget_show(window);

    gtk_main();
    
    return 0;
}

void on_select_changed(GtkWidget * c) {
        
        printf("GOT INTO THE CLICK FUNCTION!\n");
        gchar * value1;
        gchar * value2;
        gchar * value3;
        gchar * value4;
        gchar * value5;
        GtkTreeIter iter;
        GtkTreeModel * model;
        
        

        if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(c), &model, &iter) == FALSE) {
                return;
        }
        printf("Hey?");
        gtk_tree_model_get(model, &iter, 0, &value1, -1);
        
        gtk_tree_model_get(model, &iter, 1, &value2, -1);
        
        gtk_tree_model_get(model, &iter, 2, &value3, -1);
        
        gtk_tree_model_get(model, &iter, 3, &value4, -1);
        
        gtk_tree_model_get(model, &iter, 4, &value5, -1);
        
        char procInfo[1000];
        sprintf(procInfo, "%s %s %s %s %s", value1,value2,value3,value4,value5);
        
        printf("%s", procInfo);
        printf("Hey?3");
        builder_2 = gtk_builder_new();
        gtk_builder_add_from_file(builder_2, "glade_proc.glade", NULL);
        printf("Hey?4");
        window2 = GTK_WIDGET(gtk_builder_get_object(builder_2, "unique_window"));
	gtk_builder_connect_signals(builder_2, NULL);        
        printf("Hey?5");
        
        GtkWidget * label_proc = GTK_WIDGET(gtk_builder_get_object(builder_2, "proc_labe"));
        
        int pid_priv = atoi(value4);
        char pid_string[10];
        sprintf(pid_string, "%d", pid_priv);
        
        gtk_label_set_text(GTK_LABEL(label_proc), (const gchar *) pid_string);
        
        printf("Hey?");
        make_mmaps_table(pid_priv);
        
        make_op_table(pid_priv);
        
        make_detail_table(pid_priv);

        gtk_widget_show(window2);
}
