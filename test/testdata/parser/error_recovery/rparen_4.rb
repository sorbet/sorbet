# typed: false

class A
  def before; end
  def foo
    puts 'before'
    T.any(Integer, String
    puts 'after'
  # ^^^^ error: unexpected token tIDENTIFIER
  end
# ^^^ error: unterminated (
  def after; end
end
