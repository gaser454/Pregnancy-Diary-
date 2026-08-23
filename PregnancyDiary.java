// PregnancyDiary.java
import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.util.*;
import com.google.gson.*;

class Note {
    String id;
    String content;
    String category;
    String date;
    Double weight;
    Integer mood;
    String created_at;

    Note() {}
    Note(String content, String category, String date, Double weight, Integer mood) {
        this.id = UUID.randomUUID().toString().substring(0,8);
        this.content = content;
        this.category = category;
        this.date = date != null ? date : LocalDate.now().toString();
        this.weight = weight;
        this.mood = mood;
        this.created_at = Instant.now().toString();
    }
}

public class PregnancyDiary {
    private List<Note> notes = new ArrayList<>();
    private final String dataFile = "pregnancy_diary.json";
    private final Gson gson = new GsonBuilder().setPrettyPrinting().create();

    public PregnancyDiary() { load(); }

    private void load() {
        try {
            Path path = Paths.get(dataFile);
            if (Files.exists(path)) {
                String json = new String(Files.readAllBytes(path));
                Note[] arr = gson.fromJson(json, Note[].class);
                notes = Arrays.asList(arr);
            }
        } catch (Exception e) {}
    }

    private void save() {
        try {
            Files.write(Paths.get(dataFile), gson.toJson(notes).getBytes());
        } catch (Exception e) {}
    }

    public void add(String content, String category, String date, Double weight, Integer mood) {
        Note n = new Note(content, category, date, weight, mood);
        notes.add(n);
        save();
        System.out.printf("✅ Note added (ID: %s)%n", n.id);
    }

    public void list(String category, String since) {
        List<Note> filtered = notes;
        if (category != null && !category.isEmpty()) {
            filtered = new ArrayList<>();
            for (Note n : notes) if (n.category.equals(category)) filtered.add(n);
        }
        if (since != null && !since.isEmpty()) {
            List<Note> temp = new ArrayList<>();
            for (Note n : filtered) if (n.date.compareTo(since) >= 0) temp.add(n);
            filtered = temp;
        }
        if (filtered.isEmpty()) {
            System.out.println("No matching notes.");
            return;
        }
        System.out.printf("\n📋 Pregnancy Diary (%d entries):\n\n", filtered.size());
        for (Note n : filtered) {
            String weightStr = n.weight != null ? String.format(" – %.1f kg", n.weight) : "";
            String moodStr = n.mood != null ? " ⭐" + "⭐".repeat(n.mood-1) : "";
            System.out.printf("%s: [%s] %s%s%s%n", n.date, n.category, n.content, weightStr, moodStr);
        }
    }

    public void stats() {
        if (notes.isEmpty()) {
            System.out.println("No entries yet.");
            return;
        }
        Map<String, Integer> cats = new HashMap<>();
        List<Integer> moods = new ArrayList<>();
        List<Double> weights = new ArrayList<>();
        for (Note n : notes) {
            cats.put(n.category, cats.getOrDefault(n.category, 0) + 1);
            if (n.mood != null) moods.add(n.mood);
            if (n.weight != null) weights.add(n.weight);
        }
        System.out.println("\n📊 Statistics:");
        System.out.printf("  Total entries: %d%n", notes.size());
        for (Map.Entry<String, Integer> e : cats.entrySet()) {
            System.out.printf("  %s entries: %d%n", e.getKey(), e.getValue());
        }
        if (!moods.isEmpty()) {
            double avg = moods.stream().mapToInt(Integer::intValue).average().orElse(0);
            System.out.printf("  Average mood: %.1f/5%n", avg);
        }
        if (!weights.isEmpty()) {
            System.out.printf("  Latest weight: %.1f kg%n", weights.get(weights.size()-1));
        }
    }

    public void search(String term) {
        List<Note> results = new ArrayList<>();
        for (Note n : notes) {
            if (n.content.toLowerCase().contains(term.toLowerCase())) results.add(n);
        }
        if (results.isEmpty()) {
            System.out.println("No matches.");
            return;
        }
        System.out.printf("\n🔍 Found %d note(s):%n", results.size());
        for (Note n : results) {
            System.out.printf("%s: [%s] %s%n", n.date, n.category, n.content);
        }
    }

    public void exportHTML(String filename) throws IOException {
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\n<html><head><title>Pregnancy Diary</title>\n");
        html.append("<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}\n");
        html.append(".note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}\n");
        html.append(".meta{color:#999;font-size:0.9em;}\n");
        html.append("</style></head><body>\n<h1>🤰 Pregnancy Diary</h1>\n");
        for (int i = notes.size()-1; i >= 0; i--) {
            Note n = notes.get(i);
            html.append("<div class=\"note\">\n");
            html.append("<div class=\"meta\">").append(n.date).append(" – ").append(n.category).append("</div>\n");
            html.append("<div>").append(n.content).append("</div>\n");
            if (n.weight != null) html.append("<div>Weight: ").append(n.weight).append(" kg</div>\n");
            if (n.mood != null) html.append("<div>Mood: ").append("⭐".repeat(n.mood)).append("</div>\n");
            html.append("</div>\n");
        }
        html.append("</body></html>");
        Files.write(Paths.get(filename), html.toString().getBytes());
        System.out.printf("✅ Exported to %s%n", filename);
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.out.println("Usage: PregnancyDiary <command> [options]");
            return;
        }
        PregnancyDiary d = new PregnancyDiary();
        String cmd = args[0];
        Map<String, String> params = new HashMap<>();
        for (int i=1; i<args.length; i++) {
            if (args[i].startsWith("--") && i+1 < args.length) {
                params.put(args[i].substring(2), args[++i]);
            }
        }
        switch (cmd) {
            case "add":
                if (args.length < 2) { System.out.println("add <content> [--category CAT] [--date DATE] [--weight KG] [--mood RATING]"); return; }
                String content = args[1];
                String category = params.getOrDefault("category", "general");
                String date = params.get("date");
                Double weight = params.containsKey("weight") ? Double.parseDouble(params.get("weight")) : null;
                Integer mood = params.containsKey("mood") ? Integer.parseInt(params.get("mood")) : null;
                d.add(content, category, date, weight, mood);
                break;
            case "list":
                d.list(params.get("category"), params.get("since"));
                break;
            case "stats":
                d.stats();
                break;
            case "search":
                if (args.length < 2) { System.out.println("search <term>"); return; }
                d.search(args[1]);
                break;
            case "export":
                String filename = params.getOrDefault("filename", "diary.html");
                d.exportHTML(filename);
                break;
            default:
                System.out.println("Unknown command.");
        }
    }
}
