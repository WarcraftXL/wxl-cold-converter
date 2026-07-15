using System.Runtime.InteropServices;
using WxlConverterUi.Models;

namespace WxlConverterUi.Services;

public sealed record ScanUpdate(ulong Found);
public sealed record EngineProgress(ulong Current, ulong Total, int Percent, string Label);

// Owns the P/Invoke surface of wxl-converter.dll. One instance for the app's lifetime: callbacks are
// registered once at construction (WxlSetCallbacks is documented as "for the process's lifetime, not
// per-run") and the delegate instances are kept as fields so the GC never collects them out from under
// a native thread that might still call back into them.
public sealed class ConverterEngine
{
    public event Action<string>? LogLine;
    public event Action<ScanUpdate>? ScanProgress;
    public event Action<EngineProgress>? ProgressChanged;

    private readonly NativeLogCallback _logCallback;
    private readonly NativeScanCallback _scanCallback;
    private readonly NativeProgressCallback _progressCallback;

    public ConverterEngine()
    {
        _logCallback = OnLog;
        _scanCallback = OnScan;
        _progressCallback = OnProgress;
        NativeMethods.WxlSetCallbacks(_logCallback, _scanCallback, _progressCallback);
    }

    private void OnLog(IntPtr message) => LogLine?.Invoke(Marshal.PtrToStringAnsi(message) ?? string.Empty);
    private void OnScan(ulong found) => ScanProgress?.Invoke(new ScanUpdate(found));
    private void OnProgress(ulong current, ulong total, int percent, IntPtr label) =>
        ProgressChanged?.Invoke(new EngineProgress(current, total, percent, Marshal.PtrToStringAnsi(label) ?? string.Empty));

    /** Blocks the calling thread for the whole batch -- always invoke via Task.Run from a UI thread. */
    public Task<int> RunAsync(AppSettings settings) => Task.Run(() => Run(settings));

    public void Cancel() => NativeMethods.WxlCancel();

    private static int Run(AppSettings settings)
    {
        var allocations = new List<IntPtr>();
        IntPtr Track(IntPtr p) { allocations.Add(p); return p; }
        IntPtr Ansi(string? s) => string.IsNullOrEmpty(s) ? IntPtr.Zero : Track(Marshal.StringToHGlobalAnsi(s));

        try
        {
            var caps = settings.BlpPathCaps.Where(c => !string.IsNullOrWhiteSpace(c.Prefix)).ToArray();
            IntPtr capsPtr = IntPtr.Zero;
            if (caps.Length > 0)
            {
                int capSize = Marshal.SizeOf<NativePathCap>();
                capsPtr = Track(Marshal.AllocHGlobal(capSize * caps.Length));
                for (int i = 0; i < caps.Length; i++)
                {
                    var nativeCap = new NativePathCap { Prefix = Ansi(caps[i].Prefix), MaxEdge = (uint)caps[i].MaxEdge };
                    Marshal.StructureToPtr(nativeCap, capsPtr + i * capSize, false);
                }
            }

            var threads = settings.Threads.Trim();
            var nativeThreads = threads.Equals("auto", StringComparison.OrdinalIgnoreCase) || !uint.TryParse(threads, out var t)
                ? 0u : t;

            var native = new NativeSettings
            {
                Input = Ansi(settings.Input),
                Output = Ansi(settings.Output),
                Listfile = Ansi(settings.Listfile),
                Threads = nativeThreads,
                WantM2 = settings.M2Enabled ? 1 : 0,
                WantWmo = settings.WmoEnabled ? 1 : 0,
                WantAdt = settings.AdtEnabled ? 1 : 0,
                WantBlp = settings.BlpEnabled ? 1 : 0,

                AdtHighResHoles = settings.Adt.HighResHoles ? 1 : 0,
                AdtFixWater = settings.Adt.FixWater ? 1 : 0,
                AdtShowDoodads = settings.Adt.ShowDoodads ? 1 : 0,
                AdtShowWmoObjects = settings.Adt.ShowWmoObjects ? 1 : 0,
                AdtPreserveWmoScale = settings.Adt.PreserveWmoScale ? 1 : 0,
                AdtPackExtraTextureLayers = settings.Adt.PackExtraTextureLayers ? 1 : 0,
                AdtUvScaleTable = settings.Adt.UvScaleTable ? 1 : 0,
                AdtHeightBlendTable = settings.Adt.HeightBlendTable ? 1 : 0,
                AdtPreserveAreaId = settings.Adt.PreserveAreaId ? 1 : 0,
                AdtPreserveGroundEffectId = settings.Adt.PreserveGroundEffectId ? 1 : 0,
                AdtFixWdtBigAlpha = settings.Adt.FixWdtBigAlpha ? 1 : 0,
                AdtConvertWdl = settings.Adt.ConvertWdl ? 1 : 0,

                WmoShowDoodads = settings.Wmo.ShowDoodads ? 1 : 0,
                WmoIncludePortals = settings.Wmo.IncludePortals ? 1 : 0,
                WmoIncludeLights = settings.Wmo.IncludeLights ? 1 : 0,
                WmoIncludeLiquid = settings.Wmo.IncludeLiquid ? 1 : 0,
                WmoIncludeCollision = settings.Wmo.IncludeCollision ? 1 : 0,
                WmoNeutralizeVertexColors = settings.Wmo.NeutralizeVertexColors ? 1 : 0,

                M2AnimationFix = settings.M2.AnimationFix ? 1 : 0,
                M2RibbonCompact = settings.M2.RibbonCompact ? 1 : 0,
                M2ShadowSwingFix = settings.M2.ShadowSwingFix ? 1 : 0,
                M2UnwrapAnim = settings.M2.UnwrapAnim ? 1 : 0,

                BlpDefaultMaxEdge = (uint)settings.BlpDefaultMaxEdge,
                BlpPathCaps = capsPtr,
                BlpPathCapCount = (uint)caps.Length,
                BlpExemptSideMapsFromPathCaps = settings.BlpExemptSideMapsFromPathCaps ? 1 : 0,
                BlpTranscodeToDxt5 = settings.BlpTranscodeToDxt5 ? 1 : 0,
            };

            return NativeMethods.WxlRun(ref native);
        }
        finally
        {
            foreach (var p in allocations) Marshal.FreeHGlobal(p);
        }
    }
}
