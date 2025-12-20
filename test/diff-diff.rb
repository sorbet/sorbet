#!/usr/bin/env ruby

# This script post-processes the output of llvm-diff to work around the bug
# identified in https://bugs.llvm.org/show_bug.cgi?id=48137. This bug presents
# itself as struct types that are defined in both files getting a unique suffix
# added in the diff, indicating incorrectly that two lines differ. As an
# example, you can run the following command (on osx):
#
# > /usr/local/opt/llvm@15/bin/llvm-diff \
# >   test/testdata/compiler/hello.llo.exp \
# >   test/testdata/compiler/hello.llo.exp
#
# This will indicate that there are multiple differences despite both arguments
# being the exact same file. For example, the diff reported below differs only
# in the use of `%struct.FunctionInlineCache`, where the right entry has a
# renamed version of the same struct present.
#
#   >   %send.i = call i64 @sorbet_callFuncWithCache(%struct.FunctionInlineCache.69* noundef @ic_puts, i64 0) #6, !dbg !4
#   <   %send.i = call i64 @sorbet_callFuncWithCache(%struct.FunctionInlineCache* noundef @ic_puts, i64 0) #6, !dbg !4

module DiffDiff

  def self.main(io)
    real_failure, lines = sanitize(io)
    if real_failure
      puts lines
      exit(1)
    end
  end

  LINE_ERROR = %r{llvm-diff: [^ ]+:\d+:\d+: error: }

  # Determine if this is a real failure, or differences introduced as a result
  # of the `llvm-diff` bug https://bugs.llvm.org/show_bug.cgi?id=48137
  def self.sanitize(io)
    real_failure = false

    right = []
    left = []
    in_diff = false

    lines = []

    io.each do |line|
      stripped = line.strip

      if stripped.start_with? '>'
        right << line
        in_diff = true
      elsif stripped.start_with? '<'
        left << line
        in_diff = true
      else
        if in_diff
          real_diff = sanitize_lines(lines, right, left)
          real_failure ||= real_diff

          left = []
          right = []
          in_diff = false
        end

        # encountering a function that only exists in one file or the other will
        # cause `llvm-diff` to return a non-zero exit code
        real_failure ||= line.include? 'exists only in'

        # encountering an error when parsing either file
        real_failure ||= line.match? LINE_ERROR

        lines << line
      end
    end

    # if the llvm-diff output ends with a diff, the right and left arrays will
    # need to be processed explicitly
    if in_diff
      real_diff = sanitize_lines(lines, right, left)
      real_failure ||= real_diff
    end

    [ real_failure, lines ]
  end

  # For all of the lines in the left and right sets, strip out suffixes that are
  # added by the bug in `llvm-diff`, and compare them for equality. If they are
  # equal after that sanitizing pass, indicate that this diff block did not
  # contribute to the overall diff failure and concatenate the sanitized lines
  # to the output
  def self.sanitize_lines(out, right, left)
    sanitized_left = left.map do |line|
      sanitize_line(line.sub('<', ' '))
    end

    sanitized_right = right.map do |line|
      sanitize_line(line.sub('>', ' '))
    end

    if sanitized_right == sanitized_left
      out.concat(sanitized_right)
      false
    else
      out.concat(right, left)
      true
    end
  end

  LOCAL_PATTERN = %r{(%struct\.[^\s]+)\.\d+}

  def self.sanitize_line(line)
    line.gsub(LOCAL_PATTERN, '\1')
  end

end

if __FILE__ == $0
  DiffDiff.main($stdin)
end
