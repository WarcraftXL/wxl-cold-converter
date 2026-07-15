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

#include "Runner.hpp"

#include "FileType.hpp"
#include "Progress.hpp"
#include "api/Api.hpp"
#include "core/Cancel.hpp"
#include "core/Listfile.hpp"
#include "core/Logger.hpp"
#include "core/ModuleDirectory.hpp"
#include "models/adt/Convert.hpp"
#include "models/m2/Convert.hpp"
#include "models/wmo/Convert.hpp"
#include "textures/blp/Convert.hpp"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

namespace wxl::converter::cli
{
    namespace fs  = std::filesystem;
    namespace m2  = wxl::converter::m2;
    namespace wmo = wxl::converter::wmo;
    namespace adt = wxl::converter::adt;
    namespace blp = wxl::converter::blp;

    namespace
    {
        // Set once at the top of Run(), read by the per-item Convert* free functions -- they have no direct
        // access to the Settings the batch was started with.
        bool g_unwrapAnim = true;
        bool g_fixWdtBigAlpha = true;
        bool g_convertWdl = true;

        bool ReadFile(const fs::path& path, std::vector<uint8_t>& out)
        {
            std::ifstream in(path, std::ios::binary | std::ios::ate);
            if (!in.is_open()) return false;
            const std::streamsize size = in.tellg();
            if (size < 0) return false;
            in.seekg(0);
            out.resize(static_cast<size_t>(size));
            if (size > 0 && !in.read(reinterpret_cast<char*>(out.data()), size)) return false;
            return true;
        }

        bool WriteFile(const fs::path& path, std::span<const uint8_t> data)
        {
            std::error_code ec;
            fs::create_directories(path.parent_path(), ec);
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            if (!out.is_open()) return false;
            if (!data.empty()) out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
            return out.good();
        }

        // Default output root: a SIBLING of the input, never nested inside it (a nested default lets a
        // lazy directory walk re-discover the files it just wrote and loop forever).
        fs::path DefaultOutputRoot(const fs::path& input)
        {
            if (fs::is_directory(input))
                return input.parent_path() / (input.filename().string() + "_converted");
            const fs::path stem = input.stem();
            const fs::path ext = input.extension();
            return input.parent_path() / (stem.string() + "_converted" + ext.string());
        }

        // Finds a sibling file in the same directory as `m2Path`, same stem, given extension.
        fs::path FindSibling(const fs::path& m2Path, const char* ext)
        {
            fs::path p = m2Path;
            p.replace_extension(ext);
            return p;
        }

        // Finds an ADT split sibling: <root without ".adt"> + suffix (e.g. "_tex0.adt").
        fs::path AdtSibling(const fs::path& rootPath, const char* suffix)
        {
            std::string s = rootPath.string();
            s.resize(s.size() - 4); // drop ".adt"
            s += suffix;
            return fs::path(s);
        }

        // Builds a WDT tile's _tex0 sibling name from the map base (the WDT path without ".wdt") and tile x,y.
        fs::path WdtTileTex0(const fs::path& wdtPath, uint32_t a, uint32_t b)
        {
            std::string s = wdtPath.string();
            s.resize(s.size() - 4); // drop ".wdt"
            s += "_" + std::to_string(a) + "_" + std::to_string(b) + "_tex0.adt";
            return fs::path(s);
        }

        // What a queued work item dispatches as, across every family the converter implements.
        enum class ItemKind { M2Model, M2Skin, M2Anim, M2Skel, WmoFile, AdtRoot, AdtWdt, AdtWdl, BlpFile };

        ItemKind ToItemKind(M2Kind k)
        {
            switch (k)
            {
                case M2Kind::Model: return ItemKind::M2Model;
                case M2Kind::Skin:  return ItemKind::M2Skin;
                case M2Kind::Anim:  return ItemKind::M2Anim;
                default:            return ItemKind::M2Skel;
            }
        }

        struct Counters
        {
            size_t m2Reshaped = 0, m2Passthrough = 0;
            size_t skinCopied = 0;
            size_t animUnwrapped = 0, animPassthrough = 0;
            size_t skelConsumed = 0;
            size_t wmoReshaped = 0, wmoPassthrough = 0, wmoFiltered = 0;
            size_t adtReshaped = 0, adtPassthrough = 0;
            size_t wdtReshaped = 0, wdtPassthrough = 0;
            size_t wdlReshaped = 0, wdlPassthrough = 0;
            size_t adtSiblingConsumed = 0, adtSiblingDropped = 0, adtFiltered = 0;
            size_t blpConverted = 0, blpPassthrough = 0, blpFiltered = 0;
            size_t failed = 0;
        };

        void ConvertM2Item(const fs::path& src, ItemKind kind, const fs::path& dst,
                            const wxl::converter::core::Listfile* listfile, Counters& c)
        {
            if (kind == ItemKind::M2Skel) { ++c.skelConsumed; return; } // absorbed by the sibling .m2, not served

            std::vector<uint8_t> raw;
            if (!ReadFile(src, raw)) { WLOG_ERROR("read failed: %s", src.string().c_str()); ++c.failed; return; }

            std::vector<uint8_t> converted;
            if (kind == ItemKind::M2Model)
            {
                std::vector<uint8_t> skelBytes;
                const fs::path skelPath = FindSibling(src, ".skel");
                std::span<const uint8_t> skelSpan;
                if (fs::exists(skelPath) && ReadFile(skelPath, skelBytes)) skelSpan = skelBytes;

                if (m2::ConvertModel(raw, skelSpan, listfile, converted)) { ++c.m2Reshaped; }
                else { converted = std::move(raw); ++c.m2Passthrough; }
            }
            else if (kind == ItemKind::M2Anim)
            {
                if (g_unwrapAnim && m2::ConvertAnim(raw, converted)) { ++c.animUnwrapped; }
                else { converted = std::move(raw); ++c.animPassthrough; }
            }
            else // Skin: material/texunit contract rebuild needs the live client's parsed skin object
                 // (wxl-core's Skin::Rebuild runs at OnM2SkinFinalize on pointer-fixed engine structures),
                 // so it is not yet a portable offline transform. Served as-is; the client still finalizes it.
            {
                converted = std::move(raw);
                ++c.skinCopied;
            }

            if (!WriteFile(dst, converted)) { WLOG_ERROR("write failed: %s", dst.string().c_str()); ++c.failed; }
        }

        void ConvertWmoItem(const fs::path& src, const fs::path& dst,
                             const wxl::converter::core::Listfile* listfile, Counters& c)
        {
            std::vector<uint8_t> raw;
            if (!ReadFile(src, raw)) { WLOG_ERROR("read failed: %s", src.string().c_str()); ++c.failed; return; }

            std::vector<uint8_t> converted;
            if (wmo::ConvertWmo(raw, listfile, converted)) { ++c.wmoReshaped; }
            else { converted = std::move(raw); ++c.wmoPassthrough; }

            if (!WriteFile(dst, converted)) { WLOG_ERROR("write failed: %s", dst.string().c_str()); ++c.failed; }
        }

        void ConvertAdtRootItem(const fs::path& src, const fs::path& dst,
                                 const wxl::converter::core::Listfile* listfile, Counters& c)
        {
            std::vector<uint8_t> root;
            if (!ReadFile(src, root)) { WLOG_ERROR("read failed: %s", src.string().c_str()); ++c.failed; return; }

            std::vector<uint8_t> tex0, obj0;
            const bool hasTex = fs::exists(AdtSibling(src, "_tex0.adt")) && ReadFile(AdtSibling(src, "_tex0.adt"), tex0);
            const bool hasObj = fs::exists(AdtSibling(src, "_obj0.adt")) && ReadFile(AdtSibling(src, "_obj0.adt"), obj0);

            std::vector<uint8_t> converted;
            if ((hasTex || hasObj) && adt::ConvertRoot(root, tex0, obj0, listfile, src.filename().string(), converted))
                ++c.adtReshaped;
            else { converted = std::move(root); ++c.adtPassthrough; }

            if (!WriteFile(dst, converted)) { WLOG_ERROR("write failed: %s", dst.string().c_str()); ++c.failed; }
        }

        void ConvertWdtItem(const fs::path& src, const fs::path& dst, Counters& c)
        {
            std::vector<uint8_t> raw;
            if (!ReadFile(src, raw)) { WLOG_ERROR("read failed: %s", src.string().c_str()); ++c.failed; return; }

            bool clearBigAlpha = false;
            if (g_fixWdtBigAlpha)
            {
                uint32_t tx = 0, ty = 0;
                if (adt::FirstPresentTile(raw, tx, ty))
                    if (fs::exists(WdtTileTex0(src, tx, ty)) || fs::exists(WdtTileTex0(src, ty, tx)))
                        clearBigAlpha = true;
            }

            std::vector<uint8_t> converted;
            if (adt::ConvertWdt(raw, clearBigAlpha, converted)) { ++c.wdtReshaped; }
            else { converted = std::move(raw); ++c.wdtPassthrough; }

            if (!WriteFile(dst, converted)) { WLOG_ERROR("write failed: %s", dst.string().c_str()); ++c.failed; }
        }

        void ConvertWdlItem(const fs::path& src, const fs::path& dst, Counters& c)
        {
            std::vector<uint8_t> raw;
            if (!ReadFile(src, raw)) { WLOG_ERROR("read failed: %s", src.string().c_str()); ++c.failed; return; }

            std::vector<uint8_t> converted;
            if (g_convertWdl && adt::ConvertWdl(raw, converted)) { ++c.wdlReshaped; }
            else { converted = std::move(raw); ++c.wdlPassthrough; }

            if (!WriteFile(dst, converted)) { WLOG_ERROR("write failed: %s", dst.string().c_str()); ++c.failed; }
        }

        // `relName` is the path used for [BLP] path_caps prefix matching (see textures/blp/Convert.hpp) --
        // it MUST include the leading folder components (e.g. "tileset\Aerie Peaks\..."), not just the bare
        // filename, or a "tileset"/"interface" prefix could never match.
        void ConvertBlpItem(const fs::path& src, const fs::path& dst, const fs::path& relName, Counters& c)
        {
            std::vector<uint8_t> raw;
            if (!ReadFile(src, raw)) { WLOG_ERROR("read failed: %s", src.string().c_str()); ++c.failed; return; }

            std::vector<uint8_t> converted;
            if (blp::ConvertBlp(relName.string(), raw, converted)) { ++c.blpConverted; }
            else { converted = std::move(raw); ++c.blpPassthrough; }

            if (!WriteFile(dst, converted)) { WLOG_ERROR("write failed: %s", dst.string().c_str()); ++c.failed; }
        }

        void ConvertItem(const fs::path& src, ItemKind kind, const fs::path& dst, const fs::path& relName,
                          const wxl::converter::core::Listfile* listfile, Counters& c)
        {
            switch (kind)
            {
                case ItemKind::WmoFile: ConvertWmoItem(src, dst, listfile, c); break;
                case ItemKind::AdtRoot: ConvertAdtRootItem(src, dst, listfile, c); break;
                case ItemKind::AdtWdt:  ConvertWdtItem(src, dst, c); break;
                case ItemKind::AdtWdl:  ConvertWdlItem(src, dst, c); break;
                case ItemKind::BlpFile: ConvertBlpItem(src, dst, relName, c); break;
                default:                ConvertM2Item(src, kind, dst, listfile, c); break;
            }
        }
    }

    int Run(const Settings& settings)
    {
        const fs::path input(settings.input);
        std::error_code ec;
        if (!fs::exists(input, ec)) { WLOG_ERROR("input not found: %s", settings.input.c_str()); return WXL_RESULT_BAD_INPUT; }

        wxl::converter::core::ResetCancel();

        blp::Configure(settings.blp);
        adt::Configure(settings.adt);
        wmo::Configure(settings.wmo);
        m2::Configure(settings.m2);
        g_unwrapAnim = settings.m2UnwrapAnim;
        g_fixWdtBigAlpha = settings.adtFixWdtBigAlpha;
        g_convertWdl = settings.adtConvertWdl;

        wxl::converter::core::Listfile listfile;
        const wxl::converter::core::Listfile* listfilePtr = nullptr;
        if (settings.listfile)
        {
            if (listfile.Load(*settings.listfile)) { listfilePtr = &listfile; WLOG_INFO("listfile: %zu entries loaded (%s)", listfile.Size(), settings.listfile->c_str()); }
            else WLOG_WARN("listfile: failed to load %s, FDID textures will stay unresolved", settings.listfile->c_str());
        }
        else
        {
            // No listfile set: fall back to "listfile.csv" next to the DLL, if one was dropped there.
            const fs::path defaultListfile = wxl::converter::core::ModuleDirectory() / "listfile.csv";
            if (fs::exists(defaultListfile) && listfile.Load(defaultListfile.string()))
            {
                listfilePtr = &listfile;
                WLOG_INFO("listfile: %zu entries loaded (%s)", listfile.Size(), defaultListfile.string().c_str());
            }
        }

        const bool wantM2  = settings.wantM2;
        const bool wantWmo = settings.wantWmo;
        const bool wantAdt = settings.wantAdt;
        const bool wantBlp = settings.wantBlp;

        const fs::path outRoot = settings.output ? fs::path(*settings.output) : DefaultOutputRoot(input);
        Counters c;

        if (fs::is_regular_file(input))
        {
            const M2Kind m2kind = DetectM2Kind(input.string());
            const Family fam = DetectFamily(input.string());
            // No batch root to be relative to in single-file mode; match path_caps against the path as given
            // (a relative invocation like "TILESET\Foo\bar.blp" still matches, an absolute one likely won't).
            if (m2kind != M2Kind::NotM2)
            {
                fs::create_directories(outRoot.parent_path(), ec);
                ConvertItem(input, ToItemKind(m2kind), outRoot, input, listfilePtr, c);
            }
            else if (fam == Family::Wmo)
            {
                fs::create_directories(outRoot.parent_path(), ec);
                ConvertItem(input, ItemKind::WmoFile, outRoot, input, listfilePtr, c);
            }
            else if (fam == Family::Adt)
            {
                const AdtKind ak = DetectAdtKind(input.string());
                ItemKind ik;
                if      (ak == AdtKind::Root) ik = ItemKind::AdtRoot;
                else if (ak == AdtKind::Wdt)  ik = ItemKind::AdtWdt;
                else if (ak == AdtKind::Wdl)  ik = ItemKind::AdtWdl;
                else
                {
                    WLOG_ERROR("%s is a split-tile sibling, not a standalone target (point at the root .adt instead)",
                               settings.input.c_str());
                    return WXL_RESULT_BAD_INPUT;
                }
                fs::create_directories(outRoot.parent_path(), ec);
                ConvertItem(input, ik, outRoot, input, listfilePtr, c);
            }
            else if (fam == Family::Blp)
            {
                fs::create_directories(outRoot.parent_path(), ec);
                ConvertItem(input, ItemKind::BlpFile, outRoot, input, listfilePtr, c);
            }
            else
            {
                WLOG_ERROR("%s is not a recognized asset file (.m2/.skin/.anim/.skel, .wmo, .adt/.wdt/.wdl, .blp)",
                           settings.input.c_str());
                return WXL_RESULT_BAD_INPUT;
            }
        }
        else
        {
            // Snapshot the walk before writing anything: a lazy iterator over a directory being written
            // into can re-discover freshly-written files and loop. (Can't happen with a sibling outRoot,
            // but a user-supplied output= could still land inside `input`, so guard it anyway.)
            std::vector<fs::path> files;
            for (const auto& entry : fs::recursive_directory_iterator(input, ec))
            {
                if (wxl::converter::core::IsCancelRequested()) break;
                if (!entry.is_regular_file()) continue;
                files.push_back(entry.path());
                if (files.size() % 256 == 0) progress::Scan(files.size());
            }
            progress::Scan(files.size());

            // Classify up front: an accurate bar total needs the implemented-family subset, not every file
            // touched by the walk (a filtered-out family, or a consumed/dropped ADT sibling, is counted
            // but never drawn as a bar step).
            struct WorkItem { fs::path src, dst, rel; ItemKind kind; };
            std::vector<WorkItem> work;
            for (const fs::path& src : files)
            {
                if (wxl::converter::core::IsCancelRequested()) break;

                // Skip anything already under outRoot (guards a user-supplied output= landing inside `input`).
                // fs::relative returns an EMPTY path (not an error) when src/outRoot don't share a drive --
                // must treat that as "no relation", not as "yes, inside", or a cross-drive output= silently
                // converts nothing (every input file reads as already-under-outRoot and gets skipped).
                std::error_code relToOutEc;
                const fs::path relToOut = fs::relative(src, outRoot, relToOutEc);
                if (!relToOutEc && !relToOut.empty() && !relToOut.string().starts_with("..")) continue;

                std::error_code relEc;
                const fs::path rel = fs::relative(src, input, relEc);
                if (relEc) continue;
                const fs::path dst = outRoot / rel;

                const M2Kind m2kind = DetectM2Kind(src.string());
                if (m2kind != M2Kind::NotM2)
                {
                    if (wantM2) work.push_back({src, dst, rel, ToItemKind(m2kind)});
                    continue;
                }

                const Family fam = DetectFamily(src.string());
                if (fam == Family::Wmo)
                {
                    if (wantWmo) work.push_back({src, dst, rel, ItemKind::WmoFile});
                    else ++c.wmoFiltered;
                    continue;
                }
                if (fam == Family::Adt)
                {
                    if (!wantAdt) { ++c.adtFiltered; continue; }
                    switch (DetectAdtKind(src.string()))
                    {
                        case AdtKind::Root: work.push_back({src, dst, rel, ItemKind::AdtRoot}); break;
                        case AdtKind::Wdt:  work.push_back({src, dst, rel, ItemKind::AdtWdt}); break;
                        case AdtKind::Wdl:  work.push_back({src, dst, rel, ItemKind::AdtWdl}); break;
                        case AdtKind::Tex0: case AdtKind::Obj0: ++c.adtSiblingConsumed; break;
                        case AdtKind::Obj1: case AdtKind::Lod:  ++c.adtSiblingDropped; break;
                        default: break;
                    }
                    continue;
                }
                if (fam == Family::Blp)
                {
                    if (wantBlp) work.push_back({src, dst, rel, ItemKind::BlpFile});
                    else ++c.blpFiltered;
                    continue;
                }
                // Unknown/unrelated file (sounds, ...): out of scope for this pass, left untouched.
            }

            // One shared pool, work-stolen via an atomic index over the whole (already family-merged)
            // list -- not a pool per family divided out of the thread budget. Splitting up front would
            // idle threads on a batch skewed toward one format (e.g. only BLP enabled, or a tree that's
            // 90% textures); a shared pool keeps every thread busy regardless of mix. Sized from
            // [General] threads= (0/"auto" = hardware_concurrency()).
            const size_t hwThreads = settings.threads != 0 ? settings.threads : std::max<size_t>(1, std::thread::hardware_concurrency());
            const size_t numThreads = std::min(hwThreads, std::max<size_t>(1, work.size()));
            if (numThreads > 1) WLOG_INFO("converting with %zu threads", numThreads);

            wxl::core::log::SetInfoMuted(true); // the bar carries per-file info during a batch run

            std::vector<Counters> threadCounters(numThreads);
            std::atomic<size_t> nextIndex{0};
            std::atomic<size_t> completed{0};

            auto worker = [&](size_t tid)
            {
                for (;;)
                {
                    if (wxl::converter::core::IsCancelRequested()) break;
                    const size_t i = nextIndex.fetch_add(1, std::memory_order_relaxed);
                    if (i >= work.size()) break;
                    ConvertItem(work[i].src, work[i].kind, work[i].dst, work[i].rel, listfilePtr, threadCounters[tid]);
                    const size_t done = completed.fetch_add(1, std::memory_order_relaxed) + 1;
                    progress::Bar(done, work.size(), work[i].src.filename().string());
                }
            };

            std::vector<std::thread> pool;
            pool.reserve(numThreads);
            for (size_t t = 0; t < numThreads; ++t) pool.emplace_back(worker, t);
            for (std::thread& th : pool) th.join();

            for (const Counters& tc : threadCounters)
            {
                c.m2Reshaped += tc.m2Reshaped; c.m2Passthrough += tc.m2Passthrough;
                c.skinCopied += tc.skinCopied;
                c.animUnwrapped += tc.animUnwrapped; c.animPassthrough += tc.animPassthrough;
                c.skelConsumed += tc.skelConsumed;
                c.wmoReshaped += tc.wmoReshaped; c.wmoPassthrough += tc.wmoPassthrough;
                c.adtReshaped += tc.adtReshaped; c.adtPassthrough += tc.adtPassthrough;
                c.wdtReshaped += tc.wdtReshaped; c.wdtPassthrough += tc.wdtPassthrough;
                c.wdlReshaped += tc.wdlReshaped; c.wdlPassthrough += tc.wdlPassthrough;
                c.blpConverted += tc.blpConverted; c.blpPassthrough += tc.blpPassthrough;
                c.failed += tc.failed;
            }

            wxl::core::log::SetInfoMuted(false);
        }

        WLOG_INFO("m2: %zu reshaped, %zu passthrough | skin: %zu copied | anim: %zu unwrapped, %zu passthrough | "
                  "skel: %zu consumed | wmo: %zu reshaped, %zu passthrough (%zu filtered)",
                  c.m2Reshaped, c.m2Passthrough, c.skinCopied, c.animUnwrapped, c.animPassthrough,
                  c.skelConsumed, c.wmoReshaped, c.wmoPassthrough, c.wmoFiltered);
        WLOG_INFO("adt: %zu reshaped, %zu passthrough | wdt: %zu reshaped, %zu passthrough | "
                  "wdl: %zu reshaped, %zu passthrough | adt siblings: %zu consumed, %zu dropped (%zu filtered)",
                  c.adtReshaped, c.adtPassthrough, c.wdtReshaped, c.wdtPassthrough, c.wdlReshaped, c.wdlPassthrough,
                  c.adtSiblingConsumed, c.adtSiblingDropped, c.adtFiltered);
        WLOG_INFO("blp: %zu converted, %zu passthrough (%zu filtered) | failed: %zu",
                  c.blpConverted, c.blpPassthrough, c.blpFiltered, c.failed);
        WLOG_INFO("output: %s", outRoot.string().c_str());

        if (wxl::converter::core::IsCancelRequested())
        {
            WLOG_WARN("cancelled");
            return WXL_RESULT_CANCELLED;
        }
        return c.failed == 0 ? WXL_RESULT_OK : WXL_RESULT_PARTIAL;
    }
}
