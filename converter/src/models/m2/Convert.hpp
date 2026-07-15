// M2-family pre-conversion: everything the wxl-core host does to a modern M2 byte-for-byte, run
// ahead of time to disk instead of on every serve. Mirrors wxl-core's host/models/m2/Register.cpp.
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

#include "../../core/Listfile.hpp"
#include "Downport.hpp"

namespace wxl::converter::m2
{
    // User-facing toggles for the M2 pipeline. animationFix/ribbonCompact forward to the shared downport
    // module; useListfileTextures/shadowSwingFix are converter-local (see Convert.cpp).
    struct Options
    {
        bool animationFix       = true; // remap sequence ids the client can't resolve to a safe fallback
        bool ribbonCompact      = true; // downport ribbon emitter records
        bool shadowSwingFix     = true; // zero the bone lookup on static-prop shadow-swing rigs (billboard fix)
    };

    /** @brief Sets the toggles used by every ConvertModel() call. Call once before converting a batch. */
    void Configure(const Options& options);

    /**
     * @brief Reshapes a source .m2 (bare MD20 or MD21 chunked container) onto the client contract.
     *
     * Dechunks an MD21 container (inlining TXID FileDataID textures via `listfile` when given), splices
     * in `skelBytes` when the model is a split-skeleton source, then runs the shared downport (cameras/
     * particles/ribbons/animations/textures) and the billboard boneCombos fix.
     * @param raw        Source .m2 bytes.
     * @param skelBytes  Sibling .skel bytes to splice in, or empty when there is none.
     * @param listfile   FileDataID -> path table for TXID resolution, or nullptr to leave FDID textures unresolved.
     * @param out        Receives the reshaped image on success.
     * @return True if the model was reshaped; false for content already on the client contract (out is
     *         left empty -- the caller should copy the source bytes through unchanged).
     */
    bool ConvertModel(std::span<const uint8_t> raw, std::span<const uint8_t> skelBytes,
                       const wxl::converter::core::Listfile* listfile, std::vector<uint8_t>& out);

    /**
     * @brief Strips the AFM2 chunk wrapper from a modern external animation file.
     * @param raw  Source .anim bytes.
     * @param out  Receives the unwrapped payload on success.
     * @return True if the file was wrapped and got unwrapped; false when already client-shaped.
     */
    bool ConvertAnim(std::span<const uint8_t> raw, std::vector<uint8_t>& out);
}
