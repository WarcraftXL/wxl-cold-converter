using System.Runtime.InteropServices;

namespace WxlConverterUi.Services;

// Mirrors wxl-converter/src/api/Api.hpp exactly -- field order/types must match the native struct
// byte-for-byte. Keep the two in sync by hand; there's no shared source of truth across the ABI.
[StructLayout(LayoutKind.Sequential)]
internal struct NativePathCap
{
    public IntPtr Prefix;
    public uint MaxEdge;
}

[StructLayout(LayoutKind.Sequential)]
internal struct NativeSettings
{
    public IntPtr Input;
    public IntPtr Output;
    public IntPtr Listfile;
    public uint Threads;
    public int WantM2;
    public int WantWmo;
    public int WantAdt;
    public int WantBlp;

    public int AdtHighResHoles;
    public int AdtFixWater;
    public int AdtShowDoodads;
    public int AdtShowWmoObjects;
    public int AdtPreserveWmoScale;
    public int AdtPackExtraTextureLayers;
    public int AdtUvScaleTable;
    public int AdtHeightBlendTable;
    public int AdtPreserveAreaId;
    public int AdtPreserveGroundEffectId;
    public int AdtFixWdtBigAlpha;
    public int AdtConvertWdl;

    public int WmoShowDoodads;
    public int WmoIncludePortals;
    public int WmoIncludeLights;
    public int WmoIncludeLiquid;
    public int WmoIncludeCollision;
    public int WmoNeutralizeVertexColors;

    public int M2AnimationFix;
    public int M2RibbonCompact;
    public int M2ShadowSwingFix;
    public int M2UnwrapAnim;

    public uint BlpDefaultMaxEdge;
    public IntPtr BlpPathCaps;
    public uint BlpPathCapCount;
    public int BlpExemptSideMapsFromPathCaps;
    public int BlpTranscodeToDxt5;
}

// message/label pointers are only valid for the duration of the call -- copy the string out immediately.
[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void NativeLogCallback(IntPtr message);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void NativeScanCallback(ulong found);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void NativeProgressCallback(ulong current, ulong total, int percent, IntPtr label);

internal static class NativeMethods
{
    // Bare filename, not a path: relies on the normal Windows DLL search order, which checks this
    // process's own directory first -- wxl-converter.dll is copied there at build time (see the .csproj).
    private const string DllName = "wxl-converter.dll";

    public const int ResultOk = 0;
    public const int ResultBadInput = 1;
    public const int ResultPartial = 2;
    public const int ResultCancelled = 3;

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void WxlSetCallbacks(NativeLogCallback log, NativeScanCallback scan, NativeProgressCallback progress);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int WxlRun(ref NativeSettings settings);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void WxlCancel();
}
