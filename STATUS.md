# Wayfire FreeBSD Port Status

## Last Updated
2026-06-15

## Current State

### Commits on revytechinc/wayfire (pushed)

| Commit | Description |
|--------|-------------|
| `81b8a33` | main: avoid segfault when wlr_backend_autocreate returns null |
| `dfcf0f4` | core: fix init order crash — create bindings before seat |
| `6c1e895` | core/logger: fix early init crash, handle missing config gracefully |
| `d55960d` | submodules: sync index to current working tree SHAs |
| `541286b` | test/platform: fix clock timespec test for FreeBSD CLOCK_MONOTONIC |
| `975610a` | submodules/wlroots: point to fix meson.build after project() |
| `50871b1` | submodules: point to new commits with git remote setup in meson.build |
| `81c5ff3` | .gitmodules: point all submodules to revytechinc forks |
| `fe23152` | wlroots 0.20: add OS platform abstraction + FreeBSD compat |
| `334b665` | subprojects/wlroots: update to platform abstraction commit |

### FreeBSD Ports

All four ports build and install successfully on FreeBSD 15:

| Port | Location | Status |
|------|----------|--------|
| wf-config | `/usr/ports/devel/wf-config` | Built, installed |
| wayfire | `/usr/ports/x11-wm/wayfire` | Built, installed |
| wayfire-plugins-extra | `/usr/ports/x11-wm/wayfire-plugins-extra` | Built, installed |
| wf-shell | `/usr/ports/x11/wf-shell` | Built, installed |

### Known Issues

- **wf-shell work dir**: owned by root after sudo build, must use `sudo rm -rf work*` before rebuild
- **wayfire startup**: requires GPU/DRM device — crashes gracefully with "Failed to open DRM render device" on headless systems
- **wf-shell stage issue**: PAM and xdg config files install to `${PREFIX}/etc/` on FreeBSD (correct) but may need verification on real system with GPU

---

## Pending Work

### Structured Logging System (Plan: `keen-baking-sonnet.md`)

Goal: JSONL file output with rotation, daily summaries, structured event types.

#### Completed
- [x] `src/core/logger.hpp` — `wf_event_t` struct, `wf_logger_t` class, `WF_LOG_EVT()` macro
- [x] `src/core/logger.cpp` — file rotation, JSONL, summary, config
- [x] `subprojects/wf-config/src/log.cpp` — hook into `log_plain()`
- [x] `metadata/core.xml` — Logging group with 6 options (log_file_enabled, log_directory, log_max_size_mb, log_retention_days, log_min_level, log_summary_enabled)
- [x] `meson.build` — include logger.cpp in build

#### In Progress / Pending
- [ ] Migrate `src/core/seat/cursor.cpp` to `WF_LOG_EVT()` with reason strings
- [ ] Migrate `src/main.cpp`, `src/core/core.cpp`, `src/core/plugin-loader.cpp`, `src/output/render-manager.cpp`, `src/render.cpp` to structured logging

#### Key Design
- Reason strings: generic category keys (e.g. `cursor_recovery`, `buffer_error`, `plugin_load_error`)
- Component: inferred from file path (e.g. `cursor.cpp` → `core/seat`)
- Summary: daily `*-summary.jsonl` with counts by reason/level
- Rotation: daily + size-based (configurable MB limit)

---

## Preflight Check System (Discussed, Not Started)

Enterprise-class pre-launch validation. See prior session analysis for full gap list.

### Critical Issues to Address
1. `wlr_allocator_autocreate` returns null — asserted instead of graceful degradation
2. No `XDG_RUNTIME_DIR` pre-validation — silent failure deep in event loop
3. Plugin failures surface only via LOGE — no aggregate startup report
4. No GPU extension checks (GLES, EGL features)

### Proposed Architecture
- `wf_preflight_t` class with phase-based checks
- Structured "preflight" events in logger
- JSON summary to `~/.local/share/wayfire/preflight-YYYY-MM-DD.jsonl`
- Degraded-mode surfacing in UI

---

## FreeBSD Port Maintenance Notes

### wf-config
- WRKSRC fixed: `${WRKDIR}/wf-config-0.11.0`
- GitHub tarball already has git remote setup code — no `move-git-remote.sh` needed

### wayfire
- `disable-wfconfig-tests.sh` post-patch: adds `default_options: ['tests=disabled']` to wf-config subproject
- `insert-compat.sh`: idempotent guard prevents duplicate FreeBSD compat blocks
- compat shim: `compat/linux/input-event-codes.h` for Linux kernel header
- `fix-include.sh`: patches `option-wrapper.hpp` for missing `section.hpp` include

### wayfire-plugins-extra
- WRKSRC: `${WRKDIR}/wayfire-plugins-extra-0.11.0`
- `insert-compat.sh`: idempotent, anchored on `project()` closing paren

### wf-shell
- WRKSRC: `${WRKDIR}/wf-shell-v0.11.0`
- PAM and xdg paths patched via `fix-pam-xdg-paths.py` → `${PREFIX}/etc/`
- gbm/libdrm made optional for stream-chooser

---

## Testing Checklist

- [ ] wayfire starts on real hardware with GPU (freedev001.cloudbsd.org or equivalent)
- [ ] Structured logs appear at `~/.local/share/wayfire/logs/`
- [ ] Log rotation triggers at configured size
- [ ] Daily summary file generated
- [ ] wf-shell panel/dock/background start correctly
- [ ] All 27 plugins load without errors
