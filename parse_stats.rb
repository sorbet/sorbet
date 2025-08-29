#!/usr/bin/env ruby

# Script to compare statistics between before.txt and after.txt files
# Format: "         If :           3831, 0.1%"

def format_number(num)
  num.to_s.reverse.gsub(/(\d{3})(?=\d)/, '\\1,').reverse
end

def parse_stats_file(filename)
  results = {}

  File.readlines(filename, chomp: true).each_with_index do |line, index|
    # Skip empty lines
    next if line.strip.empty?

    # Regex to match the pattern: whitespace + label + : + whitespace + number + , + percentage
    match = line.match(/^\s*(\w+)\s*:\s*(\d+),?\s*([\d.]+)?%?/)

    if match
      label = match[1]
      count = match[2].to_i
      percentage = match[3] ? match[3].to_f : 0.0

      results[label] = {
        line_number: index + 1,
        label: label,
        count: count,
        percentage: percentage,
        raw_line: line
      }
    elsif line.match(/^\s*\w+\s*:\s*\d+/) # Try to catch lines without percentages
      puts "Warning: Could not fully parse line #{index + 1}: #{line}"
    end
  end

  results
end

def compare_stats(before_stats, after_stats)
  all_labels = (before_stats.keys + after_stats.keys).uniq.sort
  comparisons = []

  all_labels.each do |label|
    before_entry = before_stats[label]
    after_entry = after_stats[label]

    if before_entry && after_entry
      # Both exist - calculate handling percentage
      before_count = before_entry[:count]
      after_count = after_entry[:count]
      not_handled_pct = (after_count.to_f / before_count * 100).round(1)
      handled_pct = (100.0 - not_handled_pct).round(1)

      comparisons << {
        label: label,
        before_count: before_count,
        after_count: after_count,
        handled_pct: handled_pct,
        not_handled_pct: not_handled_pct,
        status: :both
      }
    elsif before_entry && !after_entry
      # Only in before - completely handled
      comparisons << {
        label: label,
        before_count: before_entry[:count],
        after_count: 0,
        handled_pct: 100.0,
        not_handled_pct: 0.0,
        status: :eliminated
      }
    elsif !before_entry && after_entry
      # Only in after - newly introduced (not handled)
      comparisons << {
        label: label,
        before_count: 0,
        after_count: after_entry[:count],
        handled_pct: -Float::INFINITY,
        not_handled_pct: Float::INFINITY,
        status: :new
      }
    end
  end

  comparisons
end

# Main execution
if __FILE__ == $0
  before_file = ARGV[0] || 'before.txt'
  after_file = ARGV[1] || 'after.txt'

  unless File.exist?(before_file)
    puts "Error: File '#{before_file}' not found!"
    exit 1
  end

  unless File.exist?(after_file)
    puts "Error: File '#{after_file}' not found!"
    exit 1
  end

  before_stats = parse_stats_file(before_file)
  after_stats = parse_stats_file(after_file)

  comparisons = compare_stats(before_stats, after_stats)


  # Summary first
  both_entries = comparisons.select { |c| c[:status] == :both }
  eliminated_entries = comparisons.select { |c| c[:status] == :eliminated }
  all_entries = both_entries + eliminated_entries

  total_nodes_in_repo = all_entries.sum { |c| c[:before_count] }
  total_nodes_handled = all_entries.sum { |c| c[:after_count] }

  formatted_repo_count = format_number(total_nodes_in_repo)
  formatted_handled_count = format_number(total_nodes_handled)

  puts "Direct desugaring summary:"
  puts "    Total nodes in repo: #{formatted_repo_count}"
  puts "    Total nodes handled: %#{formatted_repo_count.length}s" % formatted_handled_count
  puts "    Completed node types: #{eliminated_entries.length}"

  if both_entries.any?
    total_before = both_entries.sum { |c| c[:before_count] }
    total_after = both_entries.sum { |c| c[:after_count] }
    overall_not_handled = (total_after.to_f / total_before * 100).round(1)
    overall_handled = (100 - overall_not_handled).round(1)
    puts "    Directly desugared rate: #{overall_handled}%"
    puts "    Fallback desugared rate: #{overall_not_handled}%"
  end

  puts
  puts "%-15s %10s %10s %10s %10s %s" % ["Node type", "Count", "Fallbacks", "Direct", "Fallback", "Status"]
  puts "-" * 80

  # Sort by handling percentage (highest handling first), then by before_count for ties
  sorted_comparisons = comparisons.sort_by { |c| [-c[:handled_pct], -c[:before_count]] }

  sorted_comparisons.each do |comp|
    status_symbol = case comp[:status]
                   when :eliminated then "Done!"
                   when :new then "NEW"
                   when :both then ""
                   end

    if comp[:status] == :new
      puts "%-15s %10s %10s %10s %10s %s" % [
        comp[:label],
        "-",
        format_number(comp[:after_count]),
        "NEW",
        "∞",
        status_symbol
      ]
    else
      puts "%-15s %10s %10s %9.1f%% %9.1f%% %s" % [
        comp[:label],
        format_number(comp[:before_count]),
        format_number(comp[:after_count]),
        comp[:handled_pct],
        comp[:not_handled_pct],
        status_symbol
      ]
    end
  end
end
