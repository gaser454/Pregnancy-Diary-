🤰 Pregnancy Diary — Multi‑Language Pregnancy Notes Tracker
8 languages, one complete pregnancy diary – track symptoms, mood, weight, and notes throughout your pregnancy journey – right from your terminal.

✨ Features
📝 Add notes – log symptoms, mood, weight, and general notes with dates

📋 List all notes – view your diary entries with filters by category

📊 Statistics – see weight progress, mood trends, and entry counts

🔍 Search – find notes by content or category

💾 Persistent storage – all data saved in pregnancy_diary.json

📤 Export to HTML – generate a beautiful diary report (optional)

🚀 Quick Start
All implementations follow the same CLI pattern:

bash
# Add a note (category: symptom, mood, weight, general)
<command> add "Feeling great today!" --category mood --date 2026-08-23

# Add a weight entry
<command> add "Weight check" --category weight --weight 68.5

# List all notes
<command> list

# List notes by category
<command> list --category mood

# Show statistics
<command> stats

# Search notes
<command> search "great"

# Export to HTML report
<command> export diary.html
Commands/Arguments:

add <content> [--category CAT] [--date DATE] [--weight KG] [--mood RATING] – add a note

list [--category CAT] [--since DATE] – list notes (filtered)

stats – show statistics

search <term> – search notes

export [--filename FILE] – export to HTML

📸 Example Output
text
📋 Pregnancy Diary (5 entries):

2026-08-23: [mood] Feeling great today! ⭐⭐⭐⭐
2026-08-22: [symptom] Mild nausea in the morning
2026-08-21: [weight] Weight check – 68.5 kg
2026-08-20: [general] Baby kicked for the first time! 🥰
2026-08-19: [mood] Tired but happy

📊 Statistics:
  Total entries: 5
  Mood entries: 2
  Weight entries: 1
  Symptom entries: 1
  General entries: 1
  Average mood: 4.0/5
  Latest weight: 68.5 kg
📁 Repository Structure
text
.
├── README.md
├── python/
│   └── pregnancy_diary.py
├── go/
│   └── pregnancy_diary.go
├── javascript/
│   └── pregnancy_diary.js
├── ruby/
│   └── pregnancy_diary.rb
├── php/
│   └── pregnancy_diary.php
├── java/
│   └── PregnancyDiary.java
├── csharp/
│   └── PregnancyDiary.cs
└── cpp/
    └── pregnancy_diary.cpp
