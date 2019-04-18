//
// For all the possible configuration options:
// https://docusaurus.io/docs/site-config
//

// List of projects/orgs using your project for the users page.
const users = [];

const siteConfig = {
  title: 'Sorbet',
  tagline: 'A static type checker for Ruby',
  url: 'https://sorbet.org',
  baseUrl: '/',
  editUrl: 'https://github.com/stripe/sorbet/edit/master/website/docs/',

  // Used for publishing and more
  projectName: 'sorbet',
  organizationName: 'stripe',

  // For no header links in the top nav bar -> headerLinks: [],
  headerLinks: [
    // 'adopting' and 'overview' are in the same top-level entry in sidebars.json,
    // so both tabs on mobile would get highlighted.  Using the 'href:' attribute
    // for "Get started" means that only the "Docs" tab will ever be active.
    {label: 'Get started', href: '/docs/adopting'},
    {label: 'Docs', doc: 'overview'},
    {label: 'Try it online', href: 'https://sorbet.run'},
    {label: 'Talks', doc: 'talks'},
  ],

  customDocsPath: 'website/docs',

  // If you have users set above, you add it here:
  users,

  // path to images for header/footer
  headerIcon: 'img/sorbet-logo-white-sparkles.svg',
  footerIcon: 'img/sorbet-logo-purple-sparkles.svg',
  favicon: 'img/favicon.ico',

  /* Colors for website */
  colors: {
    primaryColor:   '#4f4397', // --sorbet-purple-2
    secondaryColor: '#4f4397', // --sorbet-purple-2
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
  ogImage: 'img/sorbet-logo-purple-sparkles.svg',
  twitterImage: 'img/sorbet-logo-purple-sparkles.svg',

  // Show documentation's last contributor's name.
  // enableUpdateBy: true,

  // Show documentation's last update time.
  // enableUpdateTime: true,

  algolia: {
    apiKey: 'fa1ec885ab70787d636759b88e509b92',
    indexName: 'stripe_sorbet',
    algoliaOptions: {} // Optional, if provided by Algolia
  },
};

module.exports = siteConfig;
