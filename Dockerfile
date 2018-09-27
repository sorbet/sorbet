FROM 030465607062.dkr.ecr.us-west-2.amazonaws.com/stripe/build/ubuntu-14.04_ruby:latest

RUN add-apt-repository ppa:openjdk-r/ppa && apt-get update && apt-get install -y openjdk-8-jdk zip
RUN update-java-alternatives -s java-1.8.0-openjdk-amd64
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
ENV PATH $JAVA_HOME/bin:$PATH
RUN export JAVA_HOME

RUN add-apt-repository ppa:ubuntu-toolchain-r/test && apt-get update

RUN apt-get update && apt-get install -y realpath time

# Dependencies for the parser
RUN apt-get update && apt-get install -y ragel bison ruby autoconf

# Dependencies for pay-server gems
RUN apt-get update && apt-get install -y libpq-dev libicu-dev libsqlite3-dev

RUN rbenv exec gem install bundler --no-rdoc --no-ri

ADD https://shellcheck.storage.googleapis.com/shellcheck-v0.5.0.linux.x86_64.tar.xz /tmp
RUN tar -xf /tmp/shellcheck-v0.5.0.linux.x86_64.tar.xz -C /usr/local/bin --strip-components=1 shellcheck-v0.5.0/shellcheck

WORKDIR /src

ENV PATH $PATH:/src/

CMD /src/jenkins-build.sh
