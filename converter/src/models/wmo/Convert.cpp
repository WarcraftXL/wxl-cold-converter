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

#include "../../common/Chunk.hpp"
#include "WmoChunks.hpp"
#include "WmoTranslate.hpp"

namespace wxl::converter::wmo
{
    namespace mwmo = wxl::modern::assets::wmo;
    namespace iff  = wxl::modern::assets::common::iff;

    namespace
    {
        // Magic of the chunk that follows MVER: MOHD for a root, MOGP for a group, 0 if unreadable.
        uint32_t SecondChunkMagic(std::span<const uint8_t> raw)
        {
            if (raw.size() < 12 || iff::Rd32(raw.data()) != mwmo::kMVER) return 0;
            const uint32_t mverLen = 8 + iff::Rd32(raw.data() + 4);
            if (mverLen + 8 > raw.size()) return 0;
            return iff::Rd32(raw.data() + mverLen);
        }

        // FileDataID -> path adapter: routes to the CLI's --listfile table via the ResolveCtx user pointer.
        bool ResolveThunk(void* user, uint32_t fileDataId, std::string& outPath)
        {
            const auto* listfile = static_cast<const wxl::converter::core::Listfile*>(user);
            return listfile && listfile->Resolve(fileDataId, outPath);
        }
    }

    void Configure(const Options& options) { mwmo::Configure(options); }

    bool ConvertWmo(std::span<const uint8_t> raw, const wxl::converter::core::Listfile* listfile,
                     std::vector<uint8_t>& out)
    {
        const uint32_t second = SecondChunkMagic(raw);
        if (second == mwmo::kMOHD)
        {
            mwmo::ResolveCtx rc{ &ResolveThunk, const_cast<wxl::converter::core::Listfile*>(listfile) };
            return mwmo::TranslateWmoRoot(raw, rc, out);
        }
        if (second == mwmo::kMOGP)
            return mwmo::TranslateWmoGroup(raw, out);

        return false; // not a recognizable WMO root or group
    }
}
