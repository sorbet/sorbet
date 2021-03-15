# typed: true

Resolv::DNS.open do |resolv|
  T.assert_type!(resolv, Resolv::DNS)
end

T.assert_type!(Resolv::DNS.open, Resolv::DNS)
