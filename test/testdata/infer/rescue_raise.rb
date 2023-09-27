# typed: true

# Sorbet doesn't have the best support when it comes to unconditional
# raise/return inside exception handling blocks. This file aims to show some of
# those problems.
#
# Some relevant issues:
# - https://github.com/sorbet/sorbet/issues/3501
# - https://github.com/sorbet/sorbet/issues/4108
# - https://github.com/sorbet/sorbet/issues/7194

def example1
  begin
    x = 1
    raise
  rescue => e
    T.reveal_type(e) # error: `StandardError`
    T.reveal_type(x) # error: `NilClass`
  end

  T.reveal_type(e) # error: `StandardError`
  T.reveal_type(x) # error: `NilClass`
end

def example2
  begin
    x = 1
    while true
      raise
    end
  rescue => e
    T.reveal_type(e) # error: `StandardError`
    T.reveal_type(x) # error: `NilClass`
  end

  T.reveal_type(e) # error: `StandardError`
  T.reveal_type(x) # error: `NilClass`
end

def example3
  res =
    begin
      x = 1.to_i
      x
    rescue => e
      raise
    ensure
      T.reveal_type(x) # error: `T.nilable(Integer)`
      T.reveal_type(e) # error: `T.untyped`
      'not used'
    end
  T.reveal_type(x) # error: `Integer`
  T.reveal_type(e) # error: `T.untyped`
  T.reveal_type(res) # error: `Integer`
end

def example4
  res =
    begin
      x = 1.to_i
      x
    rescue => _e
      raise
    ensure
    end
  T.reveal_type(x) # error: `Integer`
  T.reveal_type(res) # error: `Integer`
end

def example5
  res =
    begin
      x = 1.to_i
      x
    rescue => e
      raise
    ensure
      T.reveal_type(e) # error: `T.untyped`
      T.reveal_type(x) # error: `T.nilable(Integer)`
    end
  T.reveal_type(x) # error: `Integer`
  T.reveal_type(res) # error: `Integer`
end

def example6
  if T.unsafe(nil)
    begin
      puts
    rescue TypeError => e
      T.reveal_type(e) # error: `TypeError`
    end
  else
    begin
      1.times do
        begin
          puts
        rescue TypeError => e
          #                 ^ error: Changing the type of a variable in a loop is not permitted
          T.reveal_type(e) # error: `T.untyped`
        end
      end
    end
  end
  T.reveal_type(e) # error: `T.nilable(TypeError)`
end
