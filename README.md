# WarcraftXL Cold Converter

A Windows GUI that pre-bakes M2, WMO, ADT and BLP assets from modern World of Warcraft formats onto the
byte contract the TLK 3.3.5a client reads natively.

## Cold vs. hot conversion

WarcraftXL's client engine, [wxl-core](https://github.com/WarcraftXL/wxl-core), converts modern assets
**hot** - in memory, on demand, the moment the client loads a model, texture, or terrain tile. That hot
path is what makes modern assets work on the client at all, and it remains required regardless of
whether Cold Converter is used: this tool does not replace wxl-core.

Hot conversion costs CPU time and latency on every load, for every player, every session. Cold Converter
runs the same byte-transform logic ahead of time and writes the converted bytes to disk, so the client's
hot path finds native-shaped data already waiting and has nothing left to do. It's a production
optimization on top of wxl-core, not an alternative to it:

- During active development, wxl-core's hot conversion alone is usually enough - no bake step, fastest
  iteration on changing assets.
- Before shipping a build, pre-baking with Cold Converter removes the hot-conversion cost players would
  otherwise pay on every asset load.

The shared transform sources both paths are built from live in
[wxl-modern-assets](https://github.com/WarcraftXL/wxl-modern-assets) and
[wxl-modern-adt](https://github.com/WarcraftXL/wxl-modern-adt).

## What it converts

- **M2 / Anim** - animation id fixes, ribbon emitters, external `.anim` unwrapping, shadow-swing
  billboard fix.
- **WMO** - doodads, portals/culling, collision, lights, liquid, vertex-color neutralization.
- **ADT** - terrain holes, water, doodad/WMO placements, texture layers (including the trailing ATL2/
  ATSC/ATHB tables), area/ground-effect ids, WDT/WDL companion files.
- **BLP** - DXT5 transcoding, per-folder size caps.

Each family's options can be toggled independently in the GUI.

## Download

Pre-built x86 and x64 builds (self-contained, no .NET install required) are published on the
[Releases](../../releases) page.

## Repository layout

```
/           GUI (WPF / .NET), builds to WarcraftXL-Cold-Converter.exe
converter/  Native DLL (C++ / CMake) the GUI drives - no standalone CLI
```

`converter/`'s shared byte-transform files are kept in sync with wxl-modern-assets and wxl-modern-adt;
a small number of files have intentionally diverged to carry this project's own per-family options layer.

## Building from source

```powershell
converter\build.ps1      # builds converter\build\Release\wxl-converter.dll (x64)
dotnet build -c Release  # builds the GUI, picking up the DLL above automatically
```

Both support building for x86 as well as x64 - see `.github/workflows/build.yml` for the exact flags.

## License

`converter/` is licensed under the GNU General Public License v3.0 (see `converter/LICENSE`).
