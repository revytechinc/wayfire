/*
 * Platform backend factory implementation.
 *
 * Creates the appropriate platform-specific backend based on the
 * current operating system.
 *
 * Copyright (c) 2026 REVYTECH, Inc.
 */

#include <wayfire/platform-backend.hpp>
#include "platform-linux.cpp"
#include "platform-freebsd.cpp"

namespace wf
{

std::unique_ptr<platform_backend_t> create_platform_backend()
{
#if defined(__FreeBSD__)
    return std::make_unique<platform_freebsd_t>();
#elif defined(__linux__)
    return std::make_unique<platform_linux_t>();
#else
    #error "Unsupported platform"
#endif
}

} // namespace wf
