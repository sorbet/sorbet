# typed: true

class A
  def example1(slug:, token:, merchant:request:)
  end
  
  def example2(merchant: request:)
  end

  def example3(merchant:request:)
    puts
  end

  def example4(merchant:request:, etc:)
    puts
  end

  def example5(merchant:request: etc:)
    puts
  end
end
