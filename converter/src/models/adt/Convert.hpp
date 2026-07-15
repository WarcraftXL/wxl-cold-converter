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

// ADT/WDT/WDL pre-conversion: everything wxl-core's host does to a modern terrain tile/map index
// byte-for-byte, run ahead of time to disk. Mirrors wxl-core's host/AdtHost.cpp.
#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "../../core/Listfile.hpp"
#include "AdtMerge.hpp"

namespace wxl::converter::adt
{
    using Options = wxl::modern::adt::Options;

    /** @brief Sets the toggles used by every ConvertRoot() call. Call once before converting a batch. */
    void Configure(const Options& options);

    /**
     * @brief Merges a split source terrain tile (root + _tex0 + _obj0) into one monolithic client tile.
     * @param root      Root .adt bytes.
     * @param tex0      Sibling _tex0.adt bytes, or empty when there is none.
     * @param obj0      Sibling _obj0.adt bytes, or empty when there is none.
     * @param listfile  FileDataID -> path table for MDID/MDDF/MODF resolution, or nullptr to fall back to placeholders.
     * @param name      Tile name, used only for log lines.
     * @param out       Receives the merged image on success.
     * @return True if a merge happened; false when the tile has no split siblings (root already monolithic).
     */
    bool ConvertRoot(std::span<const uint8_t> root, std::span<const uint8_t> tex0, std::span<const uint8_t> obj0,
                      const wxl::converter::core::Listfile* listfile, std::string_view name, std::vector<uint8_t>& out);

    /**
     * @brief Masks a WDT's MPHD flags + MAIN entries to the shape the client reads.
     * @param raw            Source .wdt bytes.
     * @param clearBigAlpha  True when the map's tiles are served as 4-bit merged ADTs (clears MPHD big_alpha).
     * @param out            Receives the masked image on success.
     * @return True if the mask changed anything; false when already client-shaped.
     */
    bool ConvertWdt(std::span<const uint8_t> raw, bool clearBigAlpha, std::vector<uint8_t>& out);

    /**
     * @brief Reshapes a modern low-detail map index (.wdl) to the client layout.
     * @param raw  Source .wdl bytes.
     * @param out  Receives the reshaped image on success.
     * @return True if reshaped; false when already client-shaped.
     */
    bool ConvertWdl(std::span<const uint8_t> raw, std::vector<uint8_t>& out);

    /**
     * @brief Finds the first present tile (has-adt bit set) in a WDT's MAIN grid.
     * @param raw  Source .wdt bytes.
     * @param x    Receives the tile's column.
     * @param y    Receives the tile's row.
     * @return True if a present tile was found.
     */
    bool FirstPresentTile(std::span<const uint8_t> raw, uint32_t& x, uint32_t& y);
}
