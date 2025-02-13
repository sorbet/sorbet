# typed: true

class A
  def example
    example{&:"{}"}
    #      ^^^^^^^^ parser-error: block pass should not be enclosed in curly braces
  end
end
