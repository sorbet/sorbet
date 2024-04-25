# frozen_string_literal: true
# typed: true
# compiled: true

# Our keywordArgsSingleton Hash trick only works so long as the global Hash is
# never visible to normal Ruby code. When it's unpacked to populate keyword
# args or dupped to populate a kwrest arg, the global singleton isn't visible
# to the callee. But when what looked like keyword args at the call site turned
# out to just be a Hash, it's possible to accidentally leak the singleton.

# Only takes positional args, not keywords
def id(x); x; end

def main
  y = id({foo: :bar})
  id({qux: 1})
  p y
end

main
