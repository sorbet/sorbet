# frozen_string_literal: true
# typed: true
# compiled: true
puts (/cat/.match("cat"))
puts (/cat/.match("CAT"))
puts (/cat/i.match("cat"))
puts (/cat/i.match("CAT"))
puts (/abc # Comment/.match("abc"))
puts (/abc # Comment/x.match("abc"))
