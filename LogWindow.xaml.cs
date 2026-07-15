using System.Windows;
using Wpf.Ui.Controls;

namespace WxlConverterUi;

public partial class LogWindow : FluentWindow
{
    public LogWindow(string summary, IEnumerable<string> lines)
    {
        InitializeComponent();
        SummaryText.Text = summary;
        foreach (var line in lines) LogList.Items.Add(line);
        if (LogList.Items.Count > 0) LogList.ScrollIntoView(LogList.Items[^1]);
    }

    private void Close_Click(object sender, RoutedEventArgs e) => Close();
}
