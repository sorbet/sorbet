# typed: true
extend T::Sig

def example1
  begin
    x = 0
  rescue TypeError => e
  rescue => e
    raise
  end

  T.reveal_type(x) # error: `T.nilable(Integer)`
  T.reveal_type(e) # error: `T.nilable(TypeError)`
end

def example2
  begin
    x = 0
  rescue TypeError => e
    raise
  rescue => e
  end

  T.reveal_type(x) # error: `T.nilable(Integer)`
  T.reveal_type(e) # error: `T.nilable(StandardError)`
end

sig { params(x: T::Boolean).void }
def example3(x:)
  begin
    puts
  rescue TypeError => e
    if x
      raise
    else
      return
    end
  rescue => e
    raise
  end

  # all paths through the function that set `e` raise or return, so `e` is
  # effectively uninitialized
  T.reveal_type(e) # error: `NilClass`
end
