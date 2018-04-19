def main
  f = File.read('third_party/parser/tests/whitequark.rs')
  prefix = 'test/testdata/disabled/whitequark/'
  Dir.mkdir(prefix) unless Dir.exist?(prefix)

  cur_file = nil

  f.split("\n").each do |line|
    match = /fn (.*)\(\)/.match(line)
    if match
      if !cur_file.nil?
        raise "No code for #{cur_file}"
      end
      cur_file = prefix + match[1] + '.rb'
      next
    end

    match = /let code = "(.*)"/.match(line)
    next unless match
    code = match[1]
    code = code.gsub('\n', "\n").gsub('\\"', "\"")
    File.write(cur_file, "# typed: true\n\n" + code.to_s)
    File.write(cur_file + '.infer.exp', '')
    cur_file = nil
  end
end
main
