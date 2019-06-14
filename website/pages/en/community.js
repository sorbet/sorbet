const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');

const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

const PageSection = require(`${process.cwd()}/core/PageSection.js`);

const TalkListItem = (props) => (
  <li>
    <a href={props.link}>{props.title}</a> at {props.venue}
  </li>
);

const TalkList = (props) => <ul>{props.content.map(TalkListItem)}</ul>;

const ProjectListItem = (props) => {
  var description = '';
  if (props.description != null && props.description != '') {
    description = ' - ' + props.description;
  }

  return (
    <li>
      <a href={props.link}>{props.title}</a>
      {description}
    </li>
  );
};

const ProjectList = (props) => <ul>{props.content.map(ProjectListItem)}</ul>;

class Index extends React.Component {
  render() {
    return (
      <div>
        <div className="homeContainer">
          <div className="homeSplashFade">
            <div className="wrapper homeWrapper">
              <div className="inner">
                <h2
                  className="projectTitle"
                  style={{maxWidth: '850px', textAlign: 'left'}}
                >
                  Community
                </h2>
                <p style={{paddingTop: '1em'}}>
                  Stay up to date with the Sorbet community!
                </p>
              </div>
            </div>
          </div>
          <PageSection align="center" className="featuresContainer" lightPurple>
            <GridBlock
              layout="threeColumn"
              contents={[
                {
                  title: `Get Help`,
                  content: `Ask a question on the [Sorbet Stack Overflow](https://stackoverflow.com/questions/tagged/sorbet) community.`,
                },
                {
                  title: `Come Chat`,
                  content: `Join the conversation on our [Slack](https://join.slack.com/t/sorbet-ruby/shared_invite/enQtNjUwNzY0MTQ1MTg2LWYzZjJhZGM2OTgzMTJiOTRmZDA0OTI1YjU5ZjU1NzQxM2MyZTFhOWNkNDE0ODQ3N2ViOTQwZjE4Y2IzOGEzYTg) instance.`,
                },
                {
                  title: `Report Issues`,
                  content: `Found a bug? Report it on the [Sorbet Github repo](https://github.com/stripe/sorbet/issues).`,
                },
              ]}
            />
          </PageSection>
        </div>
        <PageSection gray short>
          <div>
            <h1>Talks</h1>
            <TalkList
              content={[
                {
                  title: 'A practical type system for Ruby at Stripe',
                  link: '/docs/talks/ruby-kaigi-2018',
                  venue: 'RubyKaigi 2018',
                },
                {
                  title: 'Gradual typing of Ruby at Scale',
                  link: '/docs/talks/strange-loop-2018',
                  venue: 'Strange Loop 2018',
                },
                {
                  title: 'State of Sorbet: A type checker for Ruby',
                  link: '/docs/talks/ruby-kaigi-2019',
                  venue: 'RubyKaigi 2019',
                },
              ]}
            />
          </div>
          <div>
            <h1>Projects</h1>
            <ProjectList
              content={[
                {
                  title: 'sorbet-typed',
                  link: 'https://github.com/sorbet/sorbet-typed',
                  description:
                    'A central repository for sharing type definitions for Ruby gems',
                },
                {
                  title: 'sorbet-rails',
                  link: 'https://github.com/chanzuckerberg/sorbet-rails',
                  description:
                    'A set of tools to make sorbet work with rails seamlessly',
                },
              ]}
            />
          </div>
        </PageSection>
        <PageSection className="nav-footer" short lightPurple>
          <p className="footer">
            © 2019 Stripe{' · '}
            <a href="docs/adopting">Get started</a>
            {' · '}
            <a href="docs/">Docs</a>
            {' · '}
            <a href="https://sorbet.run">Try</a>
            {' · '}
            <a href="en/community">Community</a>
            {' · '}
            <a href="blog/">Blog</a>
          </p>
        </PageSection>
      </div>
    );
  }
}

Index.title = 'Community';

module.exports = Index;
