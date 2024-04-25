# typed: true
# compiled: false
# frozen_string_literal: true

# It's important that the module that raises during static init is loaded inside
# of a method defined with `define_singleton_method` or `define_method`, not
# `def`. When using def there will be no exception raised.
define_singleton_method "test_case" do
  puts "before"

  # Without the exception handling here, the crash will not happen.
  begin
    # The compiled module that's loaded raises during its root static-init
    # method, triggering exception handling locally.
    require_relative './static_init_exception__2'
  rescue
  end

  puts "after"
end

T.unsafe(self).test_case
