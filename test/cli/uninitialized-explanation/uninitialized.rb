# typed: true

extend T::Sig

sig {params(x: Integer).void}
def takes_int(x); end

def uninitialized_loc_is_decl_loc
  if T.unsafe(nil)
    x = 1
  else
    T.unsafe(self).raise "Unanalyzable raise"
  end

  takes_int(x)
end