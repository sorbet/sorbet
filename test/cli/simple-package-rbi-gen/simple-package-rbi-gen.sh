cd test/cli/simple-package-rbi-gen || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1

# TODO(jez) duplicate this test to test just the minimize parts
# We're using temp files because there was some stdout/stderr syncing problems

stderr_log="$(mktemp)"
minimize_rbi="$(mktemp)"
trap 'rm -rf "$stderr_log" "$minimize_rbi"' EXIT

cat > "$minimize_rbi" <<RUBY
# typed: true
module Project::Foo
  class FooClass
    extend T::Sig
    def initialize(value)
    end
    def dynamically_defined_method
    end
  end
end
RUBY

../../../main/sorbet --silence-dev-message --stripe-packages . --minimize-to-rbi="$minimize_rbi" --print=minimized-rbi
