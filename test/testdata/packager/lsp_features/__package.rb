# typed: strict
# enable-packager: true

class Simpsons < PackageSpec
    # go-to-def on a reference to Bart within the package goes here, but go-to-def on Bart in `import Bart` goes to the
    # package.
    import Bart
    #      ^^^^ def: bart 1 not-def-of-self
    #      ^^^^ usage: bartpkg
    export Family
    #      ^^^^^^ usage: family
end
