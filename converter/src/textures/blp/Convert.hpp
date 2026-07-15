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

// BLP pre-conversion: everything wxl-core's host does to a modern texture byte-for-byte, run ahead of
// time to disk. Mirrors wxl-core's host/textures/blp/BlpHost.cpp, minus the IsModernTexture scoping (a
// live cross-reference from the M2/WMO/ADT resolvers the host tracks per-session): every .blp the
// converter is pointed at is content the user asked to pre-bake, so the cap applies unconditionally
// here rather than only to textures a modern asset was seen to reference.
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace wxl::converter::blp
{
    // One "folder prefix -> larger-edge cap" rule (a [BLP] path_caps entry, see cli/Settings.hpp).
    struct PathCap
    {
        std::string prefix;
        uint32_t maxEdge;
    };

    // Size-cap and transcode tunables for a conversion batch.
    struct Settings
    {
        uint32_t defaultMaxEdge = 1024; // the 32-bit client's tight address space; used when no PathCap matches
        std::vector<PathCap> pathCaps = {{"tileset", 2048}, {"interface", 2048}};
        bool exemptSideMapsFromPathCaps = true; // _h/_s side-maps always use defaultMaxEdge, ignoring pathCaps
        bool transcodeToDxt5 = true; // re-encode uncompressed-BGRA (encoding 3) sources to DXT5; off = passthrough
    };

    /** @brief Sets the size-cap tunables used by every ConvertBlp() call. Call once before converting. */
    void Configure(const Settings& settings);

    /**
     * @brief Re-encodes an uncompressed-BGRA (encoding 3) BLP to DXT5, and caps an oversized texture's
     *        larger edge by dropping its top mip level(s), per the configured Settings (`_h`/`_s` side-maps
     *        always stay at defaultMaxEdge regardless of path).
     * @param name  Asset name, used to pick the size cap (longest-matching PathCap prefix, else
     *              defaultMaxEdge) and for log lines.
     * @param raw   Source .blp bytes.
     * @param out   Receives the converted image on success.
     * @return True if the image was capped and/or transcoded; false when already within budget and not
     *         encoding 3 -- the caller should copy the source bytes through unchanged.
     */
    bool ConvertBlp(std::string_view name, std::span<const uint8_t> raw, std::vector<uint8_t>& out);
}
