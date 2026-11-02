# typed: strict
# enable-packager: true
# enable-package-directed: true
# stratum: 1
class Consumer < PackageSpec
  import Producer
  #      ^^^^^^^^ import: producer-pkg
  #      ^^^^^^^^ hover: Producer package description
  #      ^ show-symbol: Producer
end
