// Public C ABI: everything a caller (the WPF UI, or any other future host) links against. Plain C
// types only -- no STL across the boundary -- so this header is safe to hand-translate into a P/Invoke
// declaration on the managed side.
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

#if defined(_WIN32)
    #define WXL_API extern "C" __declspec(dllexport)
#else
    #define WXL_API extern "C"
#endif

// One [BLP] "prefix:maxEdge" rule -- see textures/blp/Convert.hpp's PathCap for the semantics
// (longest-matching prefix wins).
struct WxlPathCap
{
    const char* prefix;   // null-terminated, e.g. "tileset"
    uint32_t    maxEdge;
};

// Everything a conversion run needs. All pointers are borrowed for the duration of WxlRun() only --
// the DLL never retains them past that call returning.
struct WxlSettings
{
    const char* input;    // required, null-terminated: file or directory to convert
    const char* output;   // nullable: output root; null/empty = sibling "<input>_converted"
    const char* listfile; // nullable: FileDataID -> path CSV; null/empty = "listfile.csv" next to the DLL
    uint32_t    threads;  // 0 = auto (hardware_concurrency)

    int32_t wantM2;
    int32_t wantWmo;
    int32_t wantAdt;
    int32_t wantBlp;

    // ADT: Geometry/Holes, Water, Objects, Texturing, Identified.
    int32_t adtHighResHoles;
    int32_t adtFixWater;
    int32_t adtShowDoodads;
    int32_t adtShowWmoObjects;
    int32_t adtPreserveWmoScale;
    int32_t adtPackExtraTextureLayers;
    int32_t adtUvScaleTable;
    int32_t adtHeightBlendTable;
    int32_t adtPreserveAreaId;
    int32_t adtPreserveGroundEffectId;
    // ADT: Companion Files.
    int32_t adtFixWdtBigAlpha;
    int32_t adtConvertWdl;

    // WMO.
    int32_t wmoShowDoodads;
    int32_t wmoIncludePortals;
    int32_t wmoIncludeLights;
    int32_t wmoIncludeLiquid;
    int32_t wmoIncludeCollision;
    int32_t wmoNeutralizeVertexColors;

    // M2 (+ Anim).
    int32_t m2AnimationFix;
    int32_t m2RibbonCompact;
    int32_t m2ShadowSwingFix;
    int32_t m2UnwrapAnim;

    // BLP.
    uint32_t           blpDefaultMaxEdge;
    const WxlPathCap*  blpPathCaps;
    uint32_t           blpPathCapCount;
    int32_t            blpExemptSideMapsFromPathCaps;
    int32_t            blpTranscodeToDxt5;
};

// message is only valid for the duration of the call -- copy it if you need to keep it.
typedef void(__cdecl* WxlLogCallback)(const char* message);
// found: files discovered so far by the directory walk.
typedef void(__cdecl* WxlScanCallback)(uint64_t found);
// label is only valid for the duration of the call -- copy it if you need to keep it.
typedef void(__cdecl* WxlProgressCallback)(uint64_t current, uint64_t total, int32_t percent, const char* label);

// Run() return codes.
#define WXL_RESULT_OK        0 // completed, nothing failed
#define WXL_RESULT_BAD_INPUT 1 // input path missing/unrecognized -- nothing was converted
#define WXL_RESULT_PARTIAL   2 // completed, but at least one item failed to read/write
#define WXL_RESULT_CANCELLED 3 // stopped early by WxlCancel(); counts reflect partial progress

/**
 * @brief Registers (or clears, by passing nullptr) the callbacks used for the rest of the process's
 *        lifetime -- not per-run. Call once at startup before the first WxlRun().
 */
WXL_API void __cdecl WxlSetCallbacks(WxlLogCallback log, WxlScanCallback scan, WxlProgressCallback progress);

/**
 * @brief Runs one conversion synchronously (blocks the calling thread for the whole batch) --
 *        callers driving a UI should invoke this from a background thread/Task.
 * @param settings  Borrowed for the duration of this call only.
 * @return One of the WXL_RESULT_* codes above.
 */
WXL_API int32_t __cdecl WxlRun(const WxlSettings* settings);

/**
 * @brief Requests the current (or next) WxlRun() to stop early. Safe to call from any thread,
 *        including while WxlRun() is running on another thread. Cooperative: the engine checks this
 *        at its scan/classify/convert loop boundaries, so it may take a moment to actually stop.
 */
WXL_API void __cdecl WxlCancel();
