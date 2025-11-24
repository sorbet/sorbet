# typed: false

  def foo
    puts 'before'
    T.any(Integer, String
    puts 'after'
  # ^^^^ error: unexpected token tIDENTIFIER
  end
