//
// For all the possible configuration options:
// https://docusaurus.io/docs/site-config
//

// List of projects/orgs using your project for the users page.
const users = [];

const siteConfig = {
  title: 'Sorbet',
  tagline: 'A static type checker for Ruby',
  url: 'https://jez.ngrok.io', // TODO(jez) Get a proper domain
  baseUrl: '/',

  // Used for publishing and more
  projectName: 'sorbet',
  organizationName: 'stripe',

  // For no header links in the top nav bar -> headerLinks: [],
  headerLinks: [
    {label: 'Docs', page: 'docs'},
    {label: 'Community', doc: 'community'},
    {label: 'Blog', blog: true},
    {label: 'GitHub', href: 'https://github.com/stripe/sorbet'},
  ],

  customDocsPath: 'website/docs',

  // If you have users set above, you add it here:
  users,

  // path to images for header/footer
  headerIcon: 'img/favicon-32x32.png',
  footerIcon: 'img/logo-192x192.png',
  favicon: 'img/favicon-16x16.png',

  /* Colors for website */
  colors: {
    primaryColor: '#6856f0',
    secondaryColor: '#9280f4',
  },

  // Custom fonts for website
  // fonts: {
  //   myFont: [
  //     "Times New Roman",
  //     "Serif"
  //   ],
  //   myOtherFont: [
  //     "-apple-system",
  //     "system-ui"
  //   ]
  // },

  // This copyright info is used in /core/Footer.js and blog RSS/Atom feeds.
  copyright: `© ${new Date().getFullYear()} Stripe`,

  highlight: {
    // Highlight.js theme to use for syntax highlighting in code blocks.
    theme: 'default',
  },

  // Add custom scripts here that would be placed in <script> tags.
  scripts: [],

  // Put Table of Contents on the right side of the page
  onPageNav: 'separate',

  // No .html extensions for paths.
  cleanUrl: true,

  // Open Graph and Twitter card images.
  ogImage: 'img/logo-512x512.png',
  twitterImage: 'img/logo-512x512.png',

  // Show documentation's last contributor's name.
  // enableUpdateBy: true,

  // Show documentation's last update time.
  // enableUpdateTime: true,
};

module.exports = siteConfig;
