# typed: true
# enable-experimental-rbs-comments: true

#: (x, y) -> void
#   ^ error: Argument kind mismatch for `*`, method declares `rest positional`, but RBS signature declares `positional`
#      ^ error: Argument kind mismatch for `**`, method declares `rest keyword`, but RBS signature declares `positional`
def invalid_duplicate_no_keywords(*, **, **nil); end
#                                       ^^^^^ error: unexpected **nil; no keywords marker disallowed after keywords
#                                     ^^^^^^^ error: unexpected multiple `**` splat parameters

#: (x, y) -> void
#  ^^^^^^^^^^^^^^ error: RBS signature has more parameters than in the method definition
def no_keywords_is_not_a_parameter(*, **nil); end
#   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: The method `no_keywords_is_not_a_parameter` does not have a `sig`
