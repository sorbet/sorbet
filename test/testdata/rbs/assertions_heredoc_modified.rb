# typed: strict
# enable-experimental-rbs-comments: true

# Modified for Prism, contains a minor difference in the exp file for HEREDOC14
# ("  ", 42, "\n", "  ", 42, "\n") vs ("  ", 42, "\n  ", 42, "\n")

HEREDOC1 = <<~MSG.strip # error: Constants must have type annotations with `T.let` when specifying `# typed: strict
  #: Integer
MSG

HEREDOC2 = "<<~ #: Integer".strip # error: Constants must have type annotations with `T.let` when specifying `# typed: strict

HEREDOC3 = <<~MSG
  #: Integer
MSG
T.reveal_type(HEREDOC3) # error: Revealed type: `String`

HEREDOC4 = <<~MSG #: String?
  foo
MSG
T.reveal_type(HEREDOC4) # error: Revealed type: `T.nilable(String)`

HEREDOC5 = <<~MSG.strip.strip #: String?
  #{42}
MSG
T.reveal_type(HEREDOC5) # error: Revealed type: `T.nilable(String)`

HEREDOC6 = String(<<~MSG.strip) #: String?
  foo
MSG
T.reveal_type(HEREDOC6) # error: Revealed type: `T.nilable(String)`

HEREDOC7 = <<~MSG #: String?
  foo
  bar
  baz
MSG
T.reveal_type(HEREDOC7) # error: Revealed type: `T.nilable(String)`

HEREDOC8 = <<~MSG #: String?
MSG
T.reveal_type(HEREDOC8) # error: Revealed type: `T.nilable(String)`

HEREDOC9 = <<-MSG #: String?
MSG
T.reveal_type(HEREDOC9) # error: Revealed type: `T.nilable(String)`

HEREDOC10 = <<-'MSG' #: String?
  foo
MSG
T.reveal_type(HEREDOC10) # error: Revealed type: `T.nilable(String)`

HEREDOC11 = <<-"MSG" #: String?
  foo
MSG
T.reveal_type(HEREDOC11) # error: Revealed type: `T.nilable(String)`

HEREDOC12 = <<-MSG#: String?
  foo
MSG
T.reveal_type(HEREDOC12) # error: Revealed type: `T.nilable(String)`

HEREDOC13 = <<-MSG#: String?
  #{42}
MSG
T.reveal_type(HEREDOC13) # error: Revealed type: `T.nilable(String)`

HEREDOC14 = <<-MSG#: String?
  #{42}
  #{42}
MSG
T.reveal_type(HEREDOC14) # error: Revealed type: `T.nilable(String)`

HEREDOC15 = <<-MSG, <<-MSG2#: String? # error: Argument does not have asserted type `T.nilable(String)`
  foo
MSG
  bar
MSG2
T.reveal_type(HEREDOC15) # error: Revealed type: `T.nilable(String)`
