/* SPDX-License-Identifier: MIT */
/*
 * FreeBSD-specific GPU detection for wayfire.
 *
 * Detects which DRM GPUs have render nodes and (if only one GPU is usable)
 * sets WLR_DRM_DEVICES to that GPU's card device so wlroots doesn't
 * try to initialize all GPUs and hit multi-GPU DMA buffer sharing issues.
 *
 * Copyright (c) 2026 REVYTECH, Inc.
 */

#ifndef LINUX_DRM_DETECT_H
#define LINUX_DRM_DETECT_H

#include <stddef.h>

/*
 * Detect GPUs with usable render nodes on FreeBSD.
 *
 * Returns a newly-allocated string like "/dev/dri/card1" (or NULL on error)
 * identifying the GPU to use. The caller owns the returned string.
 *
 * On FreeBSD, only GPUs that expose a render node can run a GL renderer.
 * When multiple GPUs exist but only one has a render node, using all GPUs
 * causes buffer-sharing failures on hybrid-graphics laptops (e.g. Intel+NVIDIA).
 * This function detects that situation and returns the usable GPU so callers
 * can set WLR_DRM_DEVICES to restrict wlroots to that GPU.
 *
 * Returns NULL if no GPUs with render nodes are found, or on error.
 * Returns an empty string "" if multiple GPUs have render nodes (no filtering needed).
 * Returns a path like "/dev/dri/card1" if only one GPU has a render node.
 */
char *wf_freebsd_detect_render_gpu(void);

#endif /* LINUX_DRM_DETECT_H */
