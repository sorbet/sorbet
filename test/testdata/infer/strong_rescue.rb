# typed: strong
extend T::Sig

sig {void}
def example1
  begin
  rescue TypeError => e
    T.reveal_type(e) # error: `TypeError`
  else
  ensure
  end

  T.reveal_type(e) # error: `T.nilable(TypeError)`
end

sig {void}
def example2
  begin
  rescue; puts("")
  ensure
  end
end

sig {void}
def example3
  begin
  rescue => e; T.reveal_type(e)
    #          ^^^^^^^^^^^^^^^^ error: `StandardError`
  ensure
  end

  T.reveal_type(e) # error: `T.nilable(StandardError)`
end

sig {void}
def example4
  begin
    puts("here we are")
  rescue; puts("")
  ensure
  end
end

sig {void}
def example5
  begin
  rescue Exception => e
    T.reveal_type(e) # error: `Exception`
  else
  ensure
  end

  T.reveal_type(e) # error: `T.nilable(Exception)`
end

sig {void}
def example6
  begin
  rescue Exception => e
    T.reveal_type(e) # error: `Exception`
  else
  ensure
    e.foo
    # ^^^ error: Call to method `foo` on `T.untyped`
  end

  e.foo
  # ^^^ error: Call to method `foo` on `T.untyped`
end

class NotAnException
  # usually indicates logic bug or problem with RBI generation
end

sig {void}
def example7
  begin
  rescue NotAnException
    puts('hello') # error: This code is unreachable
  end
end

