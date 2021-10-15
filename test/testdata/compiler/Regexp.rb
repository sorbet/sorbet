# frozen_string_literal: true
# typed: true
# compiled: true
puts (/cat/.match("cat"))
puts (/cat/.match("CAT"))
puts (/cat/i.match("cat"))
puts (/cat/i.match("CAT"))
puts (/cat # Comment/ix.match("CAT"))
puts (/abc # Comment/.match("abc"))
puts (/abc # Comment/x.match("abc"))
puts (/abc.abc/m.match("abc\nabc"))
puts (/abc.abc # Comment/mx.match("abc\nabc"))
puts (/abc.abc # Comment/mxi.match("ABC\nABC"))
