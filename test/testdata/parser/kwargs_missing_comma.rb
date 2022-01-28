# typed: true

class A
  def example1(slug:, token:, merchant:request:)
                                     # error: missing token ","
  end
  
  def example2(merchant: request:)
                      # ^ error: missing token ","
  end

  def example3(merchant:request:)
                      # error: missing token ","
    puts
  end

  def example4(merchant:request:, etc:)
                      # error: missing token ","
    puts
  end
end