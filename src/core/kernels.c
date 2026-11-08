#include "core/kernels.h"

#include "efi/fs.h"
#include "lib/str.h"

#define KERNEL_FIELD_COUNT 5

static int is_skipped_line(const char *line)
{
    return line[0] == '\0' || line[0] == '#';
}

static int is_directive(const char *line)
{
    return line[0] == '@';
}

static void apply_directive(Entry *entry, char *line)
{
    char *fields[2];
    int field_count = str_split(line, ' ', fields, 2);

    if (field_count < 2 || !str_equal(fields[0], "@oneshot")) {
        return;
    }

    if (str_equal(fields[1], "grub")) {
        entry->oneshot = ONESHOT_GRUB;
    } else if (str_equal(fields[1], "systemd-boot")) {
        entry->oneshot = ONESHOT_SYSTEMD_BOOT;
    } else {
        entry->oneshot = ONESHOT_NONE;
    }
}

static void parse_kernel(Kernel *kernel, char **fields, int field_count)
{
    mem_set(kernel, 0, sizeof(*kernel));

    str_copy(kernel->title, fields[0], sizeof(kernel->title));
    str_copy(kernel->id, fields[1], sizeof(kernel->id));

    if (field_count < KERNEL_FIELD_COUNT) {
        return;
    }

    if (str_is_empty(fields[2])) {
        return;
    }

    str_copy(kernel->kernel, fields[2], sizeof(kernel->kernel));
    str_copy(kernel->initrd, fields[3], sizeof(kernel->initrd));
    str_copy(kernel->cmdline, fields[4], sizeof(kernel->cmdline));
}

static void parse_kernel_list(Entry *entry, char *text)
{
    char *cursor = text;
    char *line = str_next_line(&cursor);

    while (line != NULL && entry->kernel_count < AARAMBH_MAX_KERNELS) {
        char *fields[KERNEL_FIELD_COUNT];
        int field_count;

        if (is_directive(line)) {
            apply_directive(entry, line);
        } else if (!is_skipped_line(line)) {
            field_count = str_split(line, '|', fields, KERNEL_FIELD_COUNT);

            if (field_count >= 2 && !str_is_empty(fields[1])) {
                parse_kernel(&entry->kernels[entry->kernel_count], fields, field_count);
                entry->kernel_count++;
            }
        }

        line = str_next_line(&cursor);
    }
}

void kernels_load(Entry *entry)
{
    char file_name[64];
    char *text;

    if (str_is_empty(entry->key)) {
        return;
    }

    str_copy(file_name, entry->key, sizeof(file_name));
    str_append(file_name, ".kernels", sizeof(file_name));

    text = fs_read(file_name, NULL);
    if (text == NULL) {
        return;
    }

    entry->kernels = mem_alloc(sizeof(Kernel) * AARAMBH_MAX_KERNELS);
    if (entry->kernels != NULL) {
        parse_kernel_list(entry, text);
    }

    mem_free(text);
}
