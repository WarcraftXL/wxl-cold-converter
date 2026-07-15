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

// WMO pre-conversion: everything wxl-core's host does to a modern WMO root/group byte-for-byte,
// run ahead of time to disk instead of on every serve. Mirrors wxl-core's host/models/wmo/WmoHost.cpp.
#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "../../core/Listfile.hpp"
#include "WmoTranslate.hpp"

namespace wxl::converter::wmo
{
    using Options = wxl::modern::assets::wmo::Options;

    /** @brief Sets the toggles used by every ConvertWmo() call. Call once before converting a batch. */
    void Configure(const Options& options);

    /**
     * @brief Down-converts a WMO root or group file to the client contract.
     *
     * Detects root vs group from the chunk following MVER (MOHD = root, MOGP = group) -- a WMO carries no
     * extension distinction between the two. FileDataID material/doodad references (root only) resolve via
     * `listfile` when given.
     * @param raw       Source .wmo bytes (root or group).
     * @param listfile  FileDataID -> path table for FDID resolution, or nullptr to leave them unresolved.
     * @param out       Receives the converted image on success.
     * @return True if the file was reshaped; false for content already on the client contract or not a
     *         recognizable WMO root/group -- the caller should copy the source bytes through unchanged.
     */
    bool ConvertWmo(std::span<const uint8_t> raw, const wxl::converter::core::Listfile* listfile,
                     std::vector<uint8_t>& out);
}
