<?php
/**
 * @file
 *   Constants for the Drush Auto-deploy extension for the Drupal Shell (Drush).
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
 */

/**
 * The name of the command-line option that signals that settings should come
 * from the source profile's `.info` files.
 *
 * @var string
 */
define('PAAU_OPTION_USE_INFO', 'use-info-for-branch');

/**
 * The name of the command-line option that indicates into which Pantheon
 * environment the site should be deployed.
 *
 * @var string
 */
define('PAAU_OPTION_TARGET_ENV', 'site-target-environment');

/**
 * The name of the command-line option that indicates which profile to use as
 * the source.
 *
 * @var string
 */
define('PAAU_OPTION_PROFILE', 'profile');

/**
 * The name of the command-line option that indicates the site should be tagged
 * with the profile version upon check-in.
 *
 * @var string
 */
define('PAAU_OPTION_TAG_AFTER_COMMIT', 'site-tag-after-commit');

/**
 * The sprintf pattern for a Pantheon clone URL.
 */
define('PAAU_PANTHEON_CLONE_PATTERN', 'ssh://codeserver.dev.%1$s@codeserver.dev.%1$s.drush.in:2222/~/repository.git');

/**
 * The sprintf pattern for a GIT command that includes the path to the repo.
 */
define('PAAU_GIT_WITH_REPO_PATTERN', 'git --work-tree=%1$s --git-dir=%1$s/.git');

/**
 * The name of Pantheon's development environment, when working with Terminus.
 */
define('PAAU_PANTHEON_ENV_DEV',  'dev');

/**
 * The name of Pantheon's test environment, when working with Terminus.
 */
define('PAAU_PANTHEON_ENV_TEST', 'test');

/**
 * The name of Pantheon's live environment, when working with Terminus.
 */
define('PAAU_PANTHEON_ENV_LIVE', 'live');

/**
 * The regular expression used to validate UUIDs, like Pantheon's site IDs.
 *
 * @var string
 */
define('PAAU_UUID_PATTERN', '/^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$/');

/**
 * The regular expression used to identify the section of an `.info` file that
 * applies only to development builds.
 */
define('PAAU_DEV_ONLY_PATTERN', '/<developmentOnly>(.*)<\/developmentOnly>/s');
