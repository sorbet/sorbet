Gem::Specification.new do |s|
  s.name        = 'sorbet-runtime'
  s.version     = '0.0.0'
  s.summary     = 'Sorbet runtime'
  s.description = "Sorbet's runtime type checking component"
  s.authors     = ['Stripe']
  s.files       = Dir.glob('lib/**/*')
  s.homepage    = 'https://sorbet.org'
  s.license     = 'Apache-2.0'
  s.metadata = {
    "source_code_uri" => "https://github.com/sorbet/sorbet",
  }

  s.required_ruby_version = ['>= 2.7.0']

  s.add_development_dependency 'minitest', '~> 5.11'
  s.add_development_dependency 'mocha', '~> 2.1'
  s.add_development_dependency 'rake'
  s.add_development_dependency 'rubocop', '1.57.1'
  s.add_development_dependency 'rubocop-performance', '1.13.2'
  # for reproducing race conditions in tests
  s.add_development_dependency 'concurrent-ruby', '~> 1.1.5'
  s.add_development_dependency 'pry'
  s.add_development_dependency 'pry-byebug'
  # for running ruby subprocesses
  s.add_development_dependency 'subprocess', '~> 1.5.3'
end
