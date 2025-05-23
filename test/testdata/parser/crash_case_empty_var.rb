# typed: true

# Crash with an empty string literal in a case-in expression.
case T.unsafe(nil); in {"":} then true; end
                      # ^^^ error: key must be valid as local variables
