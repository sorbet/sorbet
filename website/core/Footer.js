const React = require('react');

class Footer extends React.Component {
  render() {
    const {config: siteConfig} = this.props;
    const {baseUrl, docsUrl} = siteConfig;
    const docsPart = `${docsUrl ? `${docsUrl}/` : ''}`;
    const docUrl = (doc) => `${baseUrl}${docsPart}${doc}`;
    const currentYear = new Date().getFullYear();
    return (
      <footer className="nav-footer" id="footer">
        <div className="wrapper">
          <p className="footer">
            © {currentYear} Stripe{' · '}
            <a href={docUrl('adopting')}>Get started</a>
            {' · '}
            <a href={docUrl('overview')}>Docs</a>
            {' · '}
            <a href="https://sorbet.run">Try</a>
            {' · '}
            <a href={`${baseUrl}en/community`}>Community</a>
            {' · '}
            <a href={`${baseUrl}blog`}>Blog</a>
            {' · '}
            <a href="https://twitter.com/sorbet_ruby">Twitter</a>
          </p>
        </div>
      </footer>
    );
  }
}

module.exports = Footer;
