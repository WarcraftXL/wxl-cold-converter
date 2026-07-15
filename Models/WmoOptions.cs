namespace WxlConverterUi.Models;

public sealed class WmoOptions
{
    // Objects
    public bool ShowDoodads { get; set; } = true;

    // Geometry
    public bool IncludePortals { get; set; } = true;
    public bool IncludeCollision { get; set; } = true;

    // Lighting / Water
    public bool IncludeLights { get; set; } = true;
    public bool IncludeLiquid { get; set; } = true;

    // Vertex Colors
    public bool NeutralizeVertexColors { get; set; } = true;
}
