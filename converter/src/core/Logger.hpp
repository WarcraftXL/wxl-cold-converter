// Console log for the CLI. Same interface as wxl-core's core/Logger.hpp so copied shared/ files compile unmodified.
// Copyright (C) 2026 WarcraftXL
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

/// Prints one formatted line per call, prefixed by tag.
namespace wxl::core::log
{
    /**
     * @brief Appends one formatted line to stdout. Always visible, even while info is muted.
     * @param fmt  printf-style format string followed by its arguments.
     */
    void Printf(const char* fmt, ...);

    /**
     * @brief Appends one formatted line to stdout, unless muted (see SetInfoMuted).
     *
     * Per-file chatter (one line per model reshaped) drowns the progress bar during a batch run;
     * the batch loop mutes this for its duration and lets the bar carry that information instead.
     * @param fmt  printf-style format string followed by its arguments.
     */
    void Info(const char* fmt, ...);

    /**
     * @brief Toggles whether Info() actually prints.
     * @param muted  true to suppress Info() output.
     */
    void SetInfoMuted(bool muted);
}

// Record macros. WARN/ERROR always print; INFO is the suppressible, high-frequency kind.
#define WLOG_INFO(...)  ::wxl::core::log::Info(__VA_ARGS__)
#define WLOG_WARN(...)  ::wxl::core::log::Printf(__VA_ARGS__)
#define WLOG_ERROR(...) ::wxl::core::log::Printf(__VA_ARGS__)
