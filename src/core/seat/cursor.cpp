#include "cursor.hpp"
#include "pointer.hpp"
#include "../core-impl.hpp"
#include "../../view/view-impl.hpp"
#include "input-manager.hpp"
#include "wayfire/util.hpp"
#include "wayfire/output-layout.hpp"
#include "tablet.hpp"
#include "wayfire/signal-definitions.hpp"
#include <wayfire/util/log.hpp>
#include <wayfire/bindings-repository.hpp>

wf::cursor_t::cursor_t(wf::seat_t *seat)
{
    cursor     = wlr_cursor_create();
    this->seat = seat;

    wlr_cursor_attach_output_layout(cursor,
        wf::get_core().output_layout->get_handle());

    wlr_cursor_map_to_output(cursor, NULL);
    wlr_cursor_warp(cursor, NULL, cursor->x, cursor->y);
    init_xcursor();
    init_cursor_shape_manager();
    init_cursor_recovery();

    config_reloaded = [=] (auto)
    {
        init_xcursor();
    };

    wf::get_core().connect(&config_reloaded);

    request_set_cursor.set_callback([&] (void *data)
    {
        auto ev = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);
        set_cursor(ev, true);
    });
    request_set_cursor.connect(&seat->seat->events.request_set_cursor);
}

void wf::cursor_t::add_new_device(wlr_input_device *dev)
{
    wlr_cursor_attach_input_device(cursor, dev);
}

void wf::cursor_t::setup_listeners()
{
    /* Dispatch pointer events to the pointer_t */
    on_frame.set_callback([&] (void*)
    {
        seat->priv->lpointer->handle_pointer_frame();
        wf::get_core().seat->notify_activity();

        // Recovery option 2: Check cursor state on each frame
        check_cursor_recovery();
    });
    on_frame.connect(&cursor->events.frame);

#define setup_passthrough_callback(evname) \
    on_ ## evname.set_callback([&] (void *data) { \
        set_touchscreen_mode(false); \
        auto ev   = static_cast<wlr_pointer_ ## evname ## _event*>(data); \
        auto mode = emit_device_event_signal(ev, &ev->pointer->base); \
        if (mode != wf::input_event_processing_mode_t::IGNORE) \
        { \
            seat->priv->lpointer->handle_pointer_ ## evname(ev, mode); \
            wf::get_core().seat->notify_activity(); \
        } \
        emit_device_post_event_signal(ev, &ev->pointer->base); \
    }); \
    on_ ## evname.connect(&cursor->events.evname);

    setup_passthrough_callback(button);
    setup_passthrough_callback(motion);
    setup_passthrough_callback(motion_absolute);
    setup_passthrough_callback(axis);
    setup_passthrough_callback(swipe_begin);
    setup_passthrough_callback(swipe_update);
    setup_passthrough_callback(swipe_end);
    setup_passthrough_callback(pinch_begin);
    setup_passthrough_callback(pinch_update);
    setup_passthrough_callback(pinch_end);
    setup_passthrough_callback(hold_begin);
    setup_passthrough_callback(hold_end);
#undef setup_passthrough_callback

    /**
     * All tablet events are directly sent to the tablet device, it should
     * manage them
     */
#define setup_tablet_callback(evname) \
    on_tablet_ ## evname.set_callback([&] (void *data) { \
        set_touchscreen_mode(false); \
        auto ev = static_cast<wlr_tablet_tool_ ## evname ## _event*>(data); \
        auto handling_mode = emit_device_event_signal(ev, &ev->tablet->base); \
        if (ev->tablet->data) { \
            auto tablet = \
                static_cast<wf::tablet_t*>(ev->tablet->data); \
            tablet->handle_ ## evname(ev, handling_mode); \
        } \
        wf::get_core().seat->notify_activity(); \
        emit_device_post_event_signal(ev, &ev->tablet->base); \
    }); \
    on_tablet_ ## evname.connect(&cursor->events.tablet_tool_ ## evname);

    setup_tablet_callback(tip);
    setup_tablet_callback(axis);
    setup_tablet_callback(button);
    setup_tablet_callback(proximity);
#undef setup_tablet_callback
}

void wf::cursor_t::init_xcursor()
{
    std::string theme = wf::option_wrapper_t<std::string>("input/cursor_theme");
    int size = wf::option_wrapper_t<int>("input/cursor_size");
    auto theme_ptr = (theme == "default") ? NULL : theme.c_str();

    // Set environment variables needed for Xwayland and maybe other apps
    // which use them to determine the correct cursor size
    setenv("XCURSOR_SIZE", std::to_string(size).c_str(), 1);
    if (theme_ptr)
    {
        setenv("XCURSOR_THEME", theme_ptr, 1);
    }

    if (xcursor)
    {
        last_cursor_name.clear(); // make sure we set the new cursor image with the new xcursor_manager
        wlr_xcursor_manager_destroy(xcursor);
    }

    xcursor = wlr_xcursor_manager_create(theme_ptr, size);
    set_cursor("default");
}

bool wf::cursor_t::can_client_set_cursor()
{
    if (this->touchscreen_mode_active)
    {
        return false;
    }

    if (this->hide_ref_counter)
    {
        return false;
    }

    return true;
}

void wf::cursor_t::init_cursor_shape_manager()
{
    cursor_shape_manager = wlr_cursor_shape_manager_v1_create(seat->seat->display, 1);
    request_set_cursor_shape.set_callback([&] (void *data)
    {
        auto event = (wlr_cursor_shape_manager_v1_request_set_shape_event*)data;
        const char *shape_name = wlr_cursor_shape_v1_name(event->shape);
        struct wlr_seat_client *focused_client = seat->seat->pointer_state.focused_client;

        if (focused_client != event->seat_client)
        {
            return;
        }

        if (can_client_set_cursor())
        {
            wlr_cursor_set_xcursor(cursor, xcursor, shape_name);
        }
    });
    request_set_cursor_shape.connect(&cursor_shape_manager->events.request_set_shape);
}

void wf::cursor_t::set_cursor(std::string name)
{
    if (!can_client_set_cursor())
    {
        return;
    }

    if (name == "default")
    {
        name = "left_ptr";
    }

    if (name == last_cursor_name)
    {
        return;
    }

    last_cursor_name = name;

    idle_set_cursor.run_once([name, this] ()
    {
        wlr_cursor_set_xcursor(cursor, xcursor, name.c_str());
    });
}

void wf::cursor_t::force_unhide_cursor()
{
    LOGW("Cursor recovery: forcing cursor to become visible");
    hide_ref_counter = 0;
    touchscreen_mode_active = false;
    touchscreen_mode_intended = false;
    cursor_recovery_timer.disconnect();
    set_cursor("default");
}

bool wf::cursor_t::is_hidden() const
{
    return hide_ref_counter > 0 || touchscreen_mode_active;
}

void wf::cursor_t::unhide_cursor()
{
    if (this->hide_ref_counter && --this->hide_ref_counter)
    {
        return;
    }

    set_cursor("default");
}

void wf::cursor_t::hide_cursor()
{
    idle_set_cursor.disconnect();
    wlr_cursor_set_surface(cursor, NULL, 0, 0);
    this->hide_ref_counter++;
    last_cursor_name.clear();
}

void wf::cursor_t::warp_cursor(wf::pointf_t point)
{
    wlr_cursor_warp_closest(cursor, NULL, point.x, point.y);
}

wf::pointf_t wf::cursor_t::get_cursor_position()
{
    return {cursor->x, cursor->y};
}

void wf::cursor_t::set_cursor(
    wlr_seat_pointer_request_set_cursor_event *ev, bool validate_request)
{
    if (!can_client_set_cursor())
    {
        return;
    }

    if (validate_request)
    {
        auto pointer_client = seat->seat->pointer_state.focused_client;
        if (pointer_client != ev->seat_client)
        {
            return;
        }
    }

    if (ev->surface)
    {
        wf::xwayland_adjust_cursor(ev->surface);
    }

    wlr_cursor_set_surface(cursor, ev->surface, ev->hotspot_x, ev->hotspot_y);

    last_cursor_name.clear();
}

void wf::cursor_t::set_touchscreen_mode(bool enabled)
{
    if (this->touchscreen_mode_active == enabled)
    {
        return;
    }

    this->touchscreen_mode_active = enabled;

    if (enabled)
    {
        LOGD("Touchscreen mode activated, hiding cursor");
        touchscreen_mode_intended = true;
        hide_cursor();

        // Start watchdog timer for recovery option 1
        if (recovery_timeout > 0)
        {
            hide_timestamp_ms = wf::get_current_time();
            cursor_recovery_timer.disconnect();
            cursor_recovery_timer.set_timeout(recovery_timeout, [this] ()
            {
                LOGW("Cursor recovery: watchdog timer fired. touchscreen_mode_intended=",
                    touchscreen_mode_intended, " touchscreen_mode_active=", touchscreen_mode_active);
                check_cursor_recovery();
            });
        }
    } else
    {
        LOGD("Touchscreen mode deactivated, showing cursor");
        touchscreen_mode_intended = false;
        cursor_recovery_timer.disconnect();
        unhide_cursor();
    }
}

/**
 * Initialize the cursor recovery system
 */
void wf::cursor_t::init_cursor_recovery()
{
    // Recovery option 3: Key binding to manually recover cursor
    recovery_binding = [=] (auto)
    {
        LOGW("Cursor recovery key pressed");
        perform_cursor_recovery("manual key binding");
        return true;
    };

    wf::get_core().bindings->add_activator(recovery_key, &recovery_binding);
    LOGI("Cursor recovery system initialized.");
}

/**
 * Check if cursor needs recovery and perform it if necessary
 * Called on every frame to implement recovery option 2 (input event recovery)
 */
void wf::cursor_t::check_cursor_recovery()
{
    if (!is_hidden())
    {
        return;
    }

    // Recovery option 4: Sanity check - reset if ref counter is suspiciously high
    if (hide_ref_counter > sanity_limit)
    {
        LOGW("Cursor recovery: sanity check failed! hide_ref_counter=", hide_ref_counter,
            " exceeds limit=", (int)sanity_limit);
        perform_cursor_recovery("sanity check: ref counter overflow");
        return;
    }

    // Recovery option 1: Watchdog timer - check if cursor has been hidden too long
    if (recovery_timeout > 0 && touchscreen_mode_active && touchscreen_mode_intended)
    {
        int64_t elapsed = wf::get_current_time() - hide_timestamp_ms;
        if (elapsed > recovery_timeout)
        {
            // Cursor has been hidden intentionally but for too long
            // This could indicate a bug where touchscreen mode didn't deactivate properly
            LOGW("Cursor recovery: cursor hidden for ", elapsed, "ms (timeout: ", (int)recovery_timeout, "ms)",
                " - touchscreen mode was intended but may be stuck");
            // Don't auto-recover if it was intentional, but log it
            // The user can use the recovery key if needed
        }
    }

    // Recovery option 2: Force unhide if cursor should be visible but isn't
    // This handles cases where touchscreen_mode_intended was never set
    if (touchscreen_mode_active && !touchscreen_mode_intended)
    {
        LOGW("Cursor recovery: touchscreen mode active but not intended, recovering");
        perform_cursor_recovery("touchscreen mode unintended");
        return;
    }

    // If cursor has been hidden for a very long time (>5x timeout), force recover
    if (recovery_timeout > 0 && hide_ref_counter > 0)
    {
        int64_t elapsed = wf::get_current_time() - hide_timestamp_ms;
        if (elapsed > recovery_timeout * 5)
        {
            LOGW("Cursor recovery: cursor hidden for excessive time (", elapsed, "ms), forcing recovery");
            perform_cursor_recovery("excessive hide time");
            return;
        }
    }
}

/**
 * Perform the actual cursor recovery
 */
void wf::cursor_t::perform_cursor_recovery(const std::string& reason)
{
    LOGW("CURSOR RECOVERY: ", reason);
    LOGW("  Before: hide_ref_counter=", hide_ref_counter,
        " touchscreen_mode_active=", touchscreen_mode_active,
        " touchscreen_mode_intended=", touchscreen_mode_intended);

    force_unhide_cursor();

    LOGW("  After: hide_ref_counter=", hide_ref_counter,
        " touchscreen_mode_active=", touchscreen_mode_active);
}
