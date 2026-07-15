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

#include "FileType.hpp"

#include <cctype>

namespace wxl::converter::cli
{
    namespace
    {
        bool EndsWithCI(std::string_view s, std::string_view suffix)
        {
            if (suffix.size() > s.size()) return false;
            const size_t off = s.size() - suffix.size();
            for (size_t i = 0; i < suffix.size(); ++i)
                if (std::tolower(static_cast<unsigned char>(s[off + i])) !=
                    std::tolower(static_cast<unsigned char>(suffix[i]))) return false;
            return true;
        }
    }

    Family DetectFamily(std::string_view path)
    {
        if (DetectM2Kind(path) != M2Kind::NotM2) return Family::M2;
        if (DetectAdtKind(path) != AdtKind::NotAdt) return Family::Adt;
        if (EndsWithCI(path, ".wmo")) return Family::Wmo;
        if (EndsWithCI(path, ".blp")) return Family::Blp;
        return Family::Unknown;
    }

    M2Kind DetectM2Kind(std::string_view path)
    {
        if (EndsWithCI(path, ".m2")) return M2Kind::Model;
        if (EndsWithCI(path, ".skin")) return M2Kind::Skin;
        if (EndsWithCI(path, ".anim")) return M2Kind::Anim;
        if (EndsWithCI(path, ".skel")) return M2Kind::Skel;
        return M2Kind::NotM2;
    }

    AdtKind DetectAdtKind(std::string_view path)
    {
        // The modern sibling map indices carry no terrain data wxl-core's ADT module reads; check them
        // first so they fall through to NotAdt instead of matching the generic ".wdt" case below.
        if (EndsWithCI(path, "_occ.wdt") || EndsWithCI(path, "_lgt.wdt") || EndsWithCI(path, "_fogs.wdt") ||
            EndsWithCI(path, "_mpv.wdt") || EndsWithCI(path, "_tex.wdt") || EndsWithCI(path, "_obj.wdt"))
            return AdtKind::NotAdt;

        if (EndsWithCI(path, "_tex0.adt")) return AdtKind::Tex0;
        if (EndsWithCI(path, "_obj0.adt")) return AdtKind::Obj0;
        if (EndsWithCI(path, "_obj1.adt")) return AdtKind::Obj1;
        if (EndsWithCI(path, "_lod.adt"))  return AdtKind::Lod;
        if (EndsWithCI(path, ".adt"))      return AdtKind::Root;
        if (EndsWithCI(path, ".wdt"))      return AdtKind::Wdt;
        if (EndsWithCI(path, ".wdl"))      return AdtKind::Wdl;
        return AdtKind::NotAdt;
    }
}
