# typed: strict

module Root::Downstream
  # not allowed because `Root` is not imported
  Root::Included::STUFF # error: `Root::Included::STUFF` resolves

  # allowed because `Root::Ignored` is defined in a file that's
  # deliberately ignored
  Root::Ignored::STUFF
end
