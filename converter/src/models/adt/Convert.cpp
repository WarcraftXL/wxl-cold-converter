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

#include "AdtMerge.hpp"
#include "WdlFixup.hpp"
#include "WdtFixup.hpp"

namespace wxl::converter::adt
{
    namespace madt = wxl::modern::adt;

    namespace
    {
        // FileDataID -> path adapter: routes to the CLI's --listfile table via the ResolveCtx user pointer.
        bool ResolveThunk(void* user, uint32_t fileDataId, std::string& outPath)
        {
            const auto* listfile = static_cast<const wxl::converter::core::Listfile*>(user);
            return listfile && listfile->Resolve(fileDataId, outPath);
        }
    }

    void Configure(const Options& options) { madt::Configure(options); }

    bool ConvertRoot(std::span<const uint8_t> root, std::span<const uint8_t> tex0, std::span<const uint8_t> obj0,
                      const wxl::converter::core::Listfile* listfile, std::string_view name, std::vector<uint8_t>& out)
    {
        madt::ResolveCtx rc{ &ResolveThunk, const_cast<wxl::converter::core::Listfile*>(listfile) };
        return madt::MergeSplitAdt(root, tex0, obj0, out, name, rc);
    }

    bool ConvertWdt(std::span<const uint8_t> raw, bool clearBigAlpha, std::vector<uint8_t>& out)
    {
        return madt::FixWdt(raw, clearBigAlpha, out);
    }

    bool ConvertWdl(std::span<const uint8_t> raw, std::vector<uint8_t>& out)
    {
        return madt::FixWdl(raw, out);
    }

    bool FirstPresentTile(std::span<const uint8_t> raw, uint32_t& x, uint32_t& y)
    {
        return madt::FirstPresentTile(raw, x, y);
    }
}
