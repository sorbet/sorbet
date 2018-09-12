# typed: true
module A3; end
module C3 # C3, A3
  include A3
end
module D3 # D3, A3
  include A3
end
module E3 # Take E3, append C3 to end, Append-filter D3 next. E3, D3, C3, A3
  include C3
  include D3
end

# E3.ancestors    => [E3, D3, C3, A3]
