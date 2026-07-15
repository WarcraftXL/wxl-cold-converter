// Internal callback storage/dispatch -- not part of the public ABI. Logger.cpp and cli/Progress.cpp
// emit through here; Api.cpp is the only thing that calls Set().
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

#include "Api.hpp"

namespace wxl::converter::api::callbacks
{
    void Set(WxlLogCallback log, WxlScanCallback scan, WxlProgressCallback progress);

    // No-ops (thread-safe) when the corresponding callback hasn't been registered.
    void EmitLog(const char* message);
    void EmitScan(uint64_t found);
    void EmitProgress(uint64_t current, uint64_t total, int32_t percent, const char* label);
}
