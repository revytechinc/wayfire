#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <wayfire/util.hpp>
#include <wayfire/util/log.hpp>
#include <string>
#include <map>
#include <mutex>

namespace wf
{

/**
 * A structured log event with a generic reason for categorization.
 */
struct wf_event_t
{
    /** ISO 8601 timestamp with milliseconds and timezone */
    std::string timestamp;
    /** Log level: "DEBUG", "INFO", "WARN", "ERROR" */
    std::string level;
    /** Generic reason/category for aggregation, e.g. "cursor_recovery", "buffer_error" */
    std::string reason;
    /** Human-readable details (original message) */
    std::string details;
    /** Component/subsystem, e.g. "core/seat", "render", "plugin-loader" */
    std::string component;
    /** Source file name (basename only) */
    std::string source_file;
    /** Source line number */
    int source_line = 0;
    /** Optional key-value context */
    std::map<std::string, std::string> extra;
};

/**
 * Singleton structured logger that writes JSONL files with rotation.
 *
 * - JSONL output with one event per line
 * - Rotation by day and/or size
 * - Weekly retention with automatic purge
 * - Daily summary file with event counts by reason
 */
class wf_logger_t
{
  public:
    wf_logger_t() = default;
    ~wf_logger_t() = default;

    wf_logger_t(const wf_logger_t&) = delete;
    wf_logger_t(wf_logger_t&&) = delete;
    wf_logger_t& operator =(const wf_logger_t&) = delete;
    wf_logger_t& operator =(wf_logger_t&&) = delete;

    /**
     * Initialize the structured logger from config.
     * Safe to call even if logging is disabled.
     */
    void init();

    /**
     * Shutdown the logger, flushing any pending data.
     */
    void shutdown();

    /**
     * Log a structured event.
     */
    void log_event(const wf_event_t& event);

    /**
     * Convenience overload.
     */
    void log_event(wf::log::log_level_t level, const std::string& reason,
        const std::string& details, const std::string& component,
        const std::string& source_file, int source_line);

    /**
     * Convenience overload with extra context.
     */
    void log_event(wf::log::log_level_t level, const std::string& reason,
        const std::string& details, const std::string& component,
        const std::string& source_file, int source_line,
        const std::map<std::string, std::string>& extra);

    /** Check if structured file logging is enabled */
    bool is_enabled() const;

  private:
    void ensure_log_dir() const;
    std::string get_log_dir() const;
    std::string get_current_filename() const;
    std::string get_summary_filename(const std::string& date) const;
    void check_rotation();
    void check_day_change();
    void check_rotation_unlocked();
    void purge_old_logs();
    void write_summary();
    void write_summary_unlocked(const std::string& date);
    void serialize_event(const wf_event_t& event, std::string& out) const;
    void load_config();

    bool enabled = false;
    std::string log_dir;
    std::string current_date;
    int fd = -1;
    size_t current_file_size = 0;
    int current_sequence = 0;
    int max_size_mb = 10;
    int retention_days = 7;
    bool summary_enabled = true;
    wf::log::log_level_t min_level = wf::log::LOG_LEVEL_INFO;

    mutable std::mutex mutex;

    // Summary data keyed by date -> level -> reason -> {count, sample_details}
    struct summary_entry_t
    {
        int count = 0;
        std::string last_sample;
    };
    std::map<std::string, std::map<std::string, std::map<std::string, summary_entry_t>>> summary_data;

    wf::wl_timer<false> rotation_timer;
};

/**
 * Get the singleton instance of the structured logger.
 */
wf_logger_t& wf_logger_instance();

} // namespace wf

/**
 * Structured logging macro: logs to both stdout and structured JSONL file.
 *
 * Usage:
 *   #define COMPONENT "core/seat"
 *   WF_LOG_EVT(LOG_LEVEL_WARN, "cursor_recovery", "touchscreen mode unintended");
 *   WF_LOG_EVT(LOG_LEVEL_WARN, "sanity_check_failed", "hide_ref_counter=", count, " exceeds limit=", limit);
 *
 * @param level   wf::log::log_level_t value
 * @param reason  Generic reason string for categorization
 * @param details Human-readable details (message body) - also used as format string
 * @param ...     Optional: format args for LOG(), all prefixed with their format key
 */
#define WF_LOG_EVT(level, reason, details, ...) \
    do { \
        LOG(level, details, ##__VA_ARGS__); \
        wf::wf_logger_instance().log_event(level, reason, details, COMPONENT, __FILE__, __LINE__); \
    } while (0)

/**
 * Structured logging macro with extra context key-values.
 */
#define WF_LOG_EVT_EXT(level, reason, details, extra, ...) \
    do { \
        LOG(level, __VA_ARGS__); \
        wf::wf_logger_instance().log_event(level, reason, details, COMPONENT, __FILE__, __LINE__, extra); \
    } while (0)

#endif /* end of include guard: LOGGER_HPP */
