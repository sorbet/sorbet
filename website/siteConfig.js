//
// For all the possible configuration options:
// https://docusaurus.io/docs/site-config
//

// List of projects/orgs using your project for the users page.
const users = [
  {
    caption: 'Shopify',
    image: 'img/shopify-logo.svg',
    infoLink: 'https://www.shopify.com/',
  },
  {
    caption: 'Coinbase',
    image: 'img/coinbase-logo.png',
    infoLink: 'https://www.coinbase.com/',
  },
  {
    caption: 'Atrium',
    image: 'img/atrium-logo.jpg',
    infoLink: 'https://www.atrium.co/',
  },
  {
    caption: 'Chan Zuckerberg Initiative',
    image: 'img/czi-logo.svg',
    infoLink: 'https://chanzuckerberg.com/',
  },
  {
    caption: 'Gusto',
    image: 'img/gusto-logo.jpg',
    infoLink: 'https://gusto.com/',
  },
  {
    caption: 'Kickstarter',
    image: 'img/kickstarter-logo.png',
    infoLink: 'https://www.kickstarter.com/',
  },
  {
    caption: 'Vonage',
    image: 'img/vonage-logo.png',
    infoLink: 'https://www.vonage.com/',
  }
];

const siteConfig = {
  title: 'Sorbet',
  tagline: 'A static type checker for Ruby',
  url: 'https://sorbet.org',
  cname: 'sorbet.org',
  baseUrl: '/',
  editUrl: 'https://github.com/sorbet/sorbet/edit/master/website/docs/',

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
    {label: 'Try', href: 'https://sorbet.run'},
    {label: 'Community', href: '/en/community'},
    {label: 'GitHub', href: 'https://github.com/sorbet/sorbet'},
    {blog: true, label: 'Blog'},
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
    primaryColor: '#4f4397', // --sorbet-purple-2
    secondaryColor: '#4f4397', // --sorbet-purple-2
  },

  markdownOptions: {
    typographer: true,
    quotes: '“”‘’',
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
  // (these images can't be SVGs)
  ogImage: 'img/sorbet-logo-card@2x.png',
  twitterImage: 'img/sorbet-logo-card@2x.png',

  // Show documentation's last contributor's name.
  // enableUpdateBy: true,

  // Show documentation's last update time.
  // enableUpdateTime: true,

  algolia: {
    apiKey: 'fa1ec885ab70787d636759b88e509b92',
    // This name must match the index name here:
    // https://github.com/algolia/docsearch-configs/blob/master/configs/stripe_sorbet.json
    indexName: 'stripe_sorbet',
    algoliaOptions: {}, // Optional, if provided by Algolia
  },

  gaTrackingId: 'UA-119877071-2',
};

module.exports = siteConfig;
