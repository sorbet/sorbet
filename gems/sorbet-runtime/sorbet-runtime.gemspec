Gem::Specification.new do |s|
  s.name        = 'sorbet-runtime'
  s.version     = '0.0.0'
  s.summary     = 'Sorbet runtime'
  s.description = "Sorbet's runtime type checking component"
  s.authors     = ['Stripe']
  s.files       = Dir.glob('lib/**/*')
  s.homepage    = 'https://sorbet.run'

  s.add_development_dependency 'minitest', '~> 5.11'
  s.add_development_dependency 'mocha', '~> 1.7'
  s.add_development_dependency 'rake'

  # TODO this is temporary, to prevent leaking publicly.
  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the 'allowed_push_host'
  # to allow pushing to a single host or delete this section to allow pushing to any host.
  raise 'RubyGems 2.0 or newer is required to protect against public gem pushes.' unless s.respond_to?(:metadata)
  s.metadata['allowed_push_host'] = ''
end
