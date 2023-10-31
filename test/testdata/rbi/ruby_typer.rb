# typed: true

# Sorbet::Private::Static::IOLike is a type alias defined in our payload. We used to not
# handle type aliases defined in our payload correctly, but now we do.

extend T::Sig

sig {params(x: Sorbet::Private::Static::IOLike).void}
def foo(x)
  T.reveal_type(x) # error: Revealed type: `T.any(IO, StringIO, Tempfile)`
end
