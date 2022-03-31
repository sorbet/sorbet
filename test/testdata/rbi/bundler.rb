# typed: true

require "bundler"

Bundler.with_clean_env { true }
Bundler.with_original_env { true }
Bundler.with_unbundled_env { true }

Bundler.with_clean_env
#                     ^ error: `with_clean_env` requires a block parameter, but no block was passed

Bundler.with_original_env
#                        ^ error: `with_original_env` requires a block parameter, but no block was passed

Bundler.with_unbundled_env
#                         ^ error: `with_unbundled_env` requires a block parameter, but no block was passed
