// Family + sub-kind detection for CLI dispatch: extension first, magic where the extension is ambiguous.
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

namespace wxl::converter::cli
{
    /// Asset family, selected by the --m2/--adt/--wmo/--blp flags and used to filter a folder scan.
    enum class Family
    {
        M2,
        Adt,
        Wmo,
        Blp,
        Unknown,
    };

    /// Sub-kind inside the M2 family: each file type in an M2's sibling set (.m2/.skin/.anim/.skel).
    enum class M2Kind
    {
        Model,   // .m2
        Skin,    // .skin
        Anim,    // .anim
        Skel,    // .skel (consumed by the sibling .m2, never written to output on its own)
        NotM2,
    };

    /// Sub-kind inside the ADT family: a terrain tile's sibling set, plus the map-wide index files.
    enum class AdtKind
    {
        Root,   // <tile>.adt
        Tex0,   // <tile>_tex0.adt (consumed by the sibling root, never written to output on its own)
        Obj0,   // <tile>_obj0.adt (consumed by the sibling root, never written to output on its own)
        Obj1,   // <tile>_obj1.adt (not yet consumed by the merge; dropped, same as wxl-core's host)
        Lod,    // <tile>_lod.adt  (not yet consumed by the merge; dropped, same as wxl-core's host)
        Wdt,    // <map>.wdt
        Wdl,    // <map>.wdl
        NotAdt,
    };

    /**
     * @brief Classifies a file's family from its extension.
     * @param path  File path (extension is read case-insensitively).
     * @return The detected family, or Family::Unknown.
     */
    Family DetectFamily(std::string_view path);

    /**
     * @brief Classifies a file's M2 sub-kind from its extension.
     * @param path  File path (extension is read case-insensitively).
     * @return The detected sub-kind, or M2Kind::NotM2.
     */
    M2Kind DetectM2Kind(std::string_view path);

    /**
     * @brief Classifies a file's ADT sub-kind from its extension.
     * @param path  File path (extension is read case-insensitively).
     * @return The detected sub-kind, or AdtKind::NotAdt (also returned for the modern sibling map
     *         indices _occ/_lgt/_fogs/_mpv/_tex/_obj.wdt, which wxl-core's ADT module never touches).
     */
    AdtKind DetectAdtKind(std::string_view path);
}
