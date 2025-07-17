# typed: false

1..2 # Include end

3...4 # Exclude end

# Endless ranges need to be parenthesized
("Has no end.."..)
("Has no end..."...)

# Sorbet's parser can't handle these without parentheses, so we use them for parity
(.."..Has no begin")
(..."...Has no begin")
