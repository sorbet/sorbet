# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    import Bart
    #      ^^^^ def: bart
    #      ^^^^ usage: bartpkg
    export Family
    #      ^^^^^^ usage: family
end
