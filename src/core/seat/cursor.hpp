#ifndef CURSOR_HPP
#define CURSOR_HPP

#include "seat-impl.hpp"
#include "wayfire/plugin.hpp"
#include "wayfire/signal-definitions.hpp"
#include "wayfire/util.hpp"

namespace wf
{
struct cursor_t
{
    cursor_t(wf::seat_t *seat);
    ~cursor_t() = default;

    /**
     * Register a new input device.
     */
    void add_new_device(wlr_input_device *device);

    /**
     * Set the cursor image from a wlroots event.
     * @param validate_request Whether to validate the request against the
     * currently focused pointer surface, or not.
     */
    void set_cursor(wlr_seat_pointer_request_set_cursor_event *ev,
        bool validate_request);
    void set_cursor(std::string name);
    bool can_client_set_cursor();
    void unhide_cursor();
    void hide_cursor();
    int hide_ref_counter = 0;

    /**
     * Force unhide the cursor, ignoring ref counter.
     * Used for recovery when cursor gets stuck hidden.
     */
    void force_unhide_cursor();

    /**
     * Check if cursor is currently hidden (visible = false) or not.
     */
    bool is_hidden() const;

    /**
     * Delay setting the cursor, in order to avoid setting the cursor
     * multiple times in a single frame and to avoid setting it in the middle
     * of the repaint loop (not allowed by wlroots).
     */
    wf::wl_idle_call idle_set_cursor;

    /**
     * Start/stop touchscreen mode, which means the cursor will be hidden.
     * It will be shown again once a pointer or tablet event happens.
     */
    void set_touchscreen_mode(bool enabled);

    /* Move the cursor to the given point */
    void warp_cursor(wf::pointf_t point);
    wf::pointf_t get_cursor_position();

    void init_xcursor();
    void init_cursor_shape_manager();
    void init_cursor_recovery();
    void setup_listeners();

    // Device event listeners
    wf::wl_listener_wrapper on_button, on_motion, on_motion_absolute, on_axis,

        on_swipe_begin, on_swipe_update, on_swipe_end,
        on_pinch_begin, on_pinch_update, on_pinch_end,
        on_hold_begin, on_hold_end,

        on_tablet_tip, on_tablet_axis,
        on_tablet_button, on_tablet_proximity,
        on_frame;

    // Seat events
    wf::wl_listener_wrapper request_set_cursor;
    wf::wl_listener_wrapper request_set_cursor_shape;

    wf::signal::connection_t<wf::reload_config_signal> config_reloaded;
    wf::seat_t *seat;

    wlr_cursor *cursor = NULL;
    wlr_xcursor_manager *xcursor = NULL;
    wlr_cursor_shape_manager_v1 *cursor_shape_manager = NULL;

    std::string last_cursor_name;

    bool touchscreen_mode_active = false;

    // Cursor recovery system
    bool touchscreen_mode_intended = false;  // Was touchscreen mode intentionally activated?
    int64_t hide_timestamp_ms = 0;         // When cursor was hidden (for watchdog)
    wf::wl_timer<false> cursor_recovery_timer;
    wf::option_wrapper_t<int> recovery_timeout{"input/cursor_recovery_timeout"};
    wf::option_wrapper_t<activatorbinding_t> recovery_key{"input/cursor_recovery_key"};
    wf::option_wrapper_t<int> sanity_limit{"input/cursor_recovery_sanity_limit"};
    wf::activator_callback recovery_binding;

    // Internal recovery methods
    void check_cursor_recovery();
    void perform_cursor_recovery(const std::string& reason);
};

} // namespace wf

#endif /* end of include guard: CURSOR_HPP */
