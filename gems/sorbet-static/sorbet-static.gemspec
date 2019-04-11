Gem::Specification.new do |s|
  s.name        = 'sorbet-static'
  s.version     = '0.0.0'
  s.summary     = 'A Typechecker for Ruby'
  s.description = 'The Sorbet typechecker binary'
  s.authors     = ['Stripe']
  s.email       = 'sorbet@stripe.com'
  s.files       = ['libexec/sorbet']
  s.executables = []
  s.homepage    = 'https://sorbet.run'
  s.license     = 'Apache-2.0'

  # We include a pre-built binary (in libexec/), making us platform dependent.
  s.platform = Gem::Platform::CURRENT

  # TODO this is temporary, to prevent leaking publicly.
  # Prevent pushing this gem to RubyGems.org. To allow pushes either set the 'allowed_push_host'
  # to allow pushing to a single host or delete this section to allow pushing to any host.
  raise 'RubyGems 2.0 or newer is required to protect against public gem pushes.' unless s.respond_to?(:metadata)
  s.metadata['allowed_push_host'] = ''
end
