#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "color.h"
#include "cfg.h"

char *cfg_config_file = NULL;

int cfg_load(CFG *cfg)
{
    int file;
    if (cfg_config_file == NULL)
        cfg_config_file = CFG_DEFAULT_CONFIG_FILE;

    file = open(cfg_config_file, O_RDONLY);
    ssize_t size = sizeof(CFG);

    if (file < 0)
    {
        err("Fail open load %s", cfg_config_file);
        return -1;
    }

    if (read(file, (void *)cfg, size) != size)
    {
        err("Fail read %s", cfg_config_file);
        close(file);
        return 1;
    };

    close(file);
    return 0;
}

int cfg_save(CFG *cfg)
{
    int file;
    if (cfg_config_file == NULL)
        cfg_config_file = CFG_DEFAULT_CONFIG_FILE;

    file = open(cfg_config_file, O_WRONLY | O_CREAT);
    ssize_t size = sizeof(CFG);

    if (file < 0)
    {
        err("Fail open save %s", cfg_config_file);
        return -1;
    }

    if (write(file, (void *)cfg, size) != size)
    {
        err("Fail write %s", cfg_config_file);
        close(file);
        return 1;
    };

    close(file);
    return 0;
}