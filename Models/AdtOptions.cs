namespace WxlConverterUi.Models;

public sealed class AdtOptions
{
    // Geometry / Holes
    public bool HighResHoles { get; set; } = true;

    // Water
    public bool FixWater { get; set; } = true;

    // Objects
    public bool ShowDoodads { get; set; } = true;
    public bool ShowWmoObjects { get; set; } = true;
    public bool PreserveWmoScale { get; set; } = true;

    // Texturing
    public bool PackExtraTextureLayers { get; set; } = true;
    public bool UvScaleTable { get; set; } = true;
    public bool HeightBlendTable { get; set; } = true;

    // Identified
    public bool PreserveAreaId { get; set; } = true;
    public bool PreserveGroundEffectId { get; set; } = true;

    // Companion Files
    public bool FixWdtBigAlpha { get; set; } = true;
    public bool ConvertWdl { get; set; } = true;
}
