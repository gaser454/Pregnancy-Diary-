# pregnancy_diary.py
import json
import os
import sys
import argparse
import uuid
from datetime import datetime, date

DATA_FILE = "pregnancy_diary.json"

class Note:
    def __init__(self, content, category="general", date_str=None, weight=None, mood=None):
        self.id = str(uuid.uuid4())[:8]
        self.content = content
        self.category = category  # symptom, mood, weight, general
        self.date = date_str or datetime.now().strftime("%Y-%m-%d")
        self.weight = weight
        self.mood = mood
        self.created_at = datetime.now().isoformat()

    def to_dict(self):
        return {
            "id": self.id,
            "content": self.content,
            "category": self.category,
            "date": self.date,
            "weight": self.weight,
            "mood": self.mood,
            "created_at": self.created_at
        }

    @classmethod
    def from_dict(cls, data):
        note = cls(data["content"], data["category"], data["date"], data.get("weight"), data.get("mood"))
        note.id = data["id"]
        note.created_at = data["created_at"]
        return note

class Diary:
    def __init__(self):
        self.notes = []
        self.load()

    def load(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, "r") as f:
                data = json.load(f)
                self.notes = [Note.from_dict(n) for n in data]

    def save(self):
        with open(DATA_FILE, "w") as f:
            json.dump([n.to_dict() for n in self.notes], f, indent=2)

    def add(self, content, category="general", date_str=None, weight=None, mood=None):
        note = Note(content, category, date_str, weight, mood)
        self.notes.append(note)
        self.save()
        print(f"✅ Note added (ID: {note.id})")

    def list(self, category=None, since=None):
        notes = self.notes
        if category:
            notes = [n for n in notes if n.category == category]
        if since:
            notes = [n for n in notes if n.date >= since]
        if not notes:
            print("No matching notes.")
            return
        print(f"\n📋 Pregnancy Diary ({len(notes)} entries):\n")
        for n in notes:
            weight_str = f" – {n.weight} kg" if n.weight else ""
            mood_str = f" ⭐{'⭐' * (n.mood - 1) if n.mood else ''}" if n.mood else ""
            print(f"{n.date}: [{n.category}] {n.content}{weight_str}{mood_str}")

    def stats(self):
        if not self.notes:
            print("No entries yet.")
            return
        categories = {}
        moods = []
        weights = []
        for n in self.notes:
            categories[n.category] = categories.get(n.category, 0) + 1
            if n.mood:
                moods.append(n.mood)
            if n.weight:
                weights.append(n.weight)
        print("\n📊 Statistics:")
        print(f"  Total entries: {len(self.notes)}")
        for cat, count in sorted(categories.items()):
            print(f"  {cat.capitalize()} entries: {count}")
        if moods:
            avg_mood = sum(moods) / len(moods)
            print(f"  Average mood: {avg_mood:.1f}/5")
        if weights:
            latest_weight = weights[-1]
            print(f"  Latest weight: {latest_weight} kg")

    def search(self, term):
        results = [n for n in self.notes if term.lower() in n.content.lower()]
        if not results:
            print("No matches.")
            return
        print(f"\n🔍 Found {len(results)} note(s):")
        for n in results:
            print(f"{n.date}: [{n.category}] {n.content}")

    def export_html(self, filename="diary.html"):
        html = """<!DOCTYPE html>
<html><head><title>Pregnancy Diary</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
.note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.meta{color:#999;font-size:0.9em;}
</style></head><body>
<h1>🤰 Pregnancy Diary</h1>
"""
        for n in sorted(self.notes, key=lambda x: x.date, reverse=True):
            html += f"""<div class="note">
<div class="meta">{n.date} – {n.category.capitalize()}</div>
<div>{n.content}</div>
"""
            if n.weight:
                html += f"<div>Weight: {n.weight} kg</div>"
            if n.mood:
                html += f"<div>Mood: {'⭐' * n.mood}</div>"
            html += "</div>"
        html += "</body></html>"
        with open(filename, "w") as f:
            f.write(html)
        print(f"✅ Exported to {filename}")

def main():
    parser = argparse.ArgumentParser(description="Pregnancy Diary")
    subparsers = parser.add_subparsers(dest="cmd", required=True)

    add_parser = subparsers.add_parser("add")
    add_parser.add_argument("content")
    add_parser.add_argument("--category", default="general", choices=["symptom", "mood", "weight", "general"])
    add_parser.add_argument("--date", help="YYYY-MM-DD")
    add_parser.add_argument("--weight", type=float)
    add_parser.add_argument("--mood", type=int, choices=range(1,6))

    list_parser = subparsers.add_parser("list")
    list_parser.add_argument("--category", choices=["symptom", "mood", "weight", "general"])
    list_parser.add_argument("--since", help="YYYY-MM-DD")

    subparsers.add_parser("stats")
    search_parser = subparsers.add_parser("search")
    search_parser.add_argument("term")
    export_parser = subparsers.add_parser("export")
    export_parser.add_argument("--filename", default="diary.html")

    args = parser.parse_args()
    diary = Diary()

    if args.cmd == "add":
        diary.add(args.content, args.category, args.date, args.weight, args.mood)
    elif args.cmd == "list":
        diary.list(args.category, args.since)
    elif args.cmd == "stats":
        diary.stats()
    elif args.cmd == "search":
        diary.search(args.term)
    elif args.cmd == "export":
        diary.export_html(args.filename)

if __name__ == "__main__":
    main()
