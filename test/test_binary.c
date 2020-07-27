#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/file.h>

#include <helper.h>
#include "binary_reader.h"

#include "test.h"

#define BUF_SIZE 80000
#define SAMPLE_DIR "test/binary-sample/"

static char buf[BUF_SIZE] = {0},
            preempts[3][64] = {0},
            files[10][64] = {0};
static size_t buf_size = 0;
static int files_count = 0, preempts_count = 0;

int alphasort_r(const struct dirent **a, const struct dirent **b)
{
    return -strcoll((*a)->d_name, (*b)->d_name);
}

int get_list_files()
{
    struct dirent **namelist;
    int n;
    char *dst, *src;

    n = scandir(SAMPLE_DIR, &namelist, NULL, alphasort_r);
    if (n == -1)
        return 1;

    while (n--)
    {
        src = namelist[n]->d_name;
        switch (*src)
        {
        case '1':
            dst = files[files_count++];
            break;
        case 'p':
            dst = preempts[preempts_count++];
            break;
        default:
            info("Ignoring %s", src);
            continue;
        }
        strcpy(dst, src);
        free(namelist[n]);
    }
    free(namelist);
    preempts[preempts_count][0] = 0;
    files[files_count][0] = 0;
    return 0;
}

int load_file(char *name)
{
    char path[64] = {0};
    FILE *fd;
    size_t recv;

    strcat(path, SAMPLE_DIR);
    strcat(path, name);

    fd = fopen(path, "r");
    buf_size = 0;

    do
    {
        recv = fread(buf, 1, BUF_SIZE - buf_size, fd);
        buf_size += recv;
    } while (recv > 0);

    fclose(fd);

    accent("%s: %lu bytes", name, buf_size);
    return 0;
}

int test_read_int()
{
    char buf[] = "\xcd"              // int8
                 "\xf0\x0f"          // int16
                 "\x0a\xbb\x0c"      // int 20
                 "\xaa\xbb\xcc\xdd"; // int32
    binary_read_set(buf, 9);
    accent("int8  0x%02x", read_byte());
    accent("int16 0x%04x", read_int16());
    accent("int20 0x%06x", read_int20());
    accent("int32 0x%08x", read_int32());

    // reset index, keep old data
    binary_read_set(buf, 9);

    TRUTHY(read_byte() == 0xcd);
    TRUTHY(read_int16() == 0xf00f);
    TRUTHY(read_int20() == 0x0abb0c);
    TRUTHY(read_int32() == 0xaabbccdd);
    // TRUTHY(read_int16() == 0x0ff0);
    // TRUTHY(read_int20() == 0x0cbbaa);
    // TRUTHY(read_int32() == 0xaabbccdd);
    return 0;
}

int test_preempt()
{
    TRUTHY(preempts_count == 2);

    // First preempts is contact lists
    ZERO(load_file(preempts[0]));
    BINARY_NODE *node = binary_read(buf, buf_size);
    FALSY(node == NULL);

    ZERO(strcmp(node->tag, "response"));
    TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
    TRUTHY(node->child_len == 901);

    TRUTHY(node->attr_len == 2);
    FALSY(node->attrs[0].key == NULL);
    FALSY(node->attrs[1].key == NULL);
    TRUTHY(node->attrs[2].key == NULL);
    ZERO(strcmp(binary_attr(node, "checksum"), "1BE33034-E32C-40EA-BB66-554B81AF0AE7"));
    ZERO(strcmp(binary_attr(node, "type"), "contacts"));

    binary_free();
    return 0;
}

int test_main()
{
    return test_read_int() || test_preempt();
}

int test_setup()
{
    ZERO(get_list_files());
    return 0;
}

int test_cleanup()
{
    return 0;
}
