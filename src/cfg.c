#include <sys/file.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>

#include "color.h"
#include "cfg.h"

static char cfg_config_file[255] = {0};

const char *cfg_file_get()
{
    return cfg_config_file;
}

int cfg_file(const char *file_path)
{
    char *dir = NULL, tmp[255];
    cfg_config_file[0] = 0;

    if (file_path == NULL)
    {
        if ((dir = getenv("HOME")) == NULL)
        {
            dir = getpwuid(getuid())->pw_dir;
        }
        strcat(cfg_config_file, dir);
        strcat(cfg_config_file, "/.whatsappd.cfg");
    }
    else if (strlen(file_path) > 254)
    {
        return -1;
    }
    else
    {
        strcpy(cfg_config_file, file_path);
    }

    if (access(cfg_config_file, F_OK) == 0)
    {
        // file exists and has RW Access
        if (
            access(cfg_config_file, R_OK) ||
            access(cfg_config_file, W_OK))
            return -1;
        else
            return 1;
    }

    if (dir == NULL)
    {
        strcpy(tmp, file_path);
        dir = dirname(tmp);
    }

    // dir exists and RW OK
    if (access(dir, F_OK) || access(dir, R_OK) || access(dir, W_OK))
        return -1;

    return 0;
}

int cfg_load(CFG *cfg)
{
    int file = open(cfg_config_file, O_RDONLY);
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

    if (cfg->cfg_file_version != 1)
    {
        err("Unsupported config file version");
        return 1;
    }

    return 0;
}

int cfg_save(CFG *cfg)
{
    int file = open(cfg_config_file, O_WRONLY | O_CREAT, 0644);
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

int cfg_has_credentials(CFG *cfg)
{
    char null[256];
    memset(null, 0, 256);

    return (
        cfg->tokens.browser[0] &&
        cfg->tokens.client[0] &&
        cfg->tokens.server[0] &&
        memcmp(null, cfg->client_id, CFG_CLIENT_ID_LEN) &&
        memcmp(null, cfg->serverSecret, CFG_SERVER_SECRET_LEN) &&
        memcmp(null, cfg->keys.private, CFG_KEY_LEN) &&
        memcmp(null, cfg->keys.public, CFG_KEY_LEN));
}