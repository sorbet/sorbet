# typed: true

md_file = Pathname.new('/usr/local/Homebrew').find do |pn|
  T.reveal_type(pn) # error: `Pathname`
end
md_file = Pathname.new('/usr/local/Homebrew').find do |pn|
  break pn if pn.to_s.end_with?('.md')
end
T.reveal_type(md_file) # error: `T.nilable(Pathname)`

T.reveal_type(Pathname('/')) # error: `Pathname`
T.reveal_type(Pathname(Pathname('/'))) # error: `Pathname`

T.reveal_type(Pathname.glob(['*.rb', Pathname('/')])) # error: `T::Array[Pathname]`
T.reveal_type(Pathname.glob(['*.rb', Pathname('/')]) { puts _1 }) # error: `NilClass`

T.reveal_type(Pathname('/usr/bin').glob(['*.d', Pathname('/')])) # error: `T::Array[Pathname]`
T.reveal_type(Pathname('/usr/bin').glob(['*.d', Pathname('/')]) { puts _1 }) # error: `NilClass`
