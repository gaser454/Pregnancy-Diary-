// pregnancy_diary.go
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"time"
	"github.com/google/uuid"
)

type Note struct {
	ID        string  `json:"id"`
	Content   string  `json:"content"`
	Category  string  `json:"category"`
	Date      string  `json:"date"`
	Weight    float64 `json:"weight,omitempty"`
	Mood      int     `json:"mood,omitempty"`
	CreatedAt string  `json:"created_at"`
}

type Diary struct {
	Notes []Note `json:"notes"`
}

var dataFile = "pregnancy_diary.json"

func (d *Diary) load() {
	data, err := os.ReadFile(dataFile)
	if err != nil {
		return
	}
	json.Unmarshal(data, d)
}

func (d *Diary) save() {
	data, _ := json.MarshalIndent(d, "", "  ")
	os.WriteFile(dataFile, data, 0644)
}

func (d *Diary) add(content, category, date string, weight float64, mood int) {
	if date == "" {
		date = time.Now().Format("2006-01-02")
	}
	note := Note{
		ID:        uuid.New().String()[:8],
		Content:   content,
		Category:  category,
		Date:      date,
		Weight:    weight,
		Mood:      mood,
		CreatedAt: time.Now().Format(time.RFC3339),
	}
	d.Notes = append(d.Notes, note)
	d.save()
	fmt.Printf("✅ Note added (ID: %s)\n", note.ID)
}

func (d *Diary) list(category, since string) {
	notes := d.Notes
	if category != "" {
		var filtered []Note
		for _, n := range notes {
			if n.Category == category {
				filtered = append(filtered, n)
			}
		}
		notes = filtered
	}
	if since != "" {
		var filtered []Note
		for _, n := range notes {
			if n.Date >= since {
				filtered = append(filtered, n)
			}
		}
		notes = filtered
	}
	if len(notes) == 0 {
		fmt.Println("No matching notes.")
		return
	}
	fmt.Printf("\n📋 Pregnancy Diary (%d entries):\n\n", len(notes))
	for _, n := range notes {
		weightStr := ""
		if n.Weight > 0 {
			weightStr = fmt.Sprintf(" – %.1f kg", n.Weight)
		}
		moodStr := ""
		if n.Mood > 0 {
			moodStr = " ⭐"
			for i := 1; i < n.Mood; i++ {
				moodStr += "⭐"
			}
		}
		fmt.Printf("%s: [%s] %s%s%s\n", n.Date, n.Category, n.Content, weightStr, moodStr)
	}
}

func (d *Diary) stats() {
	if len(d.Notes) == 0 {
		fmt.Println("No entries yet.")
		return
	}
	cats := make(map[string]int)
	var moods []int
	var weights []float64
	for _, n := range d.Notes {
		cats[n.Category]++
		if n.Mood > 0 {
			moods = append(moods, n.Mood)
		}
		if n.Weight > 0 {
			weights = append(weights, n.Weight)
		}
	}
	fmt.Println("\n📊 Statistics:")
	fmt.Printf("  Total entries: %d\n", len(d.Notes))
	for cat, count := range cats {
		fmt.Printf("  %s entries: %d\n", cat, count)
	}
	if len(moods) > 0 {
		sum := 0
		for _, m := range moods {
			sum += m
		}
		avg := float64(sum) / float64(len(moods))
		fmt.Printf("  Average mood: %.1f/5\n", avg)
	}
	if len(weights) > 0 {
		fmt.Printf("  Latest weight: %.1f kg\n", weights[len(weights)-1])
	}
}

func (d *Diary) search(term string) {
	var results []Note
	for _, n := range d.Notes {
		if contains(n.Content, term) {
			results = append(results, n)
		}
	}
	if len(results) == 0 {
		fmt.Println("No matches.")
		return
	}
	fmt.Printf("\n🔍 Found %d note(s):\n", len(results))
	for _, n := range results {
		fmt.Printf("%s: [%s] %s\n", n.Date, n.Category, n.Content)
	}
}

func contains(s, substr string) bool {
	return len(s) >= len(substr) && (s == substr || len(substr) == 0 ||
		(s[0:len(substr)] == substr) || (len(s) > len(substr) && (s[len(s)-len(substr):] == substr ||
		indexOf(s, substr) != -1)))
}

func indexOf(s, substr string) int {
	for i := 0; i <= len(s)-len(substr); i++ {
		if s[i:i+len(substr)] == substr {
			return i
		}
	}
	return -1
}

func (d *Diary) exportHTML(filename string) {
	html := `<!DOCTYPE html>
<html><head><title>Pregnancy Diary</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
.note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.meta{color:#999;font-size:0.9em;}
</style></head><body>
<h1>🤰 Pregnancy Diary</h1>
`
	for i := len(d.Notes) - 1; i >= 0; i-- {
		n := d.Notes[i]
		html += fmt.Sprintf(`<div class="note">
<div class="meta">%s – %s</div>
<div>%s</div>`, n.Date, n.Category, n.Content)
		if n.Weight > 0 {
			html += fmt.Sprintf("<div>Weight: %.1f kg</div>", n.Weight)
		}
		if n.Mood > 0 {
			html += fmt.Sprintf("<div>Mood: %s</div>", stars(n.Mood))
		}
		html += "</div>"
	}
	html += "</body></html>"
	os.WriteFile(filename, []byte(html), 0644)
	fmt.Printf("✅ Exported to %s\n", filename)
}

func stars(n int) string {
	s := ""
	for i := 0; i < n; i++ {
		s += "⭐"
	}
	return s
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: pregnancy_diary <command> [options]")
		return
	}
	d := &Diary{}
	d.load()
	cmd := os.Args[1]

	switch cmd {
	case "add":
		addCmd := flag.NewFlagSet("add", flag.ExitOnError)
		content := addCmd.String("content", "", "")
		category := addCmd.String("category", "general", "")
		date := addCmd.String("date", "", "")
		weight := addCmd.Float64("weight", 0, "")
		mood := addCmd.Int("mood", 0, "")
		addCmd.Parse(os.Args[2:])
		if *content == "" && len(addCmd.Args()) > 0 {
			*content = addCmd.Args()[0]
		}
		if *content == "" {
			fmt.Println("add requires content")
			return
		}
		d.add(*content, *category, *date, *weight, *mood)

	case "list":
		listCmd := flag.NewFlagSet("list", flag.ExitOnError)
		category := listCmd.String("category", "", "")
		since := listCmd.String("since", "", "")
		listCmd.Parse(os.Args[2:])
		d.list(*category, *since)

	case "stats":
		d.stats()

	case "search":
		if len(os.Args) < 3 {
			fmt.Println("search <term>")
			return
		}
		d.search(os.Args[2])

	case "export":
		exportCmd := flag.NewFlagSet("export", flag.ExitOnError)
		filename := exportCmd.String("filename", "diary.html", "")
		exportCmd.Parse(os.Args[2:])
		d.exportHTML(*filename)

	default:
		fmt.Println("Unknown command.")
	}
}
