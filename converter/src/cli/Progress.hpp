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

// Thin wrapper over api::callbacks::EmitScan/EmitProgress -- Runner.cpp reports through this instead
// of calling the callback dispatch directly, so it stays decoupled from the ABI layer.
#pragma once

#include <cstddef>
#include <string_view>

namespace wxl::converter::cli::progress
{
    /**
     * @brief Reports files discovered so far during the directory walk.
     * @param found  Files discovered so far.
     */
    void Scan(size_t found);

    /**
     * @brief Reports conversion progress for one completed item.
     * @param current  Items completed so far (1-based).
     * @param total    Total items to convert.
     * @param label    Short label for the item (e.g. a file name).
     */
    void Bar(size_t current, size_t total, std::string_view label);
}
