# typed: true

class A
  def example1(slug:, token:, merchant:request:)
                                     # error: missing "," between keyword args
  end
  
  def example2(merchant: request:)
                      # ^ error: missing "," between keyword args
  end

  def example3(merchant:request:)
                      # error: missing "," between keyword args
    puts
  end

  def example4(merchant:request:, etc:)
                      # error: missing "," between keyword args
    puts
  end

  def example5(merchant:request: etc:)
                      # error: missing "," between keyword args
                              # ^ error: missing "," between keyword args
    puts
  end
end
