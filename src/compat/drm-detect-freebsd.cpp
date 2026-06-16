/*
 * FreeBSD GPU detection implementation.
 *
 * Scans /dev/dri/ for render nodes, tests GBM allocation on each.
 * Only GPUs where GBM works are considered "usable". If exactly one GPU
 * works, returns its card path so WLR_DRM_DEVICES can be set to restrict
 * wlroots to that GPU, avoiding multi-GPU DMA buffer sharing failures.
 *
 * On hybrid-graphics FreeBSD laptops (e.g. Intel iGPU + NVIDIA dGPU),
 * both GPUs may expose render nodes, but only NVIDIA's GBM works for
 * rendering. Testing GBM allocation distinguishes usable from unusable GPUs.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gbm.h>

/*
 * Test whether GBM can allocate a buffer on a render node.
 * Returns true if a minimal GBM bo can be created (GPU is usable for rendering).
 */
static bool gbm_works(const char *render_path)
{
    int fd = open(render_path, O_RDWR | O_CLOEXEC);
    if (fd < 0)
    {
        return false;
    }

    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm)
    {
        close(fd);
        return false;
    }

    /* Try to allocate the smallest possible buffer — if this fails, GBM is broken */
    struct gbm_bo *bo = gbm_bo_create(gbm, 64, 64,
        GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT);
    if (bo)
    {
        gbm_bo_destroy(bo);
    }

    gbm_device_destroy(gbm);
    close(fd);
    return bo != NULL;
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
            /* Not a valid number */
            continue;
        }

        int card_minor = render_minor - 128;
        if (card_minor < 0)
        {
            continue;
        }

        char render_path[64];
        snprintf(render_path, sizeof(render_path), "/dev/dri/%s", entry->d_name);

        /* Skip GPUs where GBM allocation fails — not usable for rendering */
        if (!gbm_works(render_path))
        {
            continue;
        }

        char card_path[64];
        snprintf(card_path, sizeof(card_path), "/dev/dri/card%d", card_minor);
        struct stat st;
        if (stat(card_path, &st) != 0)
        {
            continue;
        }

        if (usable_count >= 8)
        {
            break;
        }

        snprintf(usable_cards[usable_count], sizeof(usable_cards[0]), "%s", card_path);
        usable_count++;
    }

    closedir(dir);

    if (usable_count == 0)
    {
        return NULL; /* No GPUs with working GBM */
    }

    if (usable_count == 1)
    {
        /* Exactly one GPU with working GBM — return it for WLR_DRM_DEVICES */
        return strdup(usable_cards[0]);
    }

    /* Multiple GPUs have working GBM — no filtering (user's choice) */
    return strdup("");
}
