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
  // End Sorbet early adopters. Please alphabetize below this comment.
  {
    caption: 'Chan Zuckerberg Initiative',
    image: 'img/czi-logo.svg',
    infoLink: 'https://chanzuckerberg.com/',
  },
  {
    caption: 'Chime',
    image: 'img/chime-logo.svg',
    infoLink: 'https://www.chime.com/',
  },
  {
    caption: 'Factorial',
    image: 'img/factorial-logo.png',
    infoLink: 'https://factorialhr.com/',
  },
  {
    caption: 'Flexport',
    image: 'img/flexport-logo.jpg',
    infoLink: 'https://flexport.com/',
  },
  {
    caption: 'Grailed',
    image: 'img/grailed-logo.png',
    infoLink: 'https://www.grailed.com/',
  },
  {
    caption: 'Gusto',
    image: 'img/gusto-logo.jpg',
    infoLink: 'https://gusto.com/',
  },
  {
    caption: 'HealthSherpa',
    image: 'img/healthsherpa-logo.png',
    infoLink: 'https://www.healthsherpa.com/',
  },
  {
    caption: 'Hummingbird',
    image: 'img/hummingbird-logo.png',
    infoLink: 'https://hummingbird.co/',
  },
  {
    caption: 'Instacart',
    image: 'img/instacart-logo.svg',
    infoLink: 'https://instacart.com/',
  },
  {
    caption: 'Kickstarter',
    image: 'img/kickstarter-logo.png',
    infoLink: 'https://www.kickstarter.com/',
  },
  {
    caption: 'Marketplacer',
    image: 'img/marketplacer-logo.png',
    infoLink: 'https://marketplacer.com/',
  },
  {
    caption: 'PhishSafety',
    image: 'img/phishsafety-logo.png',
    infoLink: 'https://phishsafety.com',
  },
  {
    caption: 'TaskRabbit',
    image: 'img/taskrabbit-logo.svg',
    infoLink: 'https://www.taskrabbit.com/',
  },
  {
    caption: 'TriumphPay',
    image: 'img/triumphpay-logo.svg',
    infoLink: 'https://triumphpay.com',
  },
  {
    caption: 'Vonage',
    image: 'img/vonage-logo.png',
    infoLink: 'https://www.vonage.com/',
  },
  {
    caption: 'Workforce.com',
    image: 'img/workforce-logo.png',
    infoLink: 'https://www.workforce.com/',
  },
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

  // This copyright info is used in blog RSS/Atom feeds.
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
  // ogImage: 'img/sorbet-logo-card@2x.png',
  // twitterImage: 'img/sorbet-logo-card@2x.png',

  // Show documentation's last contributor's name.
  // enableUpdateBy: true,

  // Show documentation's last update time.
  // enableUpdateTime: true,

  algolia: {
    appId: 'ZYWC6Z01G8',
    apiKey: 'b05a65e9f8d9f50dc4ec3241db8836c4',
    // This name must match the index name here:
    // https://github.com/algolia/docsearch-configs/blob/master/configs/stripe_sorbet.json
    indexName: 'stripe_sorbet',
    algoliaOptions: {}, // Optional, if provided by Algolia
  },

  gaTrackingId: 'G-5H7PQ9Z8KF',
  gaGtag: true,
};

module.exports = siteConfig;
