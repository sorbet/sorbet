# typed: false

class A
  def test1
    if x.f
    end
  end

  def test2
# ^^^ parser-error: Hint: this "def" token might not be properly closed
    puts 'before'
    if x
  # ^^ parser-error: Hint: this "if" token might not be properly closed
    puts 'after'
end # parser-error: unexpected token "end of file"
