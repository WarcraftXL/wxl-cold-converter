// ADT merge: assemble a split source terrain tile (root + _tex0 + _obj0) into one monolithic Client tile.
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

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "Resolver.hpp"

// ADT merge: pure bytes -> bytes. The Client reads ONE monolithic tile
// (MVER|MHDR|MCIN[256]|map chunks|MCNK[256]); the source tile is split into root + _tex0 + _obj0 and
// merged into that shape here. The transform gates on chunk/field presence, so one path serves any source
// layout. Free functions, span-based, concrete.
namespace wxl::modern::adt
{
    // User-facing toggles for the merge. Every field defaults to true, so a default-constructed
    // Options enables every optional transform.
    struct Options
    {
        bool highResHoles          = true; // keep the hi-res 8x8 hole mask for the hi-res-hole engine patch
        bool fixWater              = true; // translate MH2O liquid types/heights; off = drop water (MH2O omitted)
        bool showDoodads           = true; // emit MDDF (M2 placements)
        bool showWmoObjects        = true; // emit MODF (WMO placements)
        bool preserveWmoScale      = true; // keep MODF's per-instance scale; off = force x1.0 (1024)
        bool packExtraTextureLayers = true; // pack MCLY layers 5-8 into the trailing ATL2 table; off = drop them
        bool uvScaleTable          = true; // emit the trailing ATSC per-texture UV-scale table
        bool heightBlendTable      = true; // emit the trailing ATHB per-texture height-blend table
        bool preserveAreaId        = true; // keep MCNK areaId; off = zero it
        bool preserveGroundEffectId = true; // keep MCLY/ATL2 groundEffect id; off = zero it
    };

    // Sets the toggles used by every MergeSplitAdt() call. Call once before converting a batch.
    void Configure(const Options& options);

    // Merge split root + _tex0 + _obj0 into one monolithic Client tile. `name` is used only for log lines.
    // `rc` resolves asset FileDataIDs to paths when the source references assets by id instead of a name
    // table; pass a null resolver for sources that ship name tables (the legacy-name path needs none).
    bool MergeSplitAdt(std::span<const uint8_t> root, std::span<const uint8_t> tex0,
                       std::span<const uint8_t> obj0, std::vector<uint8_t>& out, std::string_view name,
                       const ResolveCtx& rc);
}
