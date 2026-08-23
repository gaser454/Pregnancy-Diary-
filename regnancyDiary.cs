// PregnancyDiary.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

class Note
{
    [JsonPropertyName("id")] public string Id { get; set; }
    [JsonPropertyName("content")] public string Content { get; set; }
    [JsonPropertyName("category")] public string Category { get; set; }
    [JsonPropertyName("date")] public string Date { get; set; }
    [JsonPropertyName("weight")] public double? Weight { get; set; }
    [JsonPropertyName("mood")] public int? Mood { get; set; }
    [JsonPropertyName("created_at")] public string CreatedAt { get; set; }

    public Note() { }
    public Note(string content, string category, string date, double? weight, int? mood)
    {
        Id = Guid.NewGuid().ToString().Substring(0,8);
        Content = content;
        Category = category;
        Date = date ?? DateTime.Now.ToString("yyyy-MM-dd");
        Weight = weight;
        Mood = mood;
        CreatedAt = DateTime.Now.ToString("o");
    }
}

class Diary
{
    private List<Note> notes = new List<Note>();
    private readonly string dataFile = "pregnancy_diary.json";
    private readonly JsonSerializerOptions options = new JsonSerializerOptions { WriteIndented = true };

    public Diary() => Load();

    private void Load()
    {
        if (!File.Exists(dataFile)) return;
        string json = File.ReadAllText(dataFile);
        notes = JsonSerializer.Deserialize<List<Note>>(json) ?? new List<Note>();
    }

    private void Save()
    {
        string json = JsonSerializer.Serialize(notes, options);
        File.WriteAllText(dataFile, json);
    }

    public void Add(string content, string category, string date, double? weight, int? mood)
    {
        var n = new Note(content, category, date, weight, mood);
        notes.Add(n);
        Save();
        Console.WriteLine($"✅ Note added (ID: {n.Id})");
    }

    public void List(string category, string since)
    {
        var filtered = notes;
        if (!string.IsNullOrEmpty(category))
            filtered = filtered.Where(n => n.Category == category).ToList();
        if (!string.IsNullOrEmpty(since))
            filtered = filtered.Where(n => n.Date.CompareTo(since) >= 0).ToList();
        if (!filtered.Any())
        {
            Console.WriteLine("No matching notes.");
            return;
        }
        Console.WriteLine($"\n📋 Pregnancy Diary ({filtered.Count} entries):\n");
        foreach (var n in filtered)
        {
            string weightStr = n.Weight.HasValue ? $" – {n.Weight} kg" : "";
            string moodStr = n.Mood.HasValue ? $" ⭐{new string('⭐', n.Mood.Value-1)}" : "";
            Console.WriteLine($"{n.Date}: [{n.Category}] {n.Content}{weightStr}{moodStr}");
        }
    }

    public void Stats()
    {
        if (!notes.Any())
        {
            Console.WriteLine("No entries yet.");
            return;
        }
        var cats = notes.GroupBy(n => n.Category).ToDictionary(g => g.Key, g => g.Count());
        var moods = notes.Where(n => n.Mood.HasValue).Select(n => n.Mood.Value).ToList();
        var weights = notes.Where(n => n.Weight.HasValue).Select(n => n.Weight.Value).ToList();
        Console.WriteLine("\n📊 Statistics:");
        Console.WriteLine($"  Total entries: {notes.Count}");
        foreach (var kv in cats)
            Console.WriteLine($"  {kv.Key} entries: {kv.Value}");
        if (moods.Any())
        {
            double avg = moods.Average();
            Console.WriteLine($"  Average mood: {avg:F1}/5");
        }
        if (weights.Any())
            Console.WriteLine($"  Latest weight: {weights.Last()} kg");
    }

    public void Search(string term)
    {
        var results = notes.Where(n => n.Content.Contains(term, StringComparison.OrdinalIgnoreCase)).ToList();
        if (!results.Any())
        {
            Console.WriteLine("No matches.");
            return;
        }
        Console.WriteLine($"\n🔍 Found {results.Count} note(s):");
        foreach (var n in results)
            Console.WriteLine($"{n.Date}: [{n.Category}] {n.Content}");
    }

    public void ExportHTML(string filename)
    {
        var html = @"<!DOCTYPE html>
<html><head><title>Pregnancy Diary</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
.note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.meta{color:#999;font-size:0.9em;}
</style></head><body>
<h1>🤰 Pregnancy Diary</h1>
";
        foreach (var n in notes.AsEnumerable().Reverse())
        {
            html += $"<div class=\"note\">\n";
            html += $"<div class=\"meta\">{n.Date} – {n.Category}</div>\n";
            html += $"<div>{n.Content}</div>\n";
            if (n.Weight.HasValue) html += $"<div>Weight: {n.Weight} kg</div>\n";
            if (n.Mood.HasValue) html += $"<div>Mood: {new string('⭐', n.Mood.Value)}</div>\n";
            html += "</div>\n";
        }
        html += "</body></html>";
        File.WriteAllText(filename, html);
        Console.WriteLine($"✅ Exported to {filename}");
    }

    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: PregnancyDiary <command> [options]");
            return;
        }
        var d = new Diary();
        var parsed = ParseArgs(args);
        string cmd = args[0];
        switch (cmd)
        {
            case "add":
                if (args.Length < 2) { Console.WriteLine("add <content> [--category CAT] [--date DATE] [--weight KG] [--mood RATING]"); return; }
                string content = args[1];
                string category = parsed.GetValueOrDefault("category", "general");
                string date = parsed.GetValueOrDefault("date");
                double? weight = parsed.ContainsKey("weight") ? double.Parse(parsed["weight"]) : (double?)null;
                int? mood = parsed.ContainsKey("mood") ? int.Parse(parsed["mood"]) : (int?)null;
                d.Add(content, category, date, weight, mood);
                break;
            case "list":
                d.List(parsed.GetValueOrDefault("category"), parsed.GetValueOrDefault("since"));
                break;
            case "stats":
                d.Stats();
                break;
            case "search":
                if (args.Length < 2) { Console.WriteLine("search <term>"); return; }
                d.Search(args[1]);
                break;
            case "export":
                string filename = parsed.GetValueOrDefault("filename", "diary.html");
                d.ExportHTML(filename);
                break;
            default:
                Console.WriteLine("Unknown command.");
                break;
        }
    }

    static Dictionary<string, string> ParseArgs(string[] args)
    {
        var dict = new Dictionary<string, string>();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i].StartsWith("--") && i + 1 < args.Length)
                dict[args[i].Substring(2)] = args[++i];
        }
        return dict;
    }
}
