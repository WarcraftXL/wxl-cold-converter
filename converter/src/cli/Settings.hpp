// In-memory settings for a conversion run. Populated by src/api/Api.cpp from the WxlSettings C ABI
// struct a caller (the WPF UI) passes across the DLL boundary; the DLL has no CLI/exe form to read
// a config file from.
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

#include "models/adt/Convert.hpp"
#include "models/m2/Convert.hpp"
#include "models/wmo/Convert.hpp"
#include "textures/blp/Convert.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace wxl::converter::cli
{
    struct Settings
    {
        std::string input;                   // file or directory to convert
        std::optional<std::string> output;   // output root; default = sibling "<input>_converted"
        std::optional<std::string> listfile; // FileDataID -> path CSV; default = "listfile.csv" next to the DLL
        size_t threads = 0;                  // 0 = auto (hardware_concurrency)

        bool wantM2  = true;
        bool wantWmo = true;
        bool wantAdt = true;
        bool wantBlp = true;

        wxl::converter::adt::Options adt;
        wxl::converter::wmo::Options wmo;
        wxl::converter::m2::Options  m2;
        wxl::converter::blp::Settings blp;

        // Companion-file toggles, applied directly in Runner.cpp (not routed through a model Options struct).
        bool adtFixWdtBigAlpha = true; // clear a WDT's MPHD big_alpha bit when its tiles are served merged
        bool adtConvertWdl     = true; // reshape .wdl low-detail map indices onto the client layout
        bool m2UnwrapAnim      = true; // strip the AFM2 wrapper from external .anim siblings
    };
}
