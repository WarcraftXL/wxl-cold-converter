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

#include "Convert.hpp"

#include "BlpTranscode.hpp"

#include <cctype>

namespace wxl::converter::blp
{
    namespace mblp = wxl::modern::assets::textures::blp;

    namespace
    {
        Settings g_settings; // set once via Configure() before any ConvertBlp() call

        bool EndsWithCI(std::string_view s, std::string_view suffix)
        {
            if (suffix.size() > s.size()) return false;
            const size_t off = s.size() - suffix.size();
            for (size_t i = 0; i < suffix.size(); ++i)
                if (std::tolower(static_cast<unsigned char>(s[off + i])) !=
                    std::tolower(static_cast<unsigned char>(suffix[i]))) return false;
            return true;
        }

        bool StartsWithPathCI(std::string_view name, std::string_view prefix)
        {
            if (name.size() < prefix.size() + 1) return false;
            for (size_t i = 0; i < prefix.size(); ++i)
                if (std::tolower(static_cast<unsigned char>(name[i])) !=
                    std::tolower(static_cast<unsigned char>(prefix[i]))) return false;
            return name[prefix.size()] == '\\' || name[prefix.size()] == '/';
        }

        // Larger edge for `name`: the longest-matching configured [BLP] path_caps prefix, else defaultMaxEdge.
        // Longest wins so a more specific override (e.g. "tileset\ironforge") beats a broader one ("tileset").
        uint32_t ResolveMaxEdge(std::string_view name)
        {
            uint32_t best = g_settings.defaultMaxEdge;
            size_t bestPrefixLen = 0;
            for (const PathCap& cap : g_settings.pathCaps)
                if (cap.prefix.size() > bestPrefixLen && StartsWithPathCI(name, cap.prefix))
                {
                    best = cap.maxEdge;
                    bestPrefixLen = cap.prefix.size();
                }
            return best;
        }
    }

    void Configure(const Settings& settings) { g_settings = settings; }

    bool ConvertBlp(std::string_view name, std::span<const uint8_t> raw, std::vector<uint8_t>& out)
    {
        const bool sideMap = EndsWithCI(name, "_h.blp") || EndsWithCI(name, "_s.blp");
        const uint32_t maxEdge = (sideMap && g_settings.exemptSideMapsFromPathCaps)
            ? g_settings.defaultMaxEdge : ResolveMaxEdge(name);

        std::vector<uint8_t> capped;
        std::span<const uint8_t> src = raw;
        const bool didCap = mblp::CapBlpMips(raw, capped, maxEdge);
        if (didCap) src = capped;

        std::vector<uint8_t> transcoded;
        if (g_settings.transcodeToDxt5 && mblp::TranscodeBlp(src, transcoded))
        {
            out = std::move(transcoded);
            return true;
        }
        if (didCap)
        {
            out = std::move(capped);
            return true;
        }
        return false;
    }
}
