# typed: true

class I; end
class S; end
class A
  extend T::Sig
  sig {params(x: I).void}
  sig {params(x: S).void}
  def my_method(x); end
end

A.new.my_metho # error: does not exist
#             ^ completion: my_method, my_method, my_method, my_method, my_method

# TODO(jez) The above is not the right behavior.
#
# There are 3 symbols here. One of them was mangle renamed?
# Looks like we mangleRenameSymbol in resolver before fillInInfoFromSig.
#
# class ::A < ::Object () @ overloads_test.rb:5
#   method ::A#foo$1 (x, <blk>) @ overloads_test.rb:9
#     argument x<> @ Loc {file=overloads_test.rb start=9:11 end=9:12}
#     argument <blk><block> @ Loc {file=overloads_test.rb start=??? end=???}
#   method ::A#foo (overload.1) (x, <blk>) -> Sorbet::Private::Static::Void @ overloads_test.rb:8
#     argument x<> -> S @ Loc {file=overloads_test.rb start=8:15 end=8:16}
#     argument <blk><block> -> T.untyped @ Loc {file=??? start=??? end=???}
#   method ::A#foo (x, <blk>) -> Sorbet::Private::Static::Void @ overloads_test.rb:9
#     argument x<> -> I @ Loc {file=overloads_test.rb start=7:15 end=7:16}
#     argument <blk><block> -> T.untyped @ Loc {file=??? start=??? end=???}
