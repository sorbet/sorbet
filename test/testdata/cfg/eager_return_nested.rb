# typed: true
extend  T::Sig

sig {params(x: String).returns(Integer)}
def example(x)
  begin
    puts 1
    begin
      puts 1
      begin
        puts 1
        if Random.rand(2).even?
          x # error: Expected `Integer` but found `String` for method result type
        else
          T.unsafe(nil)
        end
      end
    end
  end
end
