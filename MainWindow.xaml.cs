using System.Collections.ObjectModel;
using System.Windows;
using Microsoft.Win32;
using Wpf.Ui.Controls;
using WxlConverterUi.Models;
using WxlConverterUi.Services;

namespace WxlConverterUi;

public partial class MainWindow : FluentWindow
{
    private readonly ObservableCollection<PathCap> _pathCaps = [];
    private readonly ConverterEngine _engine = new();
    private readonly List<string> _logLines = [];
    private bool _isRunning;

    public MainWindow()
    {
        InitializeComponent();
        PathCapsList.ItemsSource = _pathCaps;

        _engine.LogLine += line => Dispatcher.Invoke(() => _logLines.Add(line));
        _engine.ScanProgress += s => Dispatcher.Invoke(() => StatusText.Text = $"Scanning... {s.Found} files found");
        _engine.ProgressChanged += p => Dispatcher.Invoke(() =>
        {
            ConvertProgress.Value = p.Percent;
            StatusText.Text = $"{p.Current}/{p.Total} ({p.Percent}%) {p.Label}";
        });

        LoadSettings();
        Closing += (_, _) => SaveSettings();
    }

    private void LoadSettings()
    {
        var s = SettingsStore.Load();
        InputBox.Text = s.Input;
        OutputBox.Text = s.Output;
        ListfileBox.Text = s.Listfile;
        ThreadsBox.Text = s.Threads;
        M2Check.IsChecked = s.M2Enabled;
        WmoCheck.IsChecked = s.WmoEnabled;
        AdtCheck.IsChecked = s.AdtEnabled;
        BlpCheck.IsChecked = s.BlpEnabled;
        BlpDefaultMaxEdgeBox.Text = s.BlpDefaultMaxEdge.ToString();
        BlpExemptSideMapsToggle.IsChecked = s.BlpExemptSideMapsFromPathCaps;
        BlpTranscodeToDxt5Toggle.IsChecked = s.BlpTranscodeToDxt5;

        AdtHighResHolesToggle.IsChecked = s.Adt.HighResHoles;
        AdtFixWaterToggle.IsChecked = s.Adt.FixWater;
        AdtShowDoodadsToggle.IsChecked = s.Adt.ShowDoodads;
        AdtShowWmoObjectsToggle.IsChecked = s.Adt.ShowWmoObjects;
        AdtPreserveWmoScaleToggle.IsChecked = s.Adt.PreserveWmoScale;
        AdtPackExtraTextureLayersToggle.IsChecked = s.Adt.PackExtraTextureLayers;
        AdtUvScaleTableToggle.IsChecked = s.Adt.UvScaleTable;
        AdtHeightBlendTableToggle.IsChecked = s.Adt.HeightBlendTable;
        AdtPreserveAreaIdToggle.IsChecked = s.Adt.PreserveAreaId;
        AdtPreserveGroundEffectIdToggle.IsChecked = s.Adt.PreserveGroundEffectId;
        AdtFixWdtBigAlphaToggle.IsChecked = s.Adt.FixWdtBigAlpha;
        AdtConvertWdlToggle.IsChecked = s.Adt.ConvertWdl;

        WmoShowDoodadsToggle.IsChecked = s.Wmo.ShowDoodads;
        WmoIncludePortalsToggle.IsChecked = s.Wmo.IncludePortals;
        WmoIncludeCollisionToggle.IsChecked = s.Wmo.IncludeCollision;
        WmoIncludeLightsToggle.IsChecked = s.Wmo.IncludeLights;
        WmoIncludeLiquidToggle.IsChecked = s.Wmo.IncludeLiquid;
        WmoNeutralizeVertexColorsToggle.IsChecked = s.Wmo.NeutralizeVertexColors;

        M2AnimationFixToggle.IsChecked = s.M2.AnimationFix;
        M2UnwrapAnimToggle.IsChecked = s.M2.UnwrapAnim;
        M2RibbonCompactToggle.IsChecked = s.M2.RibbonCompact;
        M2ShadowSwingFixToggle.IsChecked = s.M2.ShadowSwingFix;

        _pathCaps.Clear();
        foreach (var cap in s.BlpPathCaps) _pathCaps.Add(cap);

        UpdateFamilyVisibility();
    }

    // Unchecking a family in "Families" hides its whole options section below and lets the remaining
    // cards reflow up to fill the gap (a plain StackPanel does this for free once a child is Collapsed).
    private void FamilyToggle_Changed(object sender, RoutedEventArgs e) => UpdateFamilyVisibility();

    private void UpdateFamilyVisibility()
    {
        // IsChecked="True" in XAML fires Checked while InitializeComponent is still parsing the tree --
        // the Family toggles are declared before the family Cards, so this can run before those fields
        // are assigned. No-op until the whole window is built; LoadSettings() re-syncs afterward.
        if (M2Card == null || WmoCard == null || AdtCard == null || BlpCard == null) return;

        M2Card.Visibility = M2Check.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
        WmoCard.Visibility = WmoCheck.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
        AdtCard.Visibility = AdtCheck.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
        BlpCard.Visibility = BlpCheck.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
    }

    private AppSettings CollectSettings() => new()
    {
        Input = InputBox.Text.Trim(),
        Output = OutputBox.Text.Trim(),
        Listfile = ListfileBox.Text.Trim(),
        Threads = string.IsNullOrWhiteSpace(ThreadsBox.Text) ? "auto" : ThreadsBox.Text.Trim(),
        M2Enabled = M2Check.IsChecked == true,
        WmoEnabled = WmoCheck.IsChecked == true,
        AdtEnabled = AdtCheck.IsChecked == true,
        BlpEnabled = BlpCheck.IsChecked == true,
        BlpDefaultMaxEdge = int.TryParse(BlpDefaultMaxEdgeBox.Text, out var edge) ? edge : 1024,
        BlpExemptSideMapsFromPathCaps = BlpExemptSideMapsToggle.IsChecked == true,
        BlpTranscodeToDxt5 = BlpTranscodeToDxt5Toggle.IsChecked == true,
        BlpPathCaps = [.. _pathCaps],
        Adt = new AdtOptions
        {
            HighResHoles = AdtHighResHolesToggle.IsChecked == true,
            FixWater = AdtFixWaterToggle.IsChecked == true,
            ShowDoodads = AdtShowDoodadsToggle.IsChecked == true,
            ShowWmoObjects = AdtShowWmoObjectsToggle.IsChecked == true,
            PreserveWmoScale = AdtPreserveWmoScaleToggle.IsChecked == true,
            PackExtraTextureLayers = AdtPackExtraTextureLayersToggle.IsChecked == true,
            UvScaleTable = AdtUvScaleTableToggle.IsChecked == true,
            HeightBlendTable = AdtHeightBlendTableToggle.IsChecked == true,
            PreserveAreaId = AdtPreserveAreaIdToggle.IsChecked == true,
            PreserveGroundEffectId = AdtPreserveGroundEffectIdToggle.IsChecked == true,
            FixWdtBigAlpha = AdtFixWdtBigAlphaToggle.IsChecked == true,
            ConvertWdl = AdtConvertWdlToggle.IsChecked == true,
        },
        Wmo = new WmoOptions
        {
            ShowDoodads = WmoShowDoodadsToggle.IsChecked == true,
            IncludePortals = WmoIncludePortalsToggle.IsChecked == true,
            IncludeCollision = WmoIncludeCollisionToggle.IsChecked == true,
            IncludeLights = WmoIncludeLightsToggle.IsChecked == true,
            IncludeLiquid = WmoIncludeLiquidToggle.IsChecked == true,
            NeutralizeVertexColors = WmoNeutralizeVertexColorsToggle.IsChecked == true,
        },
        M2 = new M2Options
        {
            AnimationFix = M2AnimationFixToggle.IsChecked == true,
            UnwrapAnim = M2UnwrapAnimToggle.IsChecked == true,
            RibbonCompact = M2RibbonCompactToggle.IsChecked == true,
            ShadowSwingFix = M2ShadowSwingFixToggle.IsChecked == true,
        },
    };

    private void SaveSettings() => SettingsStore.Save(CollectSettings());

    private void BrowseInputFile_Click(object sender, RoutedEventArgs e)
    {
        var dlg = new OpenFileDialog { Filter = "All files|*.*" };
        if (dlg.ShowDialog() == true) InputBox.Text = dlg.FileName;
    }

    private void BrowseInputFolder_Click(object sender, RoutedEventArgs e)
    {
        var dlg = new OpenFolderDialog();
        if (dlg.ShowDialog() == true) InputBox.Text = dlg.FolderName;
    }

    private void BrowseOutputFolder_Click(object sender, RoutedEventArgs e)
    {
        var dlg = new OpenFolderDialog();
        if (dlg.ShowDialog() == true) OutputBox.Text = dlg.FolderName;
    }

    private void BrowseListfile_Click(object sender, RoutedEventArgs e)
    {
        var dlg = new OpenFileDialog { Filter = "CSV listfile|*.csv|All files|*.*" };
        if (dlg.ShowDialog() == true) ListfileBox.Text = dlg.FileName;
    }

    private void AddPathCap_Click(object sender, RoutedEventArgs e) => _pathCaps.Add(new PathCap { Prefix = "", MaxEdge = 1024 });

    private void RemovePathCap_Click(object sender, RoutedEventArgs e)
    {
        if (sender is FrameworkElement { Tag: PathCap cap }) _pathCaps.Remove(cap);
    }

    private async void Convert_Click(object sender, RoutedEventArgs e)
    {
        var settings = CollectSettings();

        if (string.IsNullOrWhiteSpace(settings.Input))
        {
            System.Windows.MessageBox.Show(this, "Set an input file or folder first.", "wxl-converter",
                System.Windows.MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        SaveSettings();
        _logLines.Clear();
        ConvertProgress.Value = 0;
        StatusText.Text = "Starting...";
        ConvertButton.IsEnabled = false;
        CancelButton.IsEnabled = true;
        _isRunning = true;

        string summary;
        try
        {
            var exitCode = await _engine.RunAsync(settings);
            summary = exitCode switch
            {
                NativeMethods.ResultOk => "Done.",
                NativeMethods.ResultCancelled => "Cancelled.",
                NativeMethods.ResultBadInput => "Bad input -- see log.",
                _ => $"Finished with exit code {exitCode}.",
            };
            StatusText.Text = summary;
        }
        catch (Exception ex)
        {
            _logLines.Add($"error: {ex.Message}");
            summary = "Failed to run the engine.";
            StatusText.Text = summary;
        }
        finally
        {
            ConvertButton.IsEnabled = true;
            CancelButton.IsEnabled = false;
            _isRunning = false;
        }

        var logWindow = new LogWindow(summary, _logLines) { Owner = this };
        logWindow.Show();
    }

    private void Cancel_Click(object sender, RoutedEventArgs e)
    {
        if (!_isRunning) return;
        _engine.Cancel();
        StatusText.Text = "Cancelling...";
    }
}
