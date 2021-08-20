# frozen_string_literal: true
# typed: strict
# enable-packager: true

# Bart's teacher
class Krabappel < PackageSpec
#     ^^^^^^^^^ def: krabappel-pkg
    export Test::Krabappel::Popquiz
    #                       ^^^^^^^ usage: popquiz
end