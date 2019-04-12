# typed: true

extend T::Sig

sig {params(blk: T.nilable(Proc)).void}
def f(&blk)
  if block_given?
    yield
    blk.call
  end

  T.assert_type!(block_given?, T::Boolean)
end


if block_given? # used to crash us
  puts("1")
end

yield 1 # used to crash us too
