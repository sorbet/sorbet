Gem::Specification.new do |s|
  s.name        = 'sorbet'
  s.version     = '0.0.0'
  s.summary     = 'A Typechecker for Ruby'
  s.description = 'The main entrypoint for using Sorbet'
  s.authors     = ['Stripe']
  s.email       = 'sorbet@stripe.com'
  s.files       = Dir.glob('lib/**/*')
  s.executables = Dir.glob('bin/**/*').map {|path| path.gsub('bin/', '')}
  s.homepage    = 'https://sorbet.org'
  s.license     = 'Apache-2.0'
  s.metadata = {
    "source_code_uri" => "https://github.com/sorbet/sorbet"
  }

  s.add_dependency 'sorbet-static', '0.0.0'

  s.required_ruby_version = ['>= 2.7.0']

  s.add_development_dependency 'minitest', '~> 5.11'
  s.add_development_dependency 'mocha', '~> 1.7'
  s.add_development_dependency 'rake'
end
