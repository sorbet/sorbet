# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    # ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "(nothing)"
    # ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "Simpsons"
    # ^^^^^^^^ symbol-search: "Simpsons", name = "Simpsons", container = "<PackageTests>::Simpsons_Package_Private"
    # ^ show-symbol: Simpsons
    # go-to-def on a reference to Bart within the package goes here, but go-to-def on Bart in `import Bart` goes to the
    # package.
    import Bart
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
    #                         ^       show-symbol: Simpsons::Private
end
