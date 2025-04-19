# typed: true

class A
  def example1(slug:, token:, merchant:request:)
                                     # parser-error: missing "," between keyword args
  end
  
  def example2(merchant: request:)
                      # ^ parser-error: missing "," between keyword args
  end

  def example3(merchant:request:)
                      # parser-error: missing "," between keyword args
    puts
  end

  def example4(merchant:request:, etc:)
                      # parser-error: missing "," between keyword args
    puts
  end

  def example5(merchant:request: etc:)
                      # parser-error: missing "," between keyword args
                              # ^ parser-error: missing "," between keyword args
    puts
  end
end
