# Don't put things here since our builds don't hit caches very often, we want them baked into an image.
# Instead put them in:
#   https://git.corp.stripe.com/stripe-internal/dev-vm/blob/master/dockerfiles/sorbet/Dockerfile.sorbet
# and then once it is on master do:
#   henson deploy --prod ubuntu-1604-sorbet-container

FROM 030465607062.dkr.ecr.us-west-2.amazonaws.com/stripe/build/ubuntu-16.04-sorbet:latest

CMD /src/jenkins-build.sh
