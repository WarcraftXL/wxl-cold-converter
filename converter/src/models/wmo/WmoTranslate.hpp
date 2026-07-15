// WMO translate: read a source WMO, emit Client-shaped WMO bytes.
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
#include <vector>

#include "Resolver.hpp"

// Pure bytes -> bytes. Strips source-only chunks, rebuilds MOTX from FileDataID resolution, collapses the
// shader family to the Client's 0..6, fixes MOBA, recomputes group flags. The transform gates on
// chunk/field presence (data-gated), not on a version number. Free functions, span-based, concrete.
namespace wxl::modern::assets::wmo
{
    // User-facing toggles for the translate. Every field defaults to true, so a default-constructed
    // Options enables every optional transform.
    struct Options
    {
        bool showDoodads             = true; // rebuild/emit MODN+MODD (doodad placements); off = empty
        bool includePortals          = true; // keep MOPV/MOPT/MOPR/MOVV/MOVB (portal/culling data)
        bool includeLights           = true; // keep MOLT (point/spot lights)
        bool includeLiquid           = true; // keep MLIQ (group liquid) and its MOGP flag bit
        bool includeCollision        = true; // keep MOBN/MOBR (BSP collision) and their MOGP flag bits
        bool neutralizeVertexColors  = true; // clamp near-white MOCV entries to black (ambient-shaded fix)
    };

    // Sets the toggles used by every TranslateWmoRoot()/TranslateWmoGroup() call. Call once before converting.
    void Configure(const Options& options);

    // Translate a WMO ROOT file. rc resolves FileDataID texture references (source materials) to paths
    // appended to the rebuilt MOTX blob. Returns false (serve raw) when the source is already Client-shaped
    // and needs no change.
    bool TranslateWmoRoot(std::span<const uint8_t> in, const ResolveCtx& rc, std::vector<uint8_t>& out);

    // Translate one WMO GROUP file. Returns false (serve raw) when the group is already Client-shaped (no
    // source-only sub-chunks, no flag/batch mismatch).
    bool TranslateWmoGroup(std::span<const uint8_t> in, std::vector<uint8_t>& out);
}
