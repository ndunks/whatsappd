#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "color.h"
#include "cfg.h"

/* #define BUF_SIZE 3480
typedef enum cfg_maps {
    cfg_field_client_id,

    cfg_field_keys_secret,
    cfg_field_keys_private,
    cfg_field_keys_public,

    cfg_field_tokens_client,
    cfg_field_tokens_server,
    cfg_field_tokens_browser,

    cfg_field_serverSecret,
    cfg_field_aesKey,
    cfg_field_macKey

} cfg_maps;

typedef struct cfg_field
{
    unsigned char key_id, value_len;
    char * value;
} cfg_field;

static char * buf;
 */

// static int cfg_config_file_open(int flag){
//     return open(cfg_config_file, flag);

//     // struct passwd *pw = getpwuid(getuid());
//     // const char *finalPath;
//     // finalPath = malloc( strlen(pw->pw_dir) + 20 );
//     // strcpy(finalPath, pw->pw_dir );
//     // strcat(finalPath, cfg_config_file);

//     // free(finalPath);
//     // free(pw);
// }
char *cfg_config_file = NULL;

int cfg_load(CFG *cfg)
{
    int file;
    if (cfg_config_file == NULL)
        cfg_config_file = &CFG_DEFAULT_CONFIG_FILE;

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
        cfg_config_file = &CFG_DEFAULT_CONFIG_FILE;

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