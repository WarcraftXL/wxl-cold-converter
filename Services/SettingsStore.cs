using System.IO;
using System.Text.Json;
using WxlConverterUi.Models;

namespace WxlConverterUi.Services;

// Persists AppSettings as JSON next to the UI executable. The user edits settings via the form;
// this file is UI-managed state, not meant for hand-editing.
public static class SettingsStore
{
    private static readonly string FilePath = Path.Combine(AppContext.BaseDirectory, "wxl-converter-ui.settings.json");

    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };

    public static AppSettings Load()
    {
        try
        {
            if (!File.Exists(FilePath)) return new AppSettings();
            var json = File.ReadAllText(FilePath);
            return JsonSerializer.Deserialize<AppSettings>(json, JsonOptions) ?? new AppSettings();
        }
        catch
        {
            return new AppSettings(); // corrupt/unreadable settings file -> start fresh rather than crash on launch
        }
    }

    public static void Save(AppSettings settings)
    {
        var json = JsonSerializer.Serialize(settings, JsonOptions);
        File.WriteAllText(FilePath, json);
    }
}
