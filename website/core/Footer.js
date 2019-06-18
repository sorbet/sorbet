const React = require('react');

class Footer extends React.Component {
  render() {
    // TODO(jez) Consider putting the sitemap / GitHub buttons back.
    return (
      <footer className="nav-footer" id="footer">
        <div className="wrapper">
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
            {' · '}
            <a href="https://twitter.com/sorbet_ruby">Twitter</a>
          </p>
        </div>
      </footer>
    );
  }
}

module.exports = Footer;
