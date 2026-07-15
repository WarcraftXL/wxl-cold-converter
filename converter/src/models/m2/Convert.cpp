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

#include "core/Logger.hpp"
#include "structure/m2/M2Format.hpp"

#include "Downport.hpp"
#include "Md21.hpp"
#include "Skel.hpp"

#include <cstring>

namespace wxl::converter::m2
{
    namespace dp   = wxl::modern::assets::m2::downport;
    namespace m21  = wxl::modern::assets::m2::md21;
    namespace skel = wxl::modern::assets::m2::skel;
    namespace fmt  = wxl::structure::m2;

    namespace
    {
        // Bound to the CLI's --listfile table (or nullptr) so md21::Dechunk can inline TXID textures.
        thread_local const wxl::converter::core::Listfile* g_listfile = nullptr;

        bool ResolveThunk(uint32_t fileDataId, std::string& outPath)
        {
            return g_listfile && g_listfile->Resolve(fileDataId, outPath);
        }

        Options g_opts; // set once via Configure() before any ConvertModel() call
    }

    void Configure(const Options& options)
    {
        g_opts = options;
        dp::Configure({ options.animationFix, options.ribbonCompact });
    }

    bool ConvertModel(std::span<const uint8_t> raw, std::span<const uint8_t> skelBytes,
                       const wxl::converter::core::Listfile* listfile, std::vector<uint8_t>& out)
    {
        g_listfile = listfile;

        if (m21::IsMd21(raw))
        {
            std::vector<uint8_t> md20;
            if (!m21::Dechunk(raw, &ResolveThunk, md20)) return false;

            if (!skelBytes.empty())
                skel::Merge(skelBytes, md20);

            const uint32_t orig = static_cast<uint32_t>(md20.size());
            const uint32_t work = dp::WorkSize(md20.data(), orig);
            md20.resize(work);
            if (!dp::ProcessInPlace(md20.data(), orig, work)) return false;
            if (g_opts.shadowSwingFix) m21::ZeroBoneLookup(md20.data(), static_cast<uint32_t>(md20.size()));
            out = std::move(md20);
            return true;
        }

        const uint32_t size = static_cast<uint32_t>(raw.size());
        if (!dp::IsConvertible(raw.data(), size)) return false;

        const uint32_t workSize = dp::WorkSize(raw.data(), size);
        out.resize(workSize);
        std::memcpy(out.data(), raw.data(), size);
        if (!dp::ProcessInPlace(out.data(), size, workSize)) { out.clear(); return false; }
        return true;
    }

    bool ConvertAnim(std::span<const uint8_t> raw, std::vector<uint8_t>& out)
    {
        if (raw.size() < 8) return false;
        uint32_t magic;
        std::memcpy(&magic, raw.data(), 4);
        if (magic != fmt::kMagicAFM2) return false; // already client-shaped

        uint32_t payload;
        std::memcpy(&payload, raw.data() + 4, 4);
        if (8 + size_t(payload) > raw.size()) return false;

        out.assign(raw.data() + 8, raw.data() + 8 + payload);
        return true;
    }
}
