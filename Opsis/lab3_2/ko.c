#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/ktime.h> 

#define PROC_NAME "tsulab"

// Base date: December 18, 2025, 00:00 UTC
#define BASE_TIMESTAMP 1766016000L  // 2025-12-18 00:00:00 UTC

// Neptune's astronomical parameters
#define ORBITAL_PERIOD_SECONDS 5198400000LL  // 164.8 years in seconds (164.8 * 365.25 * 24 * 3600)
#define CURRENT_CONSTELLATION "Pisces"
#define ORBITAL_SPEED_KM_PER_SEC 5  // Integer value (km/s)
#define DISTANCE_FROM_SUN_KM 4504000000UL  // 4.5 billion km

static struct proc_dir_entry *proc_file;

static ssize_t procfile_read(struct file *file, char __user *buffer,
                             size_t len, loff_t *offset)
{
    char msg[600];
    time64_t now = ktime_get_real_seconds();
    s64 elapsed_sec = now - BASE_TIMESTAMP;

    unsigned long years, months, days, hours, minutes, secs;
    unsigned long full_orbits;
    time64_t time_in_current_orbit;
    time64_t time_until_return;
    unsigned long orbit_completion_percent;
    unsigned long neptune_distance_km;
    unsigned long distance_increase;

    if (*offset > 0)
        return 0;

    if (elapsed_sec < 0)
        elapsed_sec = 0;

    // Number of full orbits since base date
    full_orbits = elapsed_sec / ORBITAL_PERIOD_SECONDS;
    
    // Time in current orbit
    time_in_current_orbit = elapsed_sec % ORBITAL_PERIOD_SECONDS;
    
    // Time until returning to the same constellation
    time_until_return = ORBITAL_PERIOD_SECONDS - time_in_current_orbit;
    
    // Current orbit completion percentage (integer multiplied by 100)
    orbit_completion_percent = (100ULL * time_in_current_orbit) / ORBITAL_PERIOD_SECONDS;

    // Neptune's distance from the Sun (simplified calculation)
    neptune_distance_km = DISTANCE_FROM_SUN_KM;
    
    // Distance increase
    distance_increase = ORBITAL_SPEED_KM_PER_SEC * elapsed_sec;

    // Time until return (broken down into components)
    years = time_until_return / (365 * 24 * 3600);
    months = (time_until_return % (365 * 24 * 3600)) / (30 * 24 * 3600);
    days = (time_until_return % (30 * 24 * 3600)) / (24 * 3600);
    hours = (time_until_return % (24 * 3600)) / 3600;
    minutes = (time_until_return % 3600) / 60;
    secs = time_until_return % 60;

    int msg_len = snprintf(msg, sizeof(msg),
        "=========== TSU: Astronomical Laboratory ===========\n"
        "                     PLANET NEPTUNE\n\n"
        "Current position: %s constellation\n"
        "Distance from Sun: %lu million km\n\n"
        "Time until returning to the same constellation:\n"
        "%lu years %lu months %lu days\n"
        "%lu hours %lu minutes %lu seconds\n\n"
        "Current orbit progress: %lu.%02lu%%\n"
        "Full orbits since 2025-12-18: %lu\n\n"
        "Technical parameters:\n"
        "Orbital period: 164.8 years\n"
        "===================================================\n",
        CURRENT_CONSTELLATION,
        neptune_distance_km / 1000000,
        years, months, days,
        hours, minutes, secs,
        orbit_completion_percent / 100,    // Integer part of percentage
        orbit_completion_percent % 100,    // Fractional part (two digits)
        full_orbits,
        ORBITAL_SPEED_KM_PER_SEC,
        distance_increase / 1000000);

    if (copy_to_user(buffer, msg, msg_len))
        return -EFAULT;

    *offset = msg_len;
    return msg_len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_fops = {
    .read = procfile_read,
};
#endif

static int __init tsu_module_init(void)
{
    proc_file = proc_create(PROC_NAME, 0444, NULL, &proc_fops);
    if (!proc_file)
        return -ENOMEM;

    pr_info("Welcome to the Tomsk State University - Neptune Lab\n");
    return 0;
}

static void __exit tsu_module_exit(void)
{
    proc_remove(proc_file);
    pr_info("Tomsk State University forever!\n");
}

module_init(tsu_module_init);
module_exit(tsu_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSU Astronomy Department");
MODULE_DESCRIPTION("Neptune orbital information module");