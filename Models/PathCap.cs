namespace WxlConverterUi.Models;

// One "folder prefix -> larger BLP edge cap" rule, written as a single "prefix:maxEdge" entry in the
// engine's wxl-converter.conf [BLP] path_caps list.
public sealed class PathCap
{
    public string Prefix { get; set; } = string.Empty;
    public int MaxEdge { get; set; } = 2048;
}
