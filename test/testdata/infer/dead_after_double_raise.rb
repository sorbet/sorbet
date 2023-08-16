# typed: strict
extend T::Sig

sig {void}
def example1
  begin
    x = 1
    if T.unsafe(nil)
      raise
    else
      raise
    end
    T.reveal_type(x)
  rescue
    T.reveal_type(x) # error: `NilClass`
  end
end

sig {void}
def example2
  begin
    y = 1
    if T.unsafe(nil)
      raise
    else
      raise
    end
    T.reveal_type(y)
  end
end
