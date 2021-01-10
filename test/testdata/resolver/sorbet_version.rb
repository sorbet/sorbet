# typed: true

# Version number is obviously brittle, so we show just enough here to know that
# this constant is set to a string literal.

T.reveal_type(Sorbet::Private::Static::VERSION) # error: Revealed type: `String("
