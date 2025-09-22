# frozen_string_literal: true
# typed: strict
# compiled: true


class MyEnum < T::Enum
  enums do
    A = new
    B = new
    C = new
    D = new
  end
end

class Main
  extend T::Sig

  sig {params(x: T.any(MyEnum::A, MyEnum::B)).returns(BasicObject)}
  def self.takes_a_or_b(x); x end

  sig {params(x: MyEnum::C).returns(BasicObject)}
  def self.takes_c(x); x end

  sig {params(x: MyEnum).returns(BasicObject)}
  def self.some_common_cases(x)
    case x
    when MyEnum::A, MyEnum::B
      takes_a_or_b(x)
    when MyEnum::C
      takes_c(x)
    else
      x
    end
  end
end

Main.takes_a_or_b(T.unsafe(MyEnum::A))
Main.takes_a_or_b(T.unsafe(MyEnum::B))
begin
  Main.takes_a_or_b(T.unsafe(MyEnum::C))
rescue TypeError => exn
  puts exn.message.match(/Expected.*/).to_s
end

begin
  Main.takes_c(T.unsafe(MyEnum::A))
rescue TypeError => exn
  puts exn.message.match(/Expected.*/).to_s
end
Main.takes_c(T.unsafe(MyEnum::C))

Main.some_common_cases(T.unsafe(MyEnum::A))
Main.some_common_cases(T.unsafe(MyEnum::B))
Main.some_common_cases(T.unsafe(MyEnum::C))
Main.some_common_cases(T.unsafe(MyEnum::D))
