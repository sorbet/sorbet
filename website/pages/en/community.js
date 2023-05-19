const React = require('react');

const CompLibrary = require('../../core/CompLibrary.js');

const Container = CompLibrary.Container;
const GridBlock = CompLibrary.GridBlock;

const PageSection = require(`${process.cwd()}/core/PageSection.js`);

const TalkListItem = (props) => (
  <li key={props.link}>
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
    <li key={props.link}>
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
                  Follow us{' '}
                  <a href="https://twitter.com/sorbet_ruby">on Twitter</a> for
                  announcements!
                </p>
              </div>
            </div>
          </div>
          <PageSection align="center" className="featuresContainer" lightPurple>
            <GridBlock
              layout="threeColumn"
              contents={[
                {
                  title: `Ask questions`,
                  content: `The Sorbet team monitors questions on Stack Overflow with the tag [\`sorbet\`](https://stackoverflow.com/questions/tagged/sorbet). We direct most questions here, to make the answers easier for others to discover.`,
                },
                {
                  title: `Chat with us`,
                  content: `We have a [Slack community](/slack) for general discussions. Topics include discussion of Sorbet internals, learning how others use Sorbet, and showing off projects built with Sorbet.`,
                },
                {
                  title: `Report issues`,
                  content: `Found a bug? We track issues [on GitHub](https://github.com/sorbet/sorbet/issues). Since we have limited time to dedicate to issue triage, you'll be asked to articulate a specific problem or explicit steps to reproduce the bug.`,
                },
              ]}
            />
          </PageSection>
        </div>
        <PageSection short>
          <div>
            <h1 id="talks">Talks</h1>
            <p>
              The Sorbet team has given a number of talks that are available
              online:
            </p>
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
                {
                  title: 'Gradual typing of Ruby at Scale',
                  link: '/docs/talks/curry-on-2019',
                  venue: 'Curry On! 2019',
                },
                {
                  title: 'Fast typechecking for Ruby',
                  link: '/docs/talks/jvm-ls-2019',
                  venue: 'JVM Language Summit 2019',
                },
                {
                  title: 'Sorbet: A type checker for Ruby 3 you can use today!',
                  link: '/docs/talks/ruby-conf-2019',
                  venue: 'RubyConf 2019',
                },
              ]}
            />
          </div>
          <div>
            <h1 id="projects">Projects</h1>
            <p>
              These are some community-maintained projects built on top of or
              alongside the core Sorbet tooling:
            </p>
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
                    'A set of tools to make Sorbet work with Rails seamlessly',
                },
                {
                  title: 'Sord',
                  link: 'https://github.com/AaronC81/sord',
                  description:
                    'A tool to generate RBIs from YARD documentation',
                },
                {
                  title: 'yard-sorbet',
                  link: 'https://github.com/dduugg/yard-sorbet',
                  description:
                    'A YARD plugin to parse Sorbet signatures when generating docs',
                },
                {
                  title: 'sorbet-progress',
                  link: 'https://github.com/jaredbeck/sorbet-progress',
                  description:
                    'Measure your progress as you adopt Sorbet, stay motivated!',
                },
                {
                  title: 'activerecord-ejection_seat',
                  link: 'https://github.com/maxveldink/activerecord-ejection_seat',
                  description:
                    'Eject from an ActiveRecord model to a Sorbet T::Struct, or buckle back in',
                },
                {
                  title: 'sorbet-result',
                  link: 'https://github.com/maxveldink/sorbet-result',
                  description:
                    'Introduces T::Result, T::Success and T::Failure types to facilitate Railway Oriented Programming',
                },
                {
                  title: 'sorbet-struct-comparable',
                  link: 'https://github.com/samuelgiles/sorbet-struct-comparable',
                  description:
                    "Comparable T::Struct's for the equality focused typed Ruby developer",
                },
                {
                  title: 'Parlour',
                  link: 'https://github.com/AaronC81/parlour',
                  description: 'A versatile RBI generator and plugin system',
                },
                {
                  title: 'rubocop-sorbet',
                  link: 'https://github.com/Shopify/rubocop-sorbet',
                  description: 'A collection of Rubocop rules for Sorbet',
                },
                {
                  title: 'pronto-sorbet',
                  link: 'https://github.com/teamsimplepay/pronto-sorbet',
                  description: 'Run Sorbet as a Pronto runner',
                },
                {
                  title: 'spoom',
                  link: 'https://github.com/Shopify/spoom',
                  description: 'Useful tools for Sorbet enthusiasts',
                },
              ]}
            />
            <p>
              If you've built a tool you think benefits the Sorbet community,
              feel free to open a PR to add it to this list!
            </p>
          </div>
          <div>
            <h1 id="legal">Legal</h1>
            <ul>
              <li>
                <a href="/docs/legal/trademark-policy">Trademark Policy</a>
              </li>
            </ul>
            <p></p>
          </div>
        </PageSection>
      </div>
    );
  }
}

Index.title = 'Community';

module.exports = Index;
