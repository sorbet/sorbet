# typed: strong
extend T::Sig

# Sorbet's implementation of exceptions currently heavily relies on T.untyped
# Fixing that involves more work than I have time for, so let's at least just
# not spam an entire method with untyped squiggles.

sig {void}
def example1
  begin
  rescue TypeError => e
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Conditional branch on `T.untyped`
    #    ^^^^^^^^^ error: Argument passed to parameter `other` is `T.untyped`
    T.reveal_type(e) # error: `TypeError`
  else
  ensure
  end
end

sig {void}
def example2
  begin
  rescue; puts("")
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Argument passed to parameter `other` is `T.untyped`
  ensure
  end
end

sig {void}
def example3
  begin
  rescue => e; T.reveal_type(e)
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Conditional branch on `T.untyped`
    #       ^ error: Argument passed to parameter `other` is `T.untyped`
    #          ^^^^^^^^^^^^^^^^ error: `StandardError`
  ensure
  end
end

sig {void}
def example4
  begin
    puts("here we are")
  rescue; puts("")
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Argument passed to parameter `other` is `T.untyped`
  ensure
  end
end

sig {void}
def example5
  begin
  rescue Exception => e
# ^^^^^^ error: Conditional branch on `T.untyped`
# ^^^^^^ error: Conditional branch on `T.untyped`
    #    ^^^^^^^^^ error: Argument passed to parameter `other` is `T.untyped`
    T.reveal_type(e) # error: `Exception`
  else
  ensure
  end
end
