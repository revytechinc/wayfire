/*
 * FreeBSD GPU detection implementation.
 *
 * Scans /dev/dri/ for render node minor numbers and matches them against
 * available card devices. If exactly one GPU has a render node, returns its
 * card path so WLR_DRM_DEVICES can be set to restrict wlroots to that GPU.
 *
 * On Linux, GPUs with render nodes have device minors 128+n where n is the
 * card minor. FreeBSD follows the same convention.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Check if a render node (by minor number) maps to a known card device.
 * On FreeBSD, render node minor N maps to card device drm/N-128 if that exists.
 *
 * Returns true if a corresponding card device exists.
 */
static bool card_exists_for_render_minor(int render_minor)
{
    char path[64];
    int card_minor = render_minor - 128; /* Linux/FreeBSD convention: renderN = card(N-128) */

    if (card_minor < 0)
    {
        return false;
    }

    snprintf(path, sizeof(path), "/dev/dri/card%d", card_minor);
    struct stat st;
    return stat(path, &st) == 0;
}

char *wf_freebsd_detect_render_gpu(void)
{
    DIR *dir = opendir("/dev/dri");
    if (!dir)
    {
        return NULL;
    }

    char usable_cards[8][32]; /* max 8 GPUs, store /dev/dri/cardN strings */
    int usable_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        /* Look for renderD128, renderD129, etc. */
        if (strncmp(entry->d_name, "renderD", 8) != 0)
        {
            continue;
        }

        int render_minor = atoi(entry->d_name + 8);
        if (render_minor == 0 && strcmp(entry->d_name + 8, "0") != 0)
        {
            /* Not a valid number, skip */
            continue;
        }

        if (!card_exists_for_render_minor(render_minor))
        {
            /* No corresponding card device */
            continue;
        }

        if (usable_count >= 8)
        {
            break; /* Too many, don't bother */
        }

        int card_minor = render_minor - 128;
        snprintf(usable_cards[usable_count], sizeof(usable_cards[0]),
            "/dev/dri/card%d", card_minor);
        usable_count++;
    }

    closedir(dir);

    if (usable_count == 0)
    {
        return NULL; /* No GPUs with render nodes */
    }

    if (usable_count == 1)
    {
        /* Exactly one GPU with render node — return it for WLR_DRM_DEVICES */
        return strdup(usable_cards[0]);
    }

    /* Multiple GPUs have render nodes — no filtering needed */
    return strdup("");
}
