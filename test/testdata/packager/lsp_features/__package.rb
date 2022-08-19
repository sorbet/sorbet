# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    # ^ show-symbol: Simpsons
    # go-to-def on a reference to Bart within the package goes here, but go-to-def on Bart in `import Bart` goes to the
    # package.
    import Bart
    #      ^^^^ import: bartpkg
    #      ^^^^ hover: Bart package description
    #      ^    show-symbol: Bart

    test_import Krabappel
    #           ^^^^^^^^^ import: krabappel-pkg
    #           ^^^^^^^^^ hover: Bart's teacher
    #           ^         show-symbol: Krabappel

    export Simpsons::Family
    #                ^^^^^^ usage: family
    #                ^      show-symbol: Simpsons::Family

    export_for_test Simpsons::Private
  # ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
    #               ^^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
    #                         ^^^^^^^ usage: s-private
    #                         ^       show-symbol: Simpsons::Private
end
