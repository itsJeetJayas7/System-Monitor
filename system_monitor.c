#include <stdio.h>    // Standard Input/Output functions
#include <stdlib.h>   // General Utilities Library   // Variable arguments
#include <string.h>   // String handling functions
#include <math.h>     // Mathematical functions
#include <ctype.h>    // Character handling functions
#include <signal.h>   // Signal handling
#include <unistd.h>   // POSIX operating system API
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <gtk/gtkx.h>

GtkWidget * window;
GtkWidget * button1;
GtkWidget * label_1;
GtkWidget * grid1;
GtkBuilder *builder;

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
    FILE *fp = popen("sw_vers -productName", "r");
    if (fp == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    if (fgets(osName, sizeof(osName), fp) != NULL) {
        // Remove newline character from the end
        osName[strcspn(osName, "\n")] = '\0';
    } else {
        printf("Failed to retrieve OS name\n");
    }

    pclose(fp);

    // Get OS release version
    char osV[256];
    size_t len = sizeof(osVersion);
    sysctlbyname("kern.osrelease", &osV, &len, NULL, 0);
    char * temp3 = "OS Version: ";
    strcat(osVersion, temp3);
    strcat(osVersion, osV);

    // Get kernel version
    char kvers[256];
    len = sizeof(kernelVersion);
    sysctlbyname("kern.version", &kvers, &len, NULL, 0);
    char * temp2 = "Kernel Version: ";
    kvers[strcspn(kvers, ":")] = '\0';
    strcat(kernelVersion, temp2);
    strcat(kernelVersion, kvers);

    // Get amount of memory
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t memSize;
    size_t size = sizeof(memSize);
    sysctl(mib, 2, &memSize, &size, NULL, 0);
    snprintf(memorySize, sizeof(memorySize), "Memory Size: %.2f GiB", (double)memSize / (1 << 30));

    // Get processor version
    char cput[256];
    len = sizeof(cpuType);
    sysctlbyname("machdep.cpu.brand_string", &cput, &len, NULL, 0);
    char * temp = "Processor Version: ";
    strcat(cpuType, temp);
    strcat(cpuType, cput);

    // Get disk storage
    struct statfs diskStats;
    statfs("/", &diskStats);
    uint64_t totalDiskSpace = diskStats.f_blocks * diskStats.f_bsize;
    snprintf(diskInfo, sizeof(diskInfo), "Total Disk Space: %.2f GiB", (double)totalDiskSpace / (1 << 30));
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

    gtk_widget_show(window);

    gtk_main();

    return 0;
}