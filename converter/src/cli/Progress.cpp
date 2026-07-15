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

#include "Progress.hpp"

#include "api/Callbacks.hpp"

#include <string>

namespace wxl::converter::cli::progress
{
    void Scan(size_t found)
    {
        wxl::converter::api::callbacks::EmitScan(static_cast<uint64_t>(found));
    }

    void Bar(size_t current, size_t total, std::string_view label)
    {
        const int percent = total ? static_cast<int>((current * 100 + total / 2) / total) : 100;
        const std::string labelStr(label); // guarantee a null terminator for the C callback
        wxl::converter::api::callbacks::EmitProgress(
            static_cast<uint64_t>(current), static_cast<uint64_t>(total), percent, labelStr.c_str());
    }
}
