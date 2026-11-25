# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Test::Simpsons < PackageSpec
  test!

  import Simpsons, uses_internals: true
  import Krabappel
  #      ^^^^^^^^^ usage: krabappel-pkg
  #      ^^^^^^^^^ hover: Bart's teacher
  #      ^         show-symbol: Krabappel
end
