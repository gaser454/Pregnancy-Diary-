# pregnancy_diary.rb
#!/usr/bin/env ruby
require 'json'
require 'securerandom'
require 'date'

DATA_FILE = 'pregnancy_diary.json'

class Note
  attr_accessor :id, :content, :category, :date, :weight, :mood, :created_at

  def initialize(content, category = 'general', date = nil, weight = nil, mood = nil)
    @id = SecureRandom.hex(4)
    @content = content
    @category = category
    @date = date || Date.today.to_s
    @weight = weight
    @mood = mood
    @created_at = Time.now.iso8601
  end

  def to_hash
    {
      id: @id, content: @content, category: @category, date: @date,
      weight: @weight, mood: @mood, created_at: @created_at
    }
  end

  def self.from_hash(h)
    note = new(h['content'], h['category'], h['date'], h['weight'], h['mood'])
    note.id = h['id']
    note.created_at = h['created_at']
    note
  end
end

class Diary
  attr_reader :notes

  def initialize
    @notes = []
    load
  end

  def load
    if File.exist?(DATA_FILE)
      data = JSON.parse(File.read(DATA_FILE))
      @notes = data.map { |h| Note.from_hash(h) }
    end
  end

  def save
    File.write(DATA_FILE, JSON.pretty_generate(@notes.map(&:to_hash)))
  end

  def add(content, category = 'general', date = nil, weight = nil, mood = nil)
    note = Note.new(content, category, date, weight, mood)
    @notes << note
    save
    puts "✅ Note added (ID: #{note.id})"
  end

  def list(category = nil, since = nil)
    notes = @notes
    notes = notes.select { |n| n.category == category } if category
    notes = notes.select { |n| n.date >= since } if since
    if notes.empty?
      puts "No matching notes."
      return
    end
    puts "\n📋 Pregnancy Diary (#{notes.size} entries):\n"
    notes.each do |n|
      weight_str = n.weight ? " – #{n.weight} kg" : ""
      mood_str = n.mood ? " ⭐#{'⭐' * (n.mood - 1)}" : ""
      puts "#{n.date}: [#{n.category}] #{n.content}#{weight_str}#{mood_str}"
    end
  end

  def stats
    if @notes.empty?
      puts "No entries yet."
      return
    end
    cats = Hash.new(0)
    moods = []
    weights = []
    @notes.each do |n|
      cats[n.category] += 1
      moods << n.mood if n.mood
      weights << n.weight if n.weight
    end
    puts "\n📊 Statistics:"
    puts "  Total entries: #{@notes.size}"
    cats.each { |cat, count| puts "  #{cat.capitalize} entries: #{count}" }
    unless moods.empty?
      avg = moods.sum.to_f / moods.size
      puts "  Average mood: #{avg.round(1)}/5"
    end
    puts "  Latest weight: #{weights.last} kg" unless weights.empty?
  end

  def search(term)
    results = @notes.select { |n| n.content.downcase.include?(term.downcase) }
    if results.empty?
      puts "No matches."
      return
    end
    puts "\n🔍 Found #{results.size} note(s):"
    results.each { |n| puts "#{n.date}: [#{n.category}] #{n.content}" }
  end

  def export_html(filename = 'diary.html')
    html = <<~HTML
    <!DOCTYPE html>
    <html><head><title>Pregnancy Diary</title>
    <style>body{font-family:sans-serif;margin:30px;background:#f5f5f5;}
    .note{background:#fff;border-radius:8px;padding:15px;margin:10px 0;box-shadow:0 2px 4px rgba(0,0,0,0.1);}
    .meta{color:#999;font-size:0.9em;}
    </style></head><body>
    <h1>🤰 Pregnancy Diary</h1>
    HTML
    @notes.reverse.each do |n|
      html += <<~NOTE
      <div class="note">
      <div class="meta">#{n.date} – #{n.category.capitalize}</div>
      <div>#{n.content}</div>
      NOTE
      html += "<div>Weight: #{n.weight} kg</div>" if n.weight
      html += "<div>Mood: #{'⭐' * n.mood}</div>" if n.mood
      html += "</div>"
    end
    html += "</body></html>"
    File.write(filename, html)
    puts "✅ Exported to #{filename}"
  end
end

if ARGV.empty?
  puts "Usage: pregnancy_diary.rb <command> [options]"
  exit
end

d = Diary.new
cmd = ARGV.shift

case cmd
when 'add'
  if ARGV.empty?
    puts "add <content> [--category CAT] [--date DATE] [--weight KG] [--mood RATING]"
    exit
  end
  content = ARGV.shift
  category = 'general'
  date = nil
  weight = nil
  mood = nil
  while ARGV.any?
    case ARGV[0]
    when '--category'
      ARGV.shift
      category = ARGV.shift || 'general'
    when '--date'
      ARGV.shift
      date = ARGV.shift
    when '--weight'
      ARGV.shift
      weight = ARGV.shift.to_f
    when '--mood'
      ARGV.shift
      mood = ARGV.shift.to_i
    else
      break
    end
  end
  d.add(content, category, date, weight, mood)

when 'list'
  category = nil
  since = nil
  if ARGV.include?('--category')
    idx = ARGV.index('--category')
    category = ARGV[idx+1] if idx
  end
  if ARGV.include?('--since')
    idx = ARGV.index('--since')
    since = ARGV[idx+1] if idx
  end
  d.list(category, since)

when 'stats'
  d.stats

when 'search'
  term = ARGV.shift
  if term.nil?
    puts "search <term>"
    exit
  end
  d.search(term)

when 'export'
  filename = 'diary.html'
  if ARGV.include?('--filename')
    idx = ARGV.index('--filename')
    filename = ARGV[idx+1] if idx
  end
  d.export_html(filename)

else
  puts "Unknown command. Use add, list, stats, search, export."
end
