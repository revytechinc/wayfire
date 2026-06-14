#include "logger.hpp"
#include <wayfire/nonstd/json.hpp>
#include <wayfire/util/log.hpp>
#include <wayfire/option-wrapper.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <algorithm>
#include <cerrno>
#include <cstdlib>

static std::string level_to_string(wf::log::log_level_t level)
{
    switch (level)
    {
      case wf::log::LOG_LEVEL_DEBUG: return "DEBUG";
      case wf::log::LOG_LEVEL_INFO:  return "INFO";
      case wf::log::LOG_LEVEL_WARN:  return "WARN";
      case wf::log::LOG_LEVEL_ERROR: return "ERROR";
      default: return "INFO";
    }
}

static std::string get_timestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto tt  = system_clock::to_time_t(now);
    auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    struct tm buffer;
    localtime_r(&tt, &buffer);

    std::ostringstream out;
    out << std::put_time(&buffer, "%Y-%m-%dT%H:%M:%S.");
    out << std::setfill('0') << std::setw(3) << ms.count();

    // Add timezone offset
    char tzbuf[8];
    if (strftime(tzbuf, sizeof(tzbuf), "%z", &buffer) > 0)
    {
        out << tzbuf;
    }

    return out.str();
}

static std::string get_date_only()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto tt  = system_clock::to_time_t(now);

    struct tm buffer;
    localtime_r(&tt, &buffer);

    std::ostringstream out;
    out << std::put_time(&buffer, "%Y-%m-%d");
    return out.str();
}

static std::string get_basename(const std::string& path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        return path;
    }
    return path.substr(pos + 1);
}

static std::string join_path(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (a.back() == '/') return a + b;
    return a + "/" + b;
}

static bool create_dir_recursive(const std::string& path)
{
    if (path.empty()) return false;

    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }

    auto pos = path.find_last_of('/');
    if (pos != std::string::npos)
    {
        create_dir_recursive(path.substr(0, pos));
    }

    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

static size_t get_file_size(int fd)
{
    struct stat st;
    if (fstat(fd, &st) == 0)
    {
        return st.st_size;
    }
    return 0;
}

static std::string wf_get_data_home()
{
    if (const char *xdg = getenv("XDG_DATA_HOME"))
    {
        return xdg;
    }

    std::string home;
    if (const char *home_env = getenv("HOME"))
    {
        home = home_env;
    } else
    {
        home = "/tmp";
    }

    return join_path(home, ".local/share");
}

// --- wf_logger_t ---

std::string wf::wf_logger_t::get_current_filename() const
{
    std::ostringstream name;
    name << "wayfire-" << current_date;
    if (current_sequence > 0)
    {
        name << ".jsonl." << current_sequence;
    } else
    {
        name << ".jsonl";
    }
    return name.str();
}

std::string wf::wf_logger_t::get_summary_filename(const std::string& date) const
{
    return "wayfire-" + date + "-summary.jsonl";
}

void wf::wf_logger_t::load_config()
{
    try
    {
        enabled = option_wrapper_t<bool>("core/log_file_enabled").value();
    }
    catch (...)
    {
        enabled = false;
    }

    try
    {
        max_size_mb = option_wrapper_t<int>("core/log_max_size_mb").value();
    }
    catch (...)
    {
        max_size_mb = 10;
    }

    try
    {
        retention_days = option_wrapper_t<int>("core/log_retention_days").value();
    }
    catch (...)
    {
        retention_days = 7;
    }

    try
    {
        summary_enabled = option_wrapper_t<bool>("core/log_summary_enabled").value();
    }
    catch (...)
    {
        summary_enabled = true;
    }

    try
    {
        log_dir = option_wrapper_t<std::string>("core/log_directory").value();
        std::string base = wf_get_data_home();
        log_dir = join_path(base, log_dir);
    }
    catch (...)
    {
        log_dir = join_path(wf_get_data_home(), "wayfire/logs");
    }
}

void wf::wf_logger_t::init()
{
    load_config();

    if (!enabled)
    {
        return;
    }

    ensure_log_dir();
    purge_old_logs();

    current_date = get_date_only();
    std::string filename = join_path(log_dir, get_current_filename());
    fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0)
    {
        current_file_size = get_file_size(fd);
    }
    else
    {
        LOGW("wf_logger_t: failed to open log file ", filename, ": ", strerror(errno));
    }

    // Start rotation timer (check every 60 seconds)
    rotation_timer.set_timeout(60000, [this]()
    {
        check_rotation();
        return true; // repeat
    });

    LOGI("wf_logger_t: initialized, log_dir=", log_dir);
}

void wf::wf_logger_t::shutdown()
{
    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }
    rotation_timer.disconnect();
    LOGI("wf_logger_t: shutdown");
}

void wf::wf_logger_t::ensure_log_dir() const
{
    if (!create_dir_recursive(log_dir))
    {
        LOGW("wf_logger_t: failed to create log directory: ", log_dir);
    }
}

void wf::wf_logger_t::serialize_event(const wf_event_t& event, std::string& out) const
{
    json_t obj;
    obj["timestamp"]    = event.timestamp;
    obj["level"]       = event.level;
    obj["reason"]      = event.reason;
    obj["details"]     = event.details;
    obj["component"]    = event.component;
    obj["source_file"] = event.source_file;
    obj["source_line"] = event.source_line;

    if (!event.extra.empty())
    {
        auto extra_obj = json_t::array();
        for (const auto& [k, v] : event.extra)
        {
            json_t item;
            item["key"] = k;
            item["value"] = v;
            extra_obj.append(item);
        }
        obj["extra"] = extra_obj;
    }

    out = obj.serialize();
}

void wf::wf_logger_t::log_event(const wf_event_t& event)
{
    if (!enabled) return;

    std::lock_guard<std::mutex> lock(mutex);

    // Update summary
    summary_data[current_date][event.level][event.reason].count++;
    summary_data[current_date][event.level][event.reason].last_sample = event.details;

    // Serialize and write
    std::string line;
    serialize_event(event, line);
    line += "\n";

    if (fd >= 0)
    {
        ssize_t written = write(fd, line.data(), line.size());
        if (written > 0)
        {
            current_file_size += static_cast<size_t>(written);
        }
    }
}

void wf::wf_logger_t::log_event(wf::log::log_level_t level,
    const std::string& reason, const std::string& details,
    const std::string& component, const std::string& source_file, int source_line)
{
    log_event(level, reason, details, component, source_file, source_line, {});
}

void wf::wf_logger_t::log_event(wf::log::log_level_t level,
    const std::string& reason, const std::string& details,
    const std::string& component, const std::string& source_file, int source_line,
    const std::map<std::string, std::string>& extra)
{
    wf_event_t event;
    event.timestamp    = get_timestamp();
    event.level        = level_to_string(level);
    event.reason       = reason;
    event.details      = details;
    event.component    = component;
    event.source_file  = get_basename(source_file);
    event.source_line  = source_line;
    event.extra        = extra;
    log_event(event);
}

bool wf::wf_logger_t::is_enabled() const
{
    return enabled;
}

void wf::wf_logger_t::check_rotation()
{
    if (!enabled) return;
    std::lock_guard<std::mutex> lock(mutex);
    check_day_change();
    check_rotation_unlocked();
}

void wf::wf_logger_t::check_day_change()
{
    std::string today = get_date_only();
    if (today != current_date)
    {
        write_summary_unlocked(current_date);
        summary_data.erase(current_date);

        current_date = today;
        current_sequence = 0;

        if (fd >= 0) close(fd);
        std::string filename = join_path(log_dir, get_current_filename());
        fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        current_file_size = (fd >= 0) ? get_file_size(fd) : 0;
    }
}

void wf::wf_logger_t::check_rotation_unlocked()
{
    if (max_size_mb > 0)
    {
        size_t max_size = static_cast<size_t>(max_size_mb) * 1024 * 1024;
        if (current_file_size >= max_size)
        {
            if (fd >= 0) close(fd);

            current_sequence++;
            std::string filename = join_path(log_dir, get_current_filename());
            fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            current_file_size = (fd >= 0) ? get_file_size(fd) : 0;
            LOGI("wf_logger_t: rotated to ", filename);
        }
    }
}

void wf::wf_logger_t::write_summary()
{
    if (!enabled) return;
    std::lock_guard<std::mutex> lock(mutex);
    write_summary_unlocked(current_date);
}

void wf::wf_logger_t::write_summary_unlocked(const std::string& date)
{
    if (!summary_enabled) return;

    auto date_it = summary_data.find(date);
    if (date_it == summary_data.end()) return;

    std::string summary_path = join_path(log_dir, get_summary_filename(date));
    int sfdo = open(summary_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (sfdo < 0) return;

    for (const auto& [level, reasons] : date_it->second)
    {
        for (const auto& [reason, entry] : reasons)
        {
            json_t obj;
            obj["date"]   = date;
            obj["level"]  = level;
            obj["reason"] = reason;
            obj["count"]  = entry.count;
            if (!entry.last_sample.empty())
            {
                obj["last_sample"] = entry.last_sample;
            }

            std::string line = obj.serialize() + "\n";
            write(sfdo, line.data(), line.size());
        }
    }

    close(sfdo);
}

void wf::wf_logger_t::purge_old_logs()
{
    if (retention_days <= 0) return;

    std::string prefix = "wayfire-";

    DIR *dir = opendir(log_dir.c_str());
    if (!dir) return;

    auto cutoff_time = time(nullptr) - (retention_days * 24 * 60 * 60);

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;
        if (name.substr(0, prefix.size()) != prefix) continue;

        std::string full_path = join_path(log_dir, name);
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
        {
            if (st.st_mtime < cutoff_time)
            {
                unlink(full_path.c_str());
                LOGI("wf_logger_t: purged old log ", full_path);
            }
        }
    }

    closedir(dir);
}

// --- singleton accessor ---
namespace wf {
wf_logger_t& wf_logger_instance()
{
    static wf_logger_t instance;
    return instance;
}
}
