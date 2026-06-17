/*
 * Platform abstraction layer for Wayfire.
 *
 * Provides an abstract interface for OS-specific operations, allowing
 * Wayfire to support multiple Unix-like systems (Linux, FreeBSD, etc.)
 * with a clean factory pattern.
 *
 * Copyright (c) 2026 REVYTECH, Inc.
 */

#ifndef WAYFIRE_PLATFORM_BACKEND_HPP
#define WAYFIRE_PLATFORM_BACKEND_HPP

#include <string>
#include <memory>
#include <cstddef>

namespace wf
{

/**
 * Abstract base class for platform-specific operations.
 *
 * This class defines the interface for OS-specific functionality that
 * may differ between Linux, FreeBSD, and other Unix-like systems.
 * Subclasses implement the actual behavior for each target platform.
 */
class platform_backend_t
{
public:
    virtual ~platform_backend_t() = default;

    /**
     * Returns the name of the current platform (e.g., "linux", "freebsd").
     */
    virtual const char* platform_name() const = 0;

    /**
     * Detect the GPU to use for rendering on hybrid-graphics systems.
     *
     * On systems with multiple GPUs, this detects which GPU has usable
     * render nodes and can be used for rendering.
     *
     * @return A newly-allocated string with the GPU path (e.g., "/dev/dri/card0"),
     *         an empty string if multiple GPUs are available (no filtering),
     *         or nullptr on error. Caller owns the returned string.
     */
    virtual char* detect_render_gpu() = 0;

    /**
     * Check if the system is running as root.
     *
     * @return true if running as root (uid 0), false otherwise.
     */
    virtual bool is_root() const = 0;

    /**
     * Drop elevated privileges if running as root.
     *
     * Returns false if privileges cannot be dropped safely.
     */
    virtual bool drop_permissions() = 0;

    /**
     * Get the path to the DRM render node for a given card.
     *
     * @param card_path The path to the DRM card device (e.g., "/dev/dri/card0").
     * @return The path to the corresponding render node, or empty string if none.
     */
    virtual std::string get_render_node_path(const std::string& card_path) = 0;

    /**
     * Check if a GPU is usable for rendering (GBM allocation works).
     *
     * @param render_path Path to the render node (e.g., "/dev/dri/renderD128").
     * @return true if GBM can allocate buffers on this GPU.
     */
    virtual bool gpu_is_usable(const char* render_path) = 0;
};

/**
 * Factory function to create the appropriate platform backend.
 *
 * Returns a new instance of the platform-specific backend for the
 * current operating system. The returned pointer is owned by the caller.
 */
std::unique_ptr<platform_backend_t> create_platform_backend();

} // namespace wf

#endif // WAYFIRE_PLATFORM_BACKEND_HPP
