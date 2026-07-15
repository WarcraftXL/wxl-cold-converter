using System.Windows;
using System.Windows.Media;
using Wpf.Ui.Appearance;

namespace WxlConverterUi;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        ApplicationAccentColorManager.Apply(Color.FromRgb(0xC9, 0x9B, 0x3F), ApplicationTheme.Dark);
    }
}
