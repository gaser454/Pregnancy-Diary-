// pregnancy_diary.js
#!/usr/bin/env node
const fs = require('fs');
const { program } = require('commander');
const { v4: uuidv4 } = require('uuid');

const DATA_FILE = 'pregnancy_diary.json';

class Note {
    constructor(content, category = 'general', date = null, weight = null, mood = null) {
        this.id = uuidv4().slice(0,8);
        this.content = content;
        this.category = category;
        this.date = date || new Date().toISOString().slice(0,10);
        this.weight = weight;
        this.mood = mood;
        this.created_at = new Date().toISOString();
    }
}

class Diary {
    constructor() {
        this.notes = [];
        this.load();
    }

    load() {
        if (fs.existsSync(DATA_FILE)) {
            try {
                this.notes = JSON.parse(fs.readFileSync(DATA_FILE));
            } catch (e) {}
        }
    }

    save() {
        fs.writeFileSync(DATA_FILE, JSON.stringify(this.notes, null, 2));
    }

    add(content, category, date, weight, mood) {
        const note = new Note(content, category, date, weight, mood);
        this.notes.push(note);
        this.save();
        console.log(`✅ Note added (ID: ${note.id})`);
    }

    list(category, since) {
        let notes = this.notes;
        if (category) {
            notes = notes.filter(n => n.category === category);
        }
        if (since) {
            notes = notes.filter(n => n.date >= since);
        }
        if (notes.length === 0) {
            console.log('No matching notes.');
            return;
        }
        console.log(`\n📋 Pregnancy Diary (${notes.length} entries):\n`);
        for (const n of notes) {
            let weightStr = n.weight ? ` – ${n.weight} kg` : '';
            let moodStr = n.mood ? ` ⭐${'⭐'.repeat(n.mood-1)}` : '';
            console.log(`${n.date}: [${n.category}] ${n.content}${weightStr}${moodStr}`);
        }
    }

    stats() {
        if (this.notes.length === 0) {
            console.log('No entries yet.');
            return;
        }
        const cats = {};
        const moods = [];
        const weights = [];
        for (const n of this.notes) {
            cats[n.category] = (cats[n.category] || 0) + 1;
            if (n.mood) moods.push(n.mood);
            if (n.weight) weights.push(n.weight);
        }
        console.log('\n📊 Statistics:');
        console.log(`  Total entries: ${this.notes.length}`);
        for (const [cat, count] of Object.entries(cats)) {
            console.log(`  ${cat.charAt(0).toUpperCase() + cat.slice(1)} entries: ${count}`);
        }
        if (moods.length) {
            const avg = moods.reduce((a,b) => a+b, 0) / moods.length;
            console.log(`  Average mood: ${avg.toFixed(1)}/5`);
        }
        if (weights.length) {
            console.log(`  Latest weight: ${weights[weights.length-1]} kg`);
        }
    }

    search(term) {
        const results = this.notes.filter(n => n.content.toLowerCase().includes(term.toLowerCase()));
        if (results.length === 0) {
            console.log('No matches.');
            return;
        }
        console.log(`\n🔍 Found ${results.length} note(s):`);
        for (const n of results) {
            console.log(`${n.date}: [${n.category}] ${n.content}`);
        }
    }

    exportHTML(filename) {
        let html = `<!DOCTYPE html>
<html><head><title>Pregnancy Diary</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
.note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.meta{color:#999;font-size:0.9em;}
</style></head><body>
<h1>🤰 Pregnancy Diary</h1>
`;
        for (const n of this.notes.slice().reverse()) {
            html += `<div class="note">
<div class="meta">${n.date} – ${n.category.charAt(0).toUpperCase() + n.category.slice(1)}</div>
<div>${n.content}</div>`;
            if (n.weight) html += `<div>Weight: ${n.weight} kg</div>`;
            if (n.mood) html += `<div>Mood: ${'⭐'.repeat(n.mood)}</div>`;
            html += `</div>`;
        }
        html += `</body></html>`;
        fs.writeFileSync(filename, html);
        console.log(`✅ Exported to ${filename}`);
    }
}

program
    .command('add <content>')
    .option('--category <category>', 'symptom, mood, weight, general', 'general')
    .option('--date <date>', 'YYYY-MM-DD')
    .option('--weight <kg>', 'Weight in kg', parseFloat)
    .option('--mood <rating>', 'Mood rating 1-5', parseInt)
    .action((content, options) => {
        const d = new Diary();
        d.add(content, options.category, options.date, options.weight, options.mood);
    });

program
    .command('list')
    .option('--category <category>', 'Filter by category')
    .option('--since <date>', 'YYYY-MM-DD')
    .action((options) => {
        const d = new Diary();
        d.list(options.category, options.since);
    });

program
    .command('stats')
    .action(() => {
        const d = new Diary();
        d.stats();
    });

program
    .command('search <term>')
    .action((term) => {
        const d = new Diary();
        d.search(term);
    });

program
    .command('export')
    .option('--filename <file>', 'Output HTML file', 'diary.html')
    .action((options) => {
        const d = new Diary();
        d.exportHTML(options.filename);
    });

program.parse(process.argv);
