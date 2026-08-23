// pregnancy_diary.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <random>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

string generateId() {
    const char* hex = "0123456789abcdef";
    string id;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 15);
    for (int i=0; i<8; i++) id += hex[dis(gen)];
    return id;
}

string currentTime() {
    time_t t = time(nullptr);
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&t));
    return string(buf);
}

string currentDate() {
    time_t t = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return string(buf);
}

struct Note {
    string id;
    string content;
    string category;
    string date;
    double weight;
    int mood;
    string created_at;

    Note() : weight(0), mood(0) {}
    Note(const string& c, const string& cat = "general", const string& d = "", double w = 0, int m = 0)
        : id(generateId()), content(c), category(cat), date(d.empty() ? currentDate() : d),
          weight(w), mood(m), created_at(currentTime()) {}
};

class Diary {
private:
    vector<Note> notes;
    string dataFile = "pregnancy_diary.json";

    void load() {
        ifstream f(dataFile);
        if (!f.is_open()) return;
        json j;
        f >> j;
        for (auto& item : j) {
            Note n;
            n.id = item["id"];
            n.content = item["content"];
            n.category = item["category"];
            n.date = item["date"];
            n.weight = item["weight"];
            n.mood = item["mood"];
            n.created_at = item["created_at"];
            notes.push_back(n);
        }
    }

    void save() {
        json j = json::array();
        for (auto& n : notes) {
            j.push_back({
                {"id", n.id},
                {"content", n.content},
                {"category", n.category},
                {"date", n.date},
                {"weight", n.weight},
                {"mood", n.mood},
                {"created_at", n.created_at}
            });
        }
        ofstream f(dataFile);
        f << setw(2) << j << endl;
    }

public:
    Diary() { load(); }

    void add(const string& content, const string& category, const string& date, double weight, int mood) {
        Note n(content, category, date, weight, mood);
        notes.push_back(n);
        save();
        cout << "✅ Note added (ID: " << n.id << ")\n";
    }

    void list(const string& category, const string& since) {
        vector<Note> filtered = notes;
        if (!category.empty()) {
            vector<Note> temp;
            for (auto& n : filtered) {
                if (n.category == category) temp.push_back(n);
            }
            filtered = temp;
        }
        if (!since.empty()) {
            vector<Note> temp;
            for (auto& n : filtered) {
                if (n.date >= since) temp.push_back(n);
            }
            filtered = temp;
        }
        if (filtered.empty()) {
            cout << "No matching notes.\n";
            return;
        }
        cout << "\n📋 Pregnancy Diary (" << filtered.size() << " entries):\n\n";
        for (auto& n : filtered) {
            string weightStr = (n.weight > 0) ? " – " + to_string(n.weight) + " kg" : "";
            string moodStr = (n.mood > 0) ? " ⭐" + string(n.mood-1, '⭐') : "";
            cout << n.date << ": [" << n.category << "] " << n.content << weightStr << moodStr << "\n";
        }
    }

    void stats() {
        if (notes.empty()) {
            cout << "No entries yet.\n";
            return;
        }
        map<string, int> cats;
        vector<int> moods;
        vector<double> weights;
        for (auto& n : notes) {
            cats[n.category]++;
            if (n.mood > 0) moods.push_back(n.mood);
            if (n.weight > 0) weights.push_back(n.weight);
        }
        cout << "\n📊 Statistics:\n";
        cout << "  Total entries: " << notes.size() << "\n";
        for (auto& kv : cats) {
            cout << "  " << kv.first << " entries: " << kv.second << "\n";
        }
        if (!moods.empty()) {
            double sum = 0;
            for (int m : moods) sum += m;
            double avg = sum / moods.size();
            cout << "  Average mood: " << fixed << setprecision(1) << avg << "/5\n";
        }
        if (!weights.empty()) {
            cout << "  Latest weight: " << weights.back() << " kg\n";
        }
    }

    void search(const string& term) {
        vector<Note> results;
        for (auto& n : notes) {
            if (n.content.find(term) != string::npos) {
                results.push_back(n);
            }
        }
        if (results.empty()) {
            cout << "No matches.\n";
            return;
        }
        cout << "\n🔍 Found " << results.size() << " note(s):\n";
        for (auto& n : results) {
            cout << n.date << ": [" << n.category << "] " << n.content << "\n";
        }
    }

    void exportHTML(const string& filename) {
        ofstream f(filename);
        f << "<!DOCTYPE html>\n<html><head><title>Pregnancy Diary</title>\n";
        f << "<style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}\n";
        f << ".note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}\n";
        f << ".meta{color:#999;font-size:0.9em;}\n";
        f << "</style></head><body>\n<h1>🤰 Pregnancy Diary</h1>\n";
        for (int i = notes.size()-1; i >= 0; i--) {
            auto& n = notes[i];
            f << "<div class=\"note\">\n";
            f << "<div class=\"meta\">" << n.date << " – " << n.category << "</div>\n";
            f << "<div>" << n.content << "</div>\n";
            if (n.weight > 0) f << "<div>Weight: " << n.weight << " kg</div>\n";
            if (n.mood > 0) f << "<div>Mood: " << string(n.mood, '⭐') << "</div>\n";
            f << "</div>\n";
        }
        f << "</body></html>\n";
        f.close();
        cout << "✅ Exported to " << filename << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: pregnancy_diary <command> [options]\n";
        return 1;
    }
    Diary d;
    string cmd = argv[1];

    if (cmd == "add") {
        if (argc < 3) { cerr << "add <content> [--category CAT] [--date DATE] [--weight KG] [--mood RATING]\n"; return 1; }
        string content = argv[2];
        string category = "general", date;
        double weight = 0;
        int mood = 0;
        for (int i=3; i<argc; i++) {
            if (string(argv[i]) == "--category" && i+1 < argc) category = argv[++i];
            if (string(argv[i]) == "--date" && i+1 < argc) date = argv[++i];
            if (string(argv[i]) == "--weight" && i+1 < argc) weight = stod(argv[++i]);
            if (string(argv[i]) == "--mood" && i+1 < argc) mood = stoi(argv[++i]);
        }
        d.add(content, category, date, weight, mood);
    } else if (cmd == "list") {
        string category, since;
        for (int i=2; i<argc; i++) {
            if (string(argv[i]) == "--category" && i+1 < argc) category = argv[++i];
            if (string(argv[i]) == "--since" && i+1 < argc) since = argv[++i];
        }
        d.list(category, since);
    } else if (cmd == "stats") {
        d.stats();
    } else if (cmd == "search") {
        if (argc < 3) { cerr << "search <term>\n"; return 1; }
        d.search(argv[2]);
    } else if (cmd == "export") {
        string filename = "diary.html";
        for (int i=2; i<argc; i++) {
            if (string(argv[i]) == "--filename" && i+1 < argc) filename = argv[++i];
        }
        d.exportHTML(filename);
    } else {
        cerr << "Unknown command. Use add, list, stats, search, export.\n";
        return 1;
    }
    return 0;
}
