#!/usr/bin/env ruby

# taken from https://github.com/buildkite-plugins/junit-annotate-buildkite-plugin/blob/master/ruby/bin/annotate


require 'rexml/document'
require 'rexml/element'

# Reads a list of junit files and returns a nice Buildkite build annotation on
# STDOUT that summarizes any failures.

junits_dir = ARGV[0]
abort("Usage: annotate <junits-dir>") unless junits_dir
abort("#{junits_dir} does not exist") unless Dir.exist?(junits_dir)

job_pattern = ENV['BUILDKITE_PLUGIN_JUNIT_ANNOTATE_JOB_UUID_FILE_PATTERN']
job_pattern = '-(.*).xml' if !job_pattern || job_pattern.empty?

failure_format = ENV['BUILDKITE_PLUGIN_JUNIT_ANNOTATE_FAILURE_FORMAT']
failure_format = 'classname' if !failure_format || failure_format.empty?

Failure = Struct.new(:name, :failed_test, :body, :job, :type)

junit_report_files = Dir.glob(File.join(junits_dir, "**", "*"))
testcases = 0
failures = []

def text_content(element)
  # Handle mulptiple CDATA/text children elements
  text = element.texts().map(&:value).join.strip
  if text.empty?
    nil
  else
    text
  end
end

junit_report_files.sort.each do |file|
  next if File.directory?(file)

  STDERR.puts "Parsing #{file.sub(junits_dir, '')}"
  job = File.basename(file)[/#{job_pattern}/, 1]
  xml = File.read(file)
  doc = REXML::Document.new(xml)

  REXML::XPath.each(doc, '//testsuite//testcase') do |testcase|
    testcases += 1
    name = testcase.attributes['name'].to_s
    failed_test = testcase.attributes[failure_format].to_s
    testcase.elements.each("failure") do |failure|
      failures << Failure.new(name, failed_test, text_content(failure), job, :failure)
    end
    testcase.elements.each("error") do |error|
      failures << Failure.new(name, failed_test, text_content(error), job, :error)
    end
  end
end

STDERR.puts "--- ❓ Checking failures"
STDERR.puts "#{testcases} testcases found"

if failures.empty?
  STDERR.puts "There were no failures/errors 🙌"
  exit 0
else
  STDERR.puts "There #{failures.length == 1 ? "is 1 failure/error" : "are #{failures.length} failures/errors" } 😭"
end

STDERR.puts "--- ✍️ Preparing annotation"

failures_count = failures.select {|f| f.type == :failure }.length
errors_count = failures.select {|f| f.type == :error }.length

puts [
  failures_count == 0 ? nil : (failures_count == 1 ? "1 failure" : "#{failures_count} failures"),
  errors_count === 0 ? nil : (errors_count == 1 ? "1 error" : "#{errors_count} errors"),
].compact.join(" and ") + ":\n\n"

failures.each do |failure|
  puts "<details>"
  puts "<summary><code>#{failure.name} in #{failure.failed_test}</code></summary>\n\n"
  if failure.body
    puts "<pre><code>#{failure.body.chomp.strip}</code></pre>\n\n"
  end
  if failure.job
    puts "in <a href=\"##{failure.job}\">Job ##{failure.job}</a>"
  end
  puts "</details>"
  puts "" unless failure == failures.last
end

