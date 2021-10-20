# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    # ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "(nothing)"
    # ^ show-symbol: Simpsons
    # go-to-def on a reference to Bart within the package goes here, but go-to-def on Bart in `import Bart` goes to the
    # package.
    import Bart
    #      ^^^^ symbol-search: "Bart", name = "Bart", container = "Simpsons"
    #      ^^^^ def: bart 1 not-def-of-self
    #      ^^^^ usage: bartpkg
    #      ^^^^ hover: Bart package description
    #      ^    show-symbol: Bart

    test_import Krabappel
    #           ^^^^^^^^^ usage: krabappel-pkg
    #           ^^^^^^^^^ hover: Bart's teacher
    #           ^         show-symbol: Krabappel

    export Simpsons::Family
    #                ^^^^^^ usage: family
    #                ^      show-symbol: Simpsons::Family

    export_for_test Simpsons::Private
    #                         ^^^^^^^ usage: s-private
    #               ^^^^^^^^^^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "<PackageTests>::Simpsons_Package"
    #                         ^       show-symbol: Simpsons::Private
    #               ^                 show-symbol: Simpsons
end
