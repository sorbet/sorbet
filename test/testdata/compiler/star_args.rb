# typed: true
# compiled: true

def has_only_star_args(*args)
  puts args.inspect
end
has_only_star_args(1)
has_only_star_args(1,2,3)
has_only_star_args(1,2,3, k:2)

def has_star_arg_and_positional(p, *args)
  puts p, args.inspect
end

has_star_arg_and_positional(1)
has_star_arg_and_positional(1,2,3)
has_star_arg_and_positional(1,2,3, k:2)

def has_star_arg_and_positional_and_default(p, d=2, *args)
  puts p, d, args.inspect
end

has_star_arg_and_positional_and_default(1)
has_star_arg_and_positional_and_default(1,2)
has_star_arg_and_positional_and_default(1,k:2)
has_star_arg_and_positional_and_default(1,2,3)
has_star_arg_and_positional_and_default(1,2,3, k:2)


