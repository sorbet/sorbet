# typed: true
# enable-experimental-rbs-comments: true

# Signature comment before a non-method statement should be an error
#: (Integer) -> void # error: Unused RBS signature comment. No method definition found after it

puts "hello"

def foo(x)
  T.reveal_type(x) # error: Revealed type: `T.untyped`
end

# Signature comment before an assignment
#: (String) -> void # error: Unused RBS signature comment. No method definition found after it

x = 1

def bar(x)
  T.reveal_type(x) # error: Revealed type: `T.untyped`
end

# Normal case: signature directly before def should still work
#: (Integer) -> void
def baz(x)
  T.reveal_type(x) # error: Revealed type: `Integer`
end

# Normal case: blank line between sig and def still works
#: (String) -> void

def qux(x)
  T.reveal_type(x) # error: Revealed type: `String`
end

# Normal case: signature before visibility + def still works
#: (Integer) -> void
private def priv(x)
  T.reveal_type(x) # error: Revealed type: `Integer`
end
