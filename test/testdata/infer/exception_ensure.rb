# typed: true

# For the time being, getting well-typed exception values in the presence of
# `ensure` blocks is a bit too tricky, because of how Sorbet desugars
# exceptional control flow.

def example1
  begin
    puts
  rescue => e
    puts
  ensure
    puts
  end

  T.reveal_type(e) # error: `T.untyped`
end

def example2
  begin
    puts
  rescue => e
    puts
  end

  T.reveal_type(e) # error: `T.nilable(StandardError)`
end


