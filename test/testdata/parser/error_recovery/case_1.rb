# typed: false

class A
  def test1
    # These errors are currently doubled because we actually report this error
    # on the first and second parse. (Most of the time, e.g. in files where
    # this is the only parse error, this syntax error alone won't force us into
    # indentation-aware recovery mode, so I've lazily decided this is good
    # enough. If you've made it better that's great!)
    puts 'before'
    case
  # ^^^^ error: "case" statement must at least have one "when" clause
  # ^^^^ error: Hint: this "case" token might not be properly closed
  # ^^^^ error: Hint: this "case" token might not be properly closed
  end

  def test2
    puts 'before'
    case x # error: unexpected token "case"
    puts 'after'
  end

  def test3
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when
    puts 'after' # error: unexpected token tSTRING
  end

  def test4
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when A
    puts 'after'
  end

  def test5
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when A then
    puts 'after'
  end

  def test6
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when then # error: unexpected token "then"
    puts 'after'
  end

  def test7
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    when A
    else
    puts 'after'
  end

  def test8
    puts 'before'
    case # error: Hint: this "case" token might not be properly closed
    when A
    else
    puts 'after'
  end

  def test9
    puts 'before'
    case x # error: Hint: this "case" token might not be properly closed
    in A
    else
    puts 'after'
  end
end # error: unexpected token "end of file"
