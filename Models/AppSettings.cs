namespace WxlConverterUi.Models;

// Everything the UI remembers between launches. No EnginePath -- wxl-converter.dll always ships next
// to this exe (Services/ConverterEngine resolves it by bare filename via the OS's normal DLL search,
// which checks the calling exe's own directory first).
public sealed class AppSettings
{
    public string Input { get; set; } = string.Empty;
    public string Output { get; set; } = string.Empty;
    public string Listfile { get; set; } = string.Empty;
    public string Threads { get; set; } = "auto";

    public bool M2Enabled { get; set; } = true;
    public bool WmoEnabled { get; set; } = true;
    public bool AdtEnabled { get; set; } = true;
    public bool BlpEnabled { get; set; } = true;

    public AdtOptions Adt { get; set; } = new();
    public WmoOptions Wmo { get; set; } = new();
    public M2Options M2 { get; set; } = new();

    public int BlpDefaultMaxEdge { get; set; } = 1024;
    public bool BlpExemptSideMapsFromPathCaps { get; set; } = true;
    public bool BlpTranscodeToDxt5 { get; set; } = true;
    public List<PathCap> BlpPathCaps { get; set; } =
    [
        new PathCap { Prefix = "tileset", MaxEdge = 2048 },
        new PathCap { Prefix = "interface", MaxEdge = 2048 },
    ];
}
