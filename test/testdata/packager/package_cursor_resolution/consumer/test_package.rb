# typed: strict
# enable-packager: true
# assert-no-code-action: quickfix

module Consumer
  Only::Test::Thing
# ^^^^^^^^^^ error: `Only::Test` cannot be referenced here because `Consumer` may not reference `test!` packages
end
