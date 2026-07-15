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

#include "Api.hpp"

#include "Callbacks.hpp"

#include "cli/Runner.hpp"
#include "cli/Settings.hpp"
#include "core/Cancel.hpp"

namespace
{
    wxl::converter::cli::Settings ToCliSettings(const WxlSettings& s)
    {
        wxl::converter::cli::Settings out;
        out.input = s.input ? s.input : "";
        if (s.output && s.output[0] != '\0') out.output = s.output;
        if (s.listfile && s.listfile[0] != '\0') out.listfile = s.listfile;
        out.threads = s.threads;

        out.wantM2  = s.wantM2  != 0;
        out.wantWmo = s.wantWmo != 0;
        out.wantAdt = s.wantAdt != 0;
        out.wantBlp = s.wantBlp != 0;

        out.adt.highResHoles           = s.adtHighResHoles != 0;
        out.adt.fixWater               = s.adtFixWater != 0;
        out.adt.showDoodads            = s.adtShowDoodads != 0;
        out.adt.showWmoObjects         = s.adtShowWmoObjects != 0;
        out.adt.preserveWmoScale       = s.adtPreserveWmoScale != 0;
        out.adt.packExtraTextureLayers = s.adtPackExtraTextureLayers != 0;
        out.adt.uvScaleTable           = s.adtUvScaleTable != 0;
        out.adt.heightBlendTable       = s.adtHeightBlendTable != 0;
        out.adt.preserveAreaId         = s.adtPreserveAreaId != 0;
        out.adt.preserveGroundEffectId = s.adtPreserveGroundEffectId != 0;
        out.adtFixWdtBigAlpha = s.adtFixWdtBigAlpha != 0;
        out.adtConvertWdl     = s.adtConvertWdl != 0;

        out.wmo.showDoodads            = s.wmoShowDoodads != 0;
        out.wmo.includePortals         = s.wmoIncludePortals != 0;
        out.wmo.includeLights          = s.wmoIncludeLights != 0;
        out.wmo.includeLiquid          = s.wmoIncludeLiquid != 0;
        out.wmo.includeCollision       = s.wmoIncludeCollision != 0;
        out.wmo.neutralizeVertexColors = s.wmoNeutralizeVertexColors != 0;

        out.m2.animationFix   = s.m2AnimationFix != 0;
        out.m2.ribbonCompact  = s.m2RibbonCompact != 0;
        out.m2.shadowSwingFix = s.m2ShadowSwingFix != 0;
        out.m2UnwrapAnim = s.m2UnwrapAnim != 0;

        out.blp.defaultMaxEdge = s.blpDefaultMaxEdge;
        out.blp.exemptSideMapsFromPathCaps = s.blpExemptSideMapsFromPathCaps != 0;
        out.blp.transcodeToDxt5            = s.blpTranscodeToDxt5 != 0;
        if (s.blpPathCaps && s.blpPathCapCount > 0)
        {
            out.blp.pathCaps.clear();
            out.blp.pathCaps.reserve(s.blpPathCapCount);
            for (uint32_t i = 0; i < s.blpPathCapCount; ++i)
            {
                const WxlPathCap& cap = s.blpPathCaps[i];
                if (!cap.prefix || cap.prefix[0] == '\0') continue;
                out.blp.pathCaps.push_back({cap.prefix, cap.maxEdge});
            }
        }
        return out;
    }
}

WXL_API void __cdecl WxlSetCallbacks(WxlLogCallback log, WxlScanCallback scan, WxlProgressCallback progress)
{
    wxl::converter::api::callbacks::Set(log, scan, progress);
}

WXL_API int32_t __cdecl WxlRun(const WxlSettings* settings)
{
    if (!settings || !settings->input || settings->input[0] == '\0') return WXL_RESULT_BAD_INPUT;
    return wxl::converter::cli::Run(ToCliSettings(*settings));
}

WXL_API void __cdecl WxlCancel()
{
    wxl::converter::core::RequestCancel();
}
