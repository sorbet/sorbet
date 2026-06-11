# frozen_string_literal: true
# typed: strict

class Test::Krabappel < PackageSpec
  test!

  import Krabappel
  #      ^^^^^^^^^ usage: krabappel-pkg

  export Test::Krabappel::Popquiz
  #                       ^^^^^^^ usage: popquiz
end
