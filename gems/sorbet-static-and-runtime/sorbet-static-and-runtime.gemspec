Gem::Specification.new do |s|
  s.name        = 'sorbet-static-and-runtime'
  s.version     = '0.0.0'
  s.summary     = 'A Typechecker for Ruby'
  s.description = 'Sorbet static and runtime in one gem'
  s.authors     = ['Stripe']
  s.email       = 'sorbet@stripe.com'
  s.homepage    = 'https://sorbet.org'
  s.license     = 'Apache-2.0'
  s.metadata = {
    'source_code_uri' => 'https://github.com/sorbet/sorbet'
  }

  s.add_dependency 'sorbet', '0.0.0'
  s.add_dependency 'sorbet-runtime', '0.0.0'

  s.required_ruby_version = ['>= 2.7.0']
end
