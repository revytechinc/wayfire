/*
 * Linux platform backend implementation.
 *
 * Implements platform-specific operations for Linux systems.
 *
 * Copyright (c) 2026 REVYTECH, Inc.
 */

#include <wayfire/platform-backend.hpp>

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <gbm.h>

namespace wf
{

class platform_linux_t : public platform_backend_t
{
public:
    const char* platform_name() const override
    {
        return "linux";
    }

    char* detect_render_gpu() override
    {
        /*
         * On Linux, wlroots handles GPU detection automatically via udev.
         * We only need to detect if there's a specific GPU that should be
         * preferred (e.g., via environment variables).
         *
         * If WLR_DRM_DEVICES is already set, don't override it.
         */
        if (getenv("WLR_DRM_DEVICES"))
        {
            return nullptr;
        }

        /* Scan /dev/dri for render nodes */
        DIR *dir = opendir("/dev/dri");
        if (!dir)
        {
            return nullptr;
        }

        char usable_cards[8][64];
        int usable_count = 0;
        struct dirent *entry;

        while ((entry = readdir(dir)) != nullptr)
        {
            if (strncmp(entry->d_name, "renderD", 8) != 0)
            {
                continue;
            }

            int render_minor = atoi(entry->d_name + 8);
            if (render_minor == 0 && strcmp(entry->d_name + 8, "0") != 0)
            {
                continue;
            }

            int card_minor = render_minor - 128;
            if (card_minor < 0)
            {
                continue;
            }

            char render_path[64];
            snprintf(render_path, sizeof(render_path), "/dev/dri/%s", entry->d_name);

            if (!gpu_is_usable(render_path))
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
            return nullptr;
        }

        if (usable_count == 1)
        {
            return strdup(usable_cards[0]);
        }

        return strdup("");
    }

    bool is_root() const override
    {
        return (geteuid() == 0);
    }

    bool drop_permissions() override
    {
        /* Get the real UID/GID from environment or usenobody */
        uid_t uid = getuid();
        gid_t gid = getgid();

        /* Try to get the nobody user for privilege dropping */
        struct passwd *pw = getpwnam("nobody");
        if (pw)
        {
            uid = pw->pw_uid;
            gid = pw->pw_gid;
        }

        if ((setgid(gid) != 0) || (setuid(uid) != 0))
        {
            return false;
        }

        /* Verify we cannot regain root */
        if ((setgid(0) != -1) || (setuid(0) != -1))
        {
            return false;
        }

        return true;
    }

    std::string get_render_node_path(const std::string& card_path) override
    {
        /* Extract card number from /dev/dri/cardN */
        int card_num = -1;
        if (sscanf(card_path.c_str(), "/dev/dri/card%d", &card_num) != 1)
        {
            return {};
        }

        char render_path[64];
        snprintf(render_path, sizeof(render_path), "/dev/dri/renderD%d", card_num + 128);

        struct stat st;
        if (stat(render_path, &st) == 0)
        {
            return render_path;
        }

        return {};
    }

    bool gpu_is_usable(const char* render_path) override
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

        struct gbm_bo *bo = gbm_bo_create(gbm, 64, 64,
            GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT);

        if (bo)
        {
            gbm_bo_destroy(bo);
        }

        gbm_device_destroy(gbm);
        close(fd);

        return bo != nullptr;
    }
};

} // namespace wf
