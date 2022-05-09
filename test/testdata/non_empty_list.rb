# typed: true

extend T::Sig

sig { params(a: T::NonEmptyArray[Integer]).void }
def main(a:)
  puts a
end

l = []
main(a: l)