# Live-syncs converter/'s shared byte-transform files from WarcraftXL/wxl-modern-assets and
# WarcraftXL/wxl-modern-adt (not committed here, refreshed at build time).
#
# EXPLICIT ALLOWLIST, not exclude-list: AdtMerge/WmoTranslate/Downport/Md21 have diverged (they carry
# the Options/Configure toggle layer) and must never be overwritten. If another file diverges in the
# future, drop it from this list first.
param(
    [Parameter(Mandatory = $true)][string]$ModernAssetsPath,
    [Parameter(Mandatory = $true)][string]$ModernAdtPath,
    [Parameter(Mandatory = $true)][string]$WxlConverterPath
)

$ErrorActionPreference = "Stop"

# (base path variable, source path relative to that repo's root, destination relative to converter/)
$syncMap = @(
    # models/m2 -- Downport.cpp/.hpp and Md21.cpp are EXCLUDED (diverged: Options/Configure layer).
    @("assets", "shared/models/m2/Animations.cpp",   "src/models/m2/Animations.cpp"),
    @("assets", "shared/models/m2/Animations.hpp",   "src/models/m2/Animations.hpp"),
    @("assets", "shared/models/m2/Cameras.cpp",      "src/models/m2/Cameras.cpp"),
    @("assets", "shared/models/m2/Cameras.hpp",      "src/models/m2/Cameras.hpp"),
    @("assets", "shared/models/m2/Contract.hpp",     "src/models/m2/Contract.hpp"),
    @("assets", "shared/models/m2/Md21.hpp",         "src/models/m2/Md21.hpp"),
    @("assets", "shared/models/m2/Particles.cpp",    "src/models/m2/Particles.cpp"),
    @("assets", "shared/models/m2/Particles.hpp",    "src/models/m2/Particles.hpp"),
    @("assets", "shared/models/m2/Ribbons.cpp",      "src/models/m2/Ribbons.cpp"),
    @("assets", "shared/models/m2/Ribbons.hpp",      "src/models/m2/Ribbons.hpp"),
    @("assets", "shared/models/m2/Skel.cpp",         "src/models/m2/Skel.cpp"),
    @("assets", "shared/models/m2/Skel.hpp",         "src/models/m2/Skel.hpp"),
    @("assets", "shared/models/m2/Textures.cpp",     "src/models/m2/Textures.cpp"),
    @("assets", "shared/models/m2/Textures.hpp",     "src/models/m2/Textures.hpp"),

    # models/wmo -- WmoTranslate.cpp/.hpp are EXCLUDED (diverged: Options/Configure layer).
    @("assets", "shared/models/wmo/Resolver.hpp",    "src/models/wmo/Resolver.hpp"),
    @("assets", "shared/models/wmo/WmoChunks.hpp",   "src/models/wmo/WmoChunks.hpp"),
    @("assets", "shared/models/wmo/WmoClient.hpp",   "src/models/wmo/WmoClient.hpp"),
    @("assets", "shared/models/wmo/WmoSource.hpp",   "src/models/wmo/WmoSource.hpp"),

    # textures/blp
    @("assets", "shared/textures/blp/BlpTranscode.cpp", "src/textures/blp/BlpTranscode.cpp"),
    @("assets", "shared/textures/blp/BlpTranscode.hpp", "src/textures/blp/BlpTranscode.hpp"),
    @("assets", "shared/textures/blp/Dxt.hpp",          "src/textures/blp/Dxt.hpp"),

    # common
    @("assets", "shared/common/Chunk.hpp", "src/common/Chunk.hpp"),

    # ADT (flat in wxl-modern-adt) -- AdtMerge.cpp/.hpp are EXCLUDED (diverged: Options/Configure layer).
    @("adt", "shared/AdtExtraLayers.hpp", "src/models/adt/AdtExtraLayers.hpp"),
    @("adt", "shared/AdtTexHeight.hpp",   "src/models/adt/AdtTexHeight.hpp"),
    @("adt", "shared/AdtTexScale.hpp",    "src/models/adt/AdtTexScale.hpp"),
    @("adt", "shared/ChunkIO.hpp",        "src/models/adt/ChunkIO.hpp"),
    @("adt", "shared/Resolver.hpp",       "src/models/adt/Resolver.hpp"),
    @("adt", "shared/WdlFixup.cpp",       "src/models/adt/WdlFixup.cpp"),
    @("adt", "shared/WdlFixup.hpp",       "src/models/adt/WdlFixup.hpp"),
    @("adt", "shared/WdtFixup.cpp",       "src/models/adt/WdtFixup.cpp"),
    @("adt", "shared/WdtFixup.hpp",       "src/models/adt/WdtFixup.hpp")
)

$bases = @{ assets = $ModernAssetsPath; adt = $ModernAdtPath }

$synced = 0
$unchanged = 0
foreach ($entry in $syncMap) {
    $base = $bases[$entry[0]]
    $src = Join-Path $base $entry[1]
    $dst = Join-Path $WxlConverterPath $entry[2]

    if (-not (Test-Path $src)) {
        Write-Error "sync: missing source ($($entry[0])): $($entry[1])"
    }
    if (-not (Test-Path $dst)) {
        Write-Error "sync: missing destination in wxl-converter (renamed/removed?): $($entry[2])"
    }

    $srcHash = (Get-FileHash $src -Algorithm SHA256).Hash
    $dstHash = (Get-FileHash $dst -Algorithm SHA256).Hash
    if ($srcHash -ne $dstHash) {
        Copy-Item $src $dst -Force
        Write-Output "synced:    $($entry[2])"
        $synced++
    } else {
        $unchanged++
    }
}

Write-Output "--- sync-shared-sources: $synced updated, $unchanged already current (of $($syncMap.Count) tracked files) ---"
