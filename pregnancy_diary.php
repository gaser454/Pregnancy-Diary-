# pregnancy_diary.php
#!/usr/bin/env php
<?php

define('DATA_FILE', 'pregnancy_diary.json');

class Note {
    public $id;
    public $content;
    public $category;
    public $date;
    public $weight;
    public $mood;
    public $created_at;

    function __construct($content, $category = 'general', $date = null, $weight = null, $mood = null) {
        $this->id = substr(bin2hex(random_bytes(4)), 0, 8);
        $this->content = $content;
        $this->category = $category;
        $this->date = $date ?: date('Y-m-d');
        $this->weight = $weight;
        $this->mood = $mood;
        $this->created_at = date('c');
    }

    function toArray() {
        return [
            'id' => $this->id,
            'content' => $this->content,
            'category' => $this->category,
            'date' => $this->date,
            'weight' => $this->weight,
            'mood' => $this->mood,
            'created_at' => $this->created_at
        ];
    }

    static function fromArray($data) {
        $n = new self($data['content'], $data['category'], $data['date'], $data['weight'], $data['mood']);
        $n->id = $data['id'];
        $n->created_at = $data['created_at'];
        return $n;
    }
}

class Diary {
    private $notes = [];

    function __construct() {
        $this->load();
    }

    function load() {
        if (file_exists(DATA_FILE)) {
            $data = json_decode(file_get_contents(DATA_FILE), true);
            $this->notes = array_map(function($d) { return Note::fromArray($d); }, $data);
        }
    }

    function save() {
        $data = array_map(function($n) { return $n->toArray(); }, $this->notes);
        file_put_contents(DATA_FILE, json_encode($data, JSON_PRETTY_PRINT));
    }

    function add($content, $category = 'general', $date = null, $weight = null, $mood = null) {
        $n = new Note($content, $category, $date, $weight, $mood);
        $this->notes[] = $n;
        $this->save();
        echo "✅ Note added (ID: {$n->id})\n";
    }

    function list($category = null, $since = null) {
        $notes = $this->notes;
        if ($category) {
            $notes = array_filter($notes, function($n) use ($category) {
                return $n->category == $category;
            });
        }
        if ($since) {
            $notes = array_filter($notes, function($n) use ($since) {
                return $n->date >= $since;
            });
        }
        if (empty($notes)) {
            echo "No matching notes.\n";
            return;
        }
        echo "\n📋 Pregnancy Diary (" . count($notes) . " entries):\n\n";
        foreach ($notes as $n) {
            $weightStr = $n->weight ? " – {$n->weight} kg" : '';
            $moodStr = $n->mood ? " ⭐" . str_repeat('⭐', $n->mood-1) : '';
            echo "{$n->date}: [{$n->category}] {$n->content}{$weightStr}{$moodStr}\n";
        }
    }

    function stats() {
        if (empty($this->notes)) {
            echo "No entries yet.\n";
            return;
        }
        $cats = [];
        $moods = [];
        $weights = [];
        foreach ($this->notes as $n) {
            $cats[$n->category] = ($cats[$n->category] ?? 0) + 1;
            if ($n->mood) $moods[] = $n->mood;
            if ($n->weight) $weights[] = $n->weight;
        }
        echo "\n📊 Statistics:\n";
        echo "  Total entries: " . count($this->notes) . "\n";
        foreach ($cats as $cat => $count) {
            echo "  " . ucfirst($cat) . " entries: $count\n";
        }
        if (!empty($moods)) {
            $avg = array_sum($moods) / count($moods);
            echo "  Average mood: " . round($avg, 1) . "/5\n";
        }
        if (!empty($weights)) {
            echo "  Latest weight: " . end($weights) . " kg\n";
        }
    }

    function search($term) {
        $results = array_filter($this->notes, function($n) use ($term) {
            return stripos($n->content, $term) !== false;
        });
        if (empty($results)) {
            echo "No matches.\n";
            return;
        }
        echo "\n🔍 Found " . count($results) . " note(s):\n";
        foreach ($results as $n) {
            echo "{$n->date}: [{$n->category}] {$n->content}\n";
        }
    }

    function exportHTML($filename = 'diary.html') {
        $html = '<!DOCTYPE html>
<html><head><title>Pregnancy Diary</title>
<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
.note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
.meta{color:#999;font-size:0.9em;}
</style></head><body>
<h1>🤰 Pregnancy Diary</h1>
';
        foreach (array_reverse($this->notes) as $n) {
            $html .= "<div class=\"note\">\n";
            $html .= "<div class=\"meta\">{$n->date} – " . ucfirst($n->category) . "</div>\n";
            $html .= "<div>{$n->content}</div>\n";
            if ($n->weight) $html .= "<div>Weight: {$n->weight} kg</div>\n";
            if ($n->mood) $html .= "<div>Mood: " . str_repeat('⭐', $n->mood) . "</div>\n";
            $html .= "</div>\n";
        }
        $html .= '</body></html>';
        file_put_contents($filename, $html);
        echo "✅ Exported to $filename\n";
    }
}

if ($argc < 2) {
    die("Usage: php pregnancy_diary.php <command> [options]\n");
}
$d = new Diary();
$cmd = $argv[1];

switch ($cmd) {
    case 'add':
        if ($argc < 3) die("add <content> [--category CAT] [--date DATE] [--weight KG] [--mood RATING]\n");
        $content = $argv[2];
        $category = 'general';
        $date = null;
        $weight = null;
        $mood = null;
        for ($i=3; $i<$argc; $i++) {
            if ($argv[$i] == '--category' && isset($argv[$i+1])) { $category = $argv[++$i]; }
            if ($argv[$i] == '--date' && isset($argv[$i+1])) { $date = $argv[++$i]; }
            if ($argv[$i] == '--weight' && isset($argv[$i+1])) { $weight = (float)$argv[++$i]; }
            if ($argv[$i] == '--mood' && isset($argv[$i+1])) { $mood = (int)$argv[++$i]; }
        }
        $d->add($content, $category, $date, $weight, $mood);
        break;

    case 'list':
        $category = null;
        $since = null;
        for ($i=2; $i<$argc; $i++) {
            if ($argv[$i] == '--category' && isset($argv[$i+1])) { $category = $argv[++$i]; }
            if ($argv[$i] == '--since' && isset($argv[$i+1])) { $since = $argv[++$i]; }
        }
        $d->list($category, $since);
        break;

    case 'stats':
        $d->stats();
        break;

    case 'search':
        if ($argc < 3) die("search <term>\n");
        $d->search($argv[2]);
        break;

    case 'export':
        $filename = 'diary.html';
        for ($i=2; $i<$argc; $i++) {
            if ($argv[$i] == '--filename' && isset($argv[$i+1])) { $filename = $argv[++$i]; }
        }
        $d->exportHTML($filename);
        break;

    default:
        echo "Unknown command. Use add, list, stats, search, export.\n";
}
?>
