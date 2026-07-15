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

#include "Callbacks.hpp"

#include <mutex>

namespace wxl::converter::api::callbacks
{
    namespace
    {
        std::mutex g_mutex; // serializes registration against concurrent Emit* calls from worker threads
        WxlLogCallback      g_log      = nullptr;
        WxlScanCallback     g_scan     = nullptr;
        WxlProgressCallback g_progress = nullptr;
    }

    void Set(WxlLogCallback log, WxlScanCallback scan, WxlProgressCallback progress)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_log = log;
        g_scan = scan;
        g_progress = progress;
    }

    void EmitLog(const char* message)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_log) g_log(message);
    }

    void EmitScan(uint64_t found)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_scan) g_scan(found);
    }

    void EmitProgress(uint64_t current, uint64_t total, int32_t percent, const char* label)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_progress) g_progress(current, total, percent, label);
    }
}
