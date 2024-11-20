# frozen_string_literal: true
# typed: true
# compiled: true

def func_a
  if T.unsafe(35 > 36)
    return 209
  end

  begin
    if T.unsafe(false)
      raise "oh no"
    end
    return 3
  rescue
    return 3.times { 444 }
  ensure
    return 822
  end
end

def func_b
  1.times { return 33 }
end

def justyield
  yield
end

def func_c
  if T.unsafe(3+3 > 5)
    puts (justyield { return 33 })
  end
end

def func_d
  if T.unsafe(3+3 > 5)
    puts (justyield { 33 })
  end
end
