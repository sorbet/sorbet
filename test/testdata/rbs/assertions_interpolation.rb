# typed: strict
# enable-experimental-rbs-comments: true

integer = T.let(nil, T.nilable(String))

result = "#{(
  integer #: as String
).upcase}"
T.reveal_type(result) # error: Revealed type: `String`

percent_string = %Q[
  #{(
    integer #: as String
  ).upcase}
]
T.reveal_type(percent_string) # error: Revealed type: `String`

heredoc = <<~STRING
  #{(
    integer #: as String
  ).upcase}
STRING
T.reveal_type(heredoc) # error: Revealed type: `String`

symbol = :"#{(
  integer #: as String
).upcase}"
T.reveal_type(symbol) # error: Revealed type: `Symbol`

regexp = /#{(
  integer #: as String
).upcase}/
T.reveal_type(regexp) # error: Revealed type: `Regexp`

xstring = `echo #{(
  integer #: as String
).upcase}`
T.reveal_type(xstring) # error: Revealed type: `String`

words = %W[
  #{(
    integer #: as String
  ).upcase}
]
T.reveal_type(words) # error: Revealed type: `[String] (1-tuple)`

symbols = %I[
  #{(
    integer #: as String
  ).upcase}
]
T.reveal_type(symbols) # error: Revealed type: `[Symbol] (1-tuple)`
