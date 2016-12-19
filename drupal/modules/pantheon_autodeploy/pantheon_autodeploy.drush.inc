<?php
/**
 * @file
 *   Code for the Drush Auto-deploy extension for the Drupal Shell (Drush).
 *
 *   Copyright (C) 2015-2017  Red Bottle Design, LLC
 *
 *   This program is free software: you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation, either version 3 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Guy Paddock (guy@redbottledesign.com)
 */
require_once('pantheon_autodeploy.constants.inc');

require_once('includes/DrushException.php');

require_once('includes/pantheon_autodeploy.file_utils.inc');
require_once('includes/pantheon_autodeploy.git_utils.inc');
require_once('includes/pantheon_autodeploy.info_utils.inc');
require_once('includes/pantheon_autodeploy.platform_utils.inc');
require_once('includes/pantheon_autodeploy.shell_utils.inc');
require_once('includes/pantheon_autodeploy.terminus_utils.inc');

//==============================================================================
// Drush Interface Methods
//==============================================================================

/**
 * Implements hook_drush_command().
 */
function pantheon_autodeploy_drush_command() {
  $items['pantheon-autodeploy'] = array(
    'bootstrap'   => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    'description' =>
      dt('Deploys an installation profile as a distribution to a Pantheon instance via GIT.'),
    'aliases' => array('pad'),
    'arguments' => array(
      'site-id' =>
        dt('The UUID for the Pantheon site.'),
    ),
    'options' => array(
      PAAU_OPTION_USE_INFO =>
        dt('Triggers the auto-deploy to use settings for the specified branch in the `.info` file of the exclusive profile in the distribution, instead of taking settings from the command-line. If there are no auto-deploy settings for the branch in the .info file, this command exits gracefully. This is typically useful for CI environments.'),

      PAAU_OPTION_TARGET_ENV =>
        dt('The environment on Pantheon (`dev`, `test`, or `live`) to which the site should be deployed. In the Pantheon workflow, `test` assumes `dev` and `test`; and `live` assumes `dev`, `test`, and `live`. (The default is `dev`).'),

      PAAU_OPTION_PROFILE =>
        dt('The installation profile to deploy. (The default is the `exclusive` profile in the current platform.)'),

      PAAU_OPTION_TAG_AFTER_COMMIT =>
        dt('Whether or not the site should be tagged with the installation profile version after being committed. (The default is `false`).'),

      PAAU_OPTION_GIT_AUTHOR =>
        dt('Overrides the GIT author used for auto-deploy commits to the Pantheon site repository. Must be in standard GIT "A U Thor <author@example.com>" format.'),

      PAAU_OPTION_SOURCE_BUILD =>
        dt('Provides the build # from a continuous integration build # for the code that is being deployed, for cross-referencing in commit logs. (The default is not to include one).'),

      PAAU_OPTION_SOURCE_COMMIT =>
        dt('Provides the SHA hash of the source commit from which code is being deployed, for cross-referencing in commit logs. (The default is not to include one).'),
    ),
    'examples' => array(
      'drush pad 7ea62566-14c1-47e1-93f1-223759193e9e' =>
        dt('Deploys the current Drupal platform with its `exclusive` installation profile to the specified Pantheon instance using key-based authentication.'),

      sprintf('drush pad 7ea62566-14c1-47e1-93f1-223759193e9e --%s="my_profile"', PAAU_OPTION_PROFILE) =>
        dt('Deploys the current Drupal platform with its "my_profile" installation profile to the specified Pantheon instance using key-based authentication.'),

      sprintf('drush pad 7ea62566-14c1-47e1-93f1-223759193e9e --%s="test" --%s=true', PAAU_OPTION_TARGET_ENV, PAAU_OPTION_TAG_AFTER_COMMIT) =>
        dt('Deploys the current Drupal platform with its `exclusive` installation profile to the dev and test environments of the specified Pantheon instance, tagging the revision after commit.'),

      sprintf('drush pad 7ea62566-14c1-47e1-93f1-223759193e9e --%s="live" --%s="611" --%s="0788603f1c948671fbc56a18afa5e71f6b247520"', PAAU_OPTION_TARGET_ENV, PAAU_OPTION_SOURCE_BUILD, PAAU_OPTION_SOURCE_COMMIT) =>
        dt('Deploys the current Drupal platform with its `exclusive` installation profile to the live environment of the specified Pantheon instance, including build #611 and source commit hash in deployment logs.'),

      sprintf('drush pad --%s="develop"', PAAU_OPTION_USE_INFO) =>
        dt('Deploys the current Drupal platform with its `exclusive` installation profile using that profile\'s settings for the `develop` branch. If there are no settings for `develop`, do nothing.'),

      sprintf('drush pad --%s="my_profile" --%s="develop"', PAAU_OPTION_PROFILE, PAAU_OPTION_USE_INFO) =>
        dt('Deploys the current Drupal platform with its "my_profile" installation profile using that profile\'s settings for the `develop` branch. If there are no settings for `develop`, do nothing.'),
    ),
  );

  return $items;
}

/**
 * The <code>pantheon-autodeploy</code> (<code>pad</code>) command.
 *
 * Deploys an installation profile as a distribution to a Pantheon instance via
 * GIT.
 *
 * This task performs the following steps:
 *   1. The site repository is cloned bare (i.e. without any files).
 *   2. The installation profile is unpacked into the working copy of the site
 *      repository.
 *   3. Any changes to <code>*.info</code> files that are purely the result of
 *      the Drush make / Drush checkout process for modules are automatically
 *      reverted.
 *   4. The updated site is committed to the site repository and pushed.
 *   5. The site is automatically pushed to the target Pantheon environment
 *      (dev, test, or live).
 *
 * @param string $site_id
 *   Pantheon's UUID for the site.
 *
 * @throws DrushException
 *   If the deployment fails.
 */
function drush_pantheon_autodeploy($site_id = NULL) {
  $use_settings_for_branch = drush_get_option(PAAU_OPTION_USE_INFO, FALSE);
  $target_environment      = drush_get_option(PAAU_OPTION_TARGET_ENV, NULL);
  $profile_name            = drush_get_option(PAAU_OPTION_PROFILE, NULL);

  $tag_after_commit =
    _pantheon_autodeploy_convert_value_to_boolean(
      drush_get_option(PAAU_OPTION_TAG_AFTER_COMMIT, FALSE));

  try {
    if (empty($profile_name)) {
      $exclusive_profile =
        _pantheon_autodeploy_locate_exclusive_install_profile();

      $profile_name = $exclusive_profile->name;
    }

    pantheon_autodeploy_validate_options_pre_profile_load(
      $site_id, $use_settings_for_branch, $tag_after_commit, $target_environment, $profile_name);

    if ($use_settings_for_branch) {
      pantheon_autodeploy_run_from_info_file($profile_name, $use_settings_for_branch);
    }
    else {
      pantheon_autodeploy_run_from_cli(
        $site_id, $profile_name, $tag_after_commit, $target_environment);
    }
  }
  catch (DrushException $ex) {
    drush_set_error($ex->getDrushErrorCode(), $ex->getMessage());
  }
  catch (InvalidArgumentException $ex) {
    drush_set_error('AUTODEPLOY_VALID_OPTION_REQUIRED', $ex->getMessage());
  }
}

/**
 * Runs deployment using the settings for the specified GIT branch in the
 * specified installation profile.
 *
 * <p>If there are no deployment settings for the branch in the profile,
 * no deployment occurs.</p>
 *
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 * @param string $branch_name
 *   The GIT branch name for which settings are desired.
 *   NOTE: This is the name of the branch in the installation profile source
 *   repo, not the Pantheon repo.
 *
 * @throws DrushException
 *   If the deployment operation fails.
 */
function pantheon_autodeploy_run_from_info_file($profile_name, $branch_name) {
  $branch_settings =
    _pantheon_autodeploy_get_settings_for($profile_name, $branch_name);

  if ($branch_settings === NULL) {
    drush_log(
      dt('No auto-deploy settings specified in @profile, so not deploying.'),
      'warning');
  }
  else {
    $site_id            = $branch_settings['target']['site-id'];
    $target_environment = $branch_settings['target']['environment'];
    $tag_after_commit   = $branch_settings['tag-after-commit'];

    pantheon_autodeploy_validate_options_post_profile_load(
      $site_id, $target_environment);

    pantheon_autodeploy_run_deployment(
      $site_id, $profile_name, $tag_after_commit, $target_environment);
  }
}

/**
 * Runs deployment using the provided settings from CLI.
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 *   after being committed.
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 * @param string $target_environment
 *   The name of the Pantheon environment to which the site should be deployed.
 * @param boolean $tag_after_commit
 *   Whether or not to tag commits with the installation profile version.
 *
 * @throws DrushException
 *   If the deployment operation fails.
 */
function pantheon_autodeploy_run_from_cli($site_id, $profile_name,
                                          $tag_after_commit,
                                          $target_environment) {
  // Default this here so we can differentiate between the default and the
  // user having set it from the CLI when we're reading options from `.info`.
  if (empty($target_environment)) {
    $target_environment = PAAU_PANTHEON_ENV_DEV;
  }

  pantheon_autodeploy_validate_options_post_profile_load(
    $site_id, $target_environment);

  pantheon_autodeploy_run_deployment(
    $site_id, $profile_name, $tag_after_commit, $target_environment);
}

/**
 * Runs the deployment operation.
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 *   after being committed.
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 * @param boolean $tag_after_commit
 *   Whether or not to tag commits with the installation profile version.
 * @param string $target_environment
 *   The name of the Pantheon environment to which the site should be deployed.
 *
 * @throws DrushException
 *   If the deployment fails.
 */
function pantheon_autodeploy_run_deployment($site_id, $profile_name, $tag_after_commit,
                                            $target_environment) {
  $site_repo_path  = pantheon_autodeploy_checkout_site($site_id);

  pantheon_autodeploy_prune_target($site_repo_path, $profile_name);
  pantheon_autodeploy_copy_profile($profile_name, $site_repo_path);
  pantheon_autodeploy_strip_development_modules($site_repo_path, $profile_name);

  $profile_version =
    pantheon_autodeploy_get_profile_version($profile_name);

  $commit_message =
    pantheon_autodeploy_generate_git_commit_message($profile_version);

  pantheon_autodeploy_commit_and_push_site(
    $site_repo_path, $commit_message, $profile_version, $tag_after_commit);

  pantheon_autodeploy_deploy_site($site_id, $target_environment, $commit_message);
}

//==============================================================================
// Pantheon Auto-deploy Argument Validation
//==============================================================================
/**
 * Validates that required arguments and options are being passed to the
 * Pantheon Auto-deploy command on the command-line.
 *
 * <p>This only verifies argument values that can be examined prior to
 * obtaining values from the installation profile itself, in case the user
 * is electing to use an `.info`-based approach.</p>
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 * @param boolean $use_settings_for_branch
 *   The name of the branch for which settings should come from the source
 *   profile; or, <code>NULL</code>, if settings should come from CLI.
 * @param boolean $tag_after_commit
 *   Whether or not to tag commits with the installation profile version.
 * @param string $target_environment
 *   The name of the Pantheon environment to which the site should be deployed
 *   after being committed.
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 *
 * @throws DrushException
 *   If any arguments are invalid.
 */
function pantheon_autodeploy_validate_options_pre_profile_load(
                                          $site_id,
                                          $use_settings_for_branch,
                                          $tag_after_commit,
                                          $target_environment,
                                          $profile_name) {
  if ($use_settings_for_branch) {
    if (!empty($site_id)) {
      throw new InvalidArgumentException(
        dt('When using settings from the profile `.info` file, you cannot specify the Pantheon site UUID.'));
    }

    if (!empty($target_environment)) {
      throw new InvalidArgumentException(
        dt('When using settings from the profile `.info` file, you cannot specify the target environment.'));
    }

    if (!empty($tag_after_commit)) {
      throw new InvalidArgumentException(
        dt('When using settings from the profile `.info` file, you cannot pass the tag-after-commit option.'));
    }
  }
  else {
    if (empty($site_id)) {
      throw new InvalidArgumentException(
        dt('When not using settings from the profile `.info` file, you need to specify a valid Pantheon site UUID (for example, `7ea62566-14c1-47e1-93f1-223759193e9e`).'));
    }
  }

  $profiles = _pantheon_autodeploy_get_install_profiles();

  if (empty($profile_name)) {
    throw new InvalidArgumentException(
      dt('No installation profile was specified, and none of the installation profiles in the current platform are `exclusive`. Please specify the target profile. Valid profiles are: [@types]',
        array(
          '@types' => implode(', ', array_keys($profiles)),
        )));
  }

  if (!isset($profiles[$profile_name])) {
    throw new InvalidArgumentException(
      dt('`@name` is not a valid installation profile in the current platform. Valid profiles are: [@types]',
       array(
         '@name'  => $profile_name,
         '@types' => implode(', ', array_keys($profiles)))));
  }
}

/**
 * Validates remaining options before executing deployment.
 *
 * <p>This complements
 * <code>pantheon_autodeploy_validate_pre_profile_load()</code> by handling any
 * validations that have to happen after settings are obtained from the
 * installation profile, in case the user is electing to use an `.info`-based
 * approach.</p>
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 * @param string $target_environment
 *   The name of the Pantheon environment to which the site should be deployed
 *   after being committed.
 *
 * @throws DrushException
 *   If any arguments are invalid.
 */
function pantheon_autodeploy_validate_options_post_profile_load($site_id, $target_environment) {
  if (empty($site_id) || !preg_match(PAAU_UUID_PATTERN, $site_id)) {
    throw new InvalidArgumentException(
      dt('You need to specify a valid Pantheon site UUID (for example, `7ea62566-14c1-47e1-93f1-223759193e9e`).'));
  }

  if (!empty($target_environment)) {
    _pantheon_autodeploy_validate_pantheon_environment($target_environment);
  }
}

//==============================================================================
// Public API Methods
//==============================================================================
/**
 * Checks-out the GIT repository for the Pantheon site.
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 * @return string
 *   The local path to the temporary folder where the site was checked out.
 *
 * @throws DrushException
 *   If the checkout operation fails.
 */
function pantheon_autodeploy_checkout_site($site_id) {
  $repo_url = pantheon_autodeploy_get_git_repo_url($site_id);

  drush_log(
    dt('Checking out site `@site_id` from Pantheon.',
      array('@site_id' => $site_id)));

  $local_checkout_path =
    _pantheon_autodeploy_git_clone_to_temporary_directory($repo_url);

  drush_log(
    dt('Site `@site_id` checked-out from GIT.',
      array('@site_id' => $site_id)),
    'success');

  return $local_checkout_path;
}

/**
 * Removes unwanted files from a checked-out copy of a Pantheon site.
 *
 * <p>This is typically files and/or folders under "sites/" and "profiles/" that
 * will be replaced by corresponding files from the platform being copied.</p>
 *
 * @param string $site_repo_path
 *   The absolute path to the location where the Pantheon site has been
 *   checked-out.
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 */
function pantheon_autodeploy_prune_target($site_repo_path, $profile_name) {
  $profile_path = sprintf('%s/profiles/%s', $site_repo_path, $profile_name);

  drush_log(dt('Pruning unwanted files from the site work tree path.'));
  _pantheon_autodeploy_force_delete_dir($profile_path);

  drush_log(
    dt('Unwanted files from site work tree have been pruned.'), 'success');
}

/**
 * Copies the current platform and the specified installation profile to
 * the specified Pantheon site checkout path.
 *
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 * @param string $site_repo_path
 *   The absolute path to the location where the Pantheon site has been
 *   checked-out.
 *
 * @throws DrushException
 *   If copying the platform and profile fails.
 */
function pantheon_autodeploy_copy_profile($profile_name, $site_repo_path) {
  $source_files = _pantheon_autodeploy_get_source_files($profile_name);

  drush_log(
    dt('Copying platform and installation profile `@profile_name` to checkout path.',
      array('@profile_name' => $profile_name)));

  if (!empty($source_files)) {
    $platform_root = pantheon_autodeploy_get_platform_root();

    foreach ($source_files as $source_file) {
      $relative_path =
        _pantheon_autodeploy_absolute_to_relative_path(
          $source_file->filename, $platform_root);

      $source_path      = $source_file->filename;
      $destination_path = $site_repo_path . $relative_path;

      _pantheon_autodeploy_mk_parent_paths($destination_path);

      drush_copy_dir(
        $source_path,
        $destination_path,
        FILE_EXISTS_MERGE);
    }
  }

  drush_log(
    dt('Platform, with `@profile_name` profile, copied to site work tree.',
      array('@profile_name' => $profile_name)),
    'success');
}

/**
 * Strips development-only features and modules out of the specified
 * installation profile in the specified checked-out copy of a Pantheon site.
 *
 * <p>This allows any tools that should only be used in development to be
 * excluded from production-ready releases to reduce attack vectors and
 * make the distribution smaller.</p>
 *
 * @param string $site_repo_path
 *   The absolute path to the location where the Pantheon site has been
 *   checked-out.
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 *
 * @throws DrushException
 *   If the operation fails.
 */
function pantheon_autodeploy_strip_development_modules($site_repo_path, $profile_name) {
  $drush_make_path =
    sprintf('%s/profiles/%s/drupal-org.make', $site_repo_path, $profile_name);

  $profile_info_path =
    sprintf('%1$s/profiles/%2$s/%2$s.info', $site_repo_path, $profile_name);

  drush_log(
    dt('Stripping development-only modules and features out of `@profile_name` in the platform.',
      array('@profile_name' => $profile_name)));

  if (!is_file($drush_make_path) && !is_readable($drush_make_path)) {
    // It's unusual, but not fatal, to be missing a `drupal-org.make` file.
    drush_log(
      dt('The Drush make file could not be located at: @path',
        array('@path' => $drush_make_path)),
      'warning');

    $dev_drush_make_info = array();
  }
  else {
    $dev_drush_make_info =
      _pantheon_autodeploy_parse_dev_only_info($drush_make_path);
  }

  if (!is_file($profile_info_path) && !is_readable($profile_info_path)) {
    // It IS fatal to be missing the profile `.info` file, though.
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The install profile `.info` file could not be located at: @path',
        array('@path' => $profile_info_path)));
  }

  $dev_profile_info =
    _pantheon_autodeploy_parse_dev_only_info($profile_info_path);

  $modules_to_strip = array();

  if (isset($dev_drush_make_info['projects'])) {
    $modules_to_strip = array_keys($dev_drush_make_info['projects']);
  }

  if (isset($dev_profile_info['dependencies'])) {
    $modules_to_strip =
      array_merge($modules_to_strip, $dev_profile_info['dependencies']);
  }

  pantheon_autodeploy_remove_modules(
    $modules_to_strip, $site_repo_path, $profile_name);

  drush_log(
    dt('Stripping development-only information out of profile info files.'));
  _pantheon_autodeploy_remove_dev_only_info($profile_info_path);

  drush_log(
    dt('Development-only modules and dependencies stripped from platform.'),
    'success');
}

/**
 * Commits the modified Pantheon site to the local repository and then
 * pushes it to Pantheon.
 *
 * <p>If desired, the site is also tagged with the profile version.</p>
 *
 * @param string $site_repo_path
 *   The absolute path to the location where the Pantheon site has been
 *   checked-out.
 * @param string $commit_message
 *   The message to use for commits and tags.
 * @param string $profile_version
 *   The version number of the profile, in Drupal's standard format of
 *   "7.x-1.0", for use by tags. Can be <code>NULL</code> if
 *   <code>$tag_after_commit</code> is <code>FALSE</code>.
 * @param boolean $tag_after_commit
 *   Whether or not to tag commits with the installation profile version.
 */
function pantheon_autodeploy_commit_and_push_site($site_repo_path,
                                                  $commit_message,
                                                  $profile_version,
                                                  $tag_after_commit) {
  if ($tag_after_commit && empty($profile_version)) {
    throw new InvalidArgumentException(
      dt('Profile version must not be NULL or empty when tagging after-commit is enabled.'));
  }

  $author_name =
    drush_get_option(
      PAAU_OPTION_GIT_AUTHOR,
      'Pantheon Auto-deploy <support@redbottledesign.com>');

  if (!$tag_after_commit) {
    drush_log(dt('Staging, committing, and pushing changed files to Pantheon site.'));
  }
  else {
    drush_log(dt('Staging, committing, tagging, and pushing changed files to Pantheon site.'));
  }

  _pantheon_autodeploy_git_add_all($site_repo_path);

  $changes = _pantheon_autodeploy_git_status($site_repo_path);
  drush_log($changes, 'success');

  $commit_hash =
    _pantheon_autodeploy_git_commit(
      $site_repo_path,
      $commit_message,
      $author_name);

  if ($tag_after_commit) {
    _pantheon_autodeploy_git_tag_commit(
      $site_repo_path, 
      $commit_hash, 
      $profile_version,
      $commit_message);
  }
  
  _pantheon_autodeploy_git_push($site_repo_path);

  drush_log(dt('Site checked-in and pushed to Pantheon.'), 'success');
}

/**
 * Deploys the specified Pantheon environment to the specified environment.
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 *   after being committed.
 * @param string $target_environment
 *   The name of the Pantheon environment to which the site should be deployed.
 * @param string $message
 *   The message to use in deployment logs.
 *
 * @throws DrushException
 *   If the deployment fails.
 */
function pantheon_autodeploy_deploy_site($site_id, $target_environment, $message) {
  $deployments_needed = array();

  $requires_test_deployment = array(PAAU_PANTHEON_ENV_TEST, PAAU_PANTHEON_ENV_LIVE);
  $requires_live_deployment = array(PAAU_PANTHEON_ENV_LIVE);

  if (in_array($target_environment, $requires_test_deployment)) {
    $deployments_needed[] = PAAU_PANTHEON_ENV_TEST;
  }

  if (in_array($target_environment, $requires_live_deployment)) {
    $deployments_needed[] = PAAU_PANTHEON_ENV_LIVE;
  }

  if (empty($deployments_needed)) {
    drush_log(
      dt('No deployment necessary -- GIT already pushed `@site_id` to the dev environment.',
        array('@site_id' => $site_id)),
      'success');
  }
  else {
    foreach ($deployments_needed as $deployment) {
      drush_log(
        dt('Deploying `@site_id` to `@environment`.',
          array(
            '@site_id'     => $site_id,
            '@environment' => $deployment)));

      _pantheon_autodeploy_pantheon_deploy_site($site_id, $deployment, $message);

      drush_log(
        dt('Site deployed to the `@environment` on Pantheon.',
          array('@environment' => $deployment)),
        'success');
    }
  }
}

//==============================================================================
// Utility Methods
//==============================================================================
/**
 * Removes modules with the specified names from the specified profile in the
 * specified checked-out copy of a Pantheon site.
 *
 * @param string[] $modules
 *   An array of machine names for modules to remove from the platform.
 * @param string $site_repo_path
 *   The absolute path to the location where the Pantheon site has been
 *   checked-out.
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 */
function pantheon_autodeploy_remove_modules(array $modules, $site_repo_path, $profile_name) {
  $profile_modules_path =
    sprintf("%s/profiles/%s/modules", $site_repo_path, $profile_name);

  $escaped_modules = array_map("_pantheon_autodeploy_regex_escape", $modules);
  $module_regex    = sprintf('/^(%s)\.info$/', implode("|", $escaped_modules));

  $found_files = drush_scan_directory($profile_modules_path, $module_regex);

  foreach ($found_files as $file) {
    $parent_dir = dirname($file->filename);
    _pantheon_autodeploy_force_delete_dir($parent_dir);
  }
}

/**
 * Gets the GIT clone URL for the specified site ID.
 *
 * <p>For example, this converts a site ID like:</p>
 * <pre>
 *   7ea62566-14c1-47e1-93f1-223759193e9e
 * </pre>
 *
 * <p>Into:</p>
 * <pre>
 *   ssh://codeserver.dev.7ea62566-14c1-47e1-93f1-223759193e9e@codeserver.dev.7ea62566-14c1-47e1-93f1-223759193e9e.drush.in:2222/~/repository.git
 * </pre>
 *
 * @param string $site_id
 *   The UUID of the target Pantheon site.
 *
 * @return string
 *   The GIT repo URL.
 */
function pantheon_autodeploy_get_git_repo_url($site_id) {
  return sprintf(PAAU_PANTHEON_CLONE_PATTERN, $site_id);
}

/**
 * Generates an appropriate message to describe a GIT commit, tag, or deployment
 * of a given version of the installation profile.
 *
 * @param string $profile_version
 *   The version number of the profile, in Drupal's standard format of
 *   "7.x-1.0".
 *
 * @return string
 *   The localized commit message.
 */
function pantheon_autodeploy_generate_git_commit_message($profile_version) {
  $source_build = drush_get_option(PAAU_OPTION_SOURCE_BUILD, NULL);
  $source_hash  = drush_get_option(PAAU_OPTION_SOURCE_COMMIT, NULL);

  if (empty($source_build) && empty($source_hash)) {
    $source_rev_id = '';
  }
  elseif (!empty($source_build) && empty($source_hash)) {
    $source_rev_id =
      dt(' [build @build_number]',
        array('@build_number' => $source_build));
  }
  elseif (empty($source_build) && !empty($source_hash)) {
    $source_rev_id =
      dt(' [source hash: @build_hash]',
        array('@build_hash' => $source_hash));
  }
  else {
    $source_rev_id =
      dt(' [build @build_number, source hash: @build_hash]',
        array(
          '@build_number' => $source_build,
          '@build_hash' => $source_hash,
        ));
  }

  $commit_message =
    dt('[@version] Automatic deployment to Pantheon@source_build_id.',
      array(
        '@version'          => $profile_version,
        '@source_build_id'  => $source_rev_id,
      ));

  return $commit_message;
}

/**
 * Assembles a list of all files from the platform that need to be copied to
 * the checked-out copy of the Pantheon site.
 *
 * @param string $profile_name
 *   The machine name of the installation profile being deployed.
 * @return stdClass[]
 *   An array of objects that represent the files to be copied. Each object
 *   is structured according to the conventions defined by
 *   <code>drush_scan_directory()</code>.
 *
 * @throws DrushException
 *   If required files are missing from the source platform.
 */
function _pantheon_autodeploy_get_source_files($profile_name) {
  $platform_root = pantheon_autodeploy_get_platform_root();

  $profile_path = sprintf('%s/profiles/%s', $platform_root, $profile_name);
  $sites_path   = sprintf('%s/sites', $platform_root);

  if (!is_dir($profile_path)) {
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('Could not find the installation profile in the expected location: @path',
        array('@path' => $profile_path)));
  }

  if (!is_dir($sites_path)) {
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('Could not find the sites folder in the expected location: @path',
        array('@path' => $sites_path)));
  }

  $exclude_paths =
    array(
      '.',
      '..',
      '.git',
      'sites',
      'profiles',
    );

  $files_to_copy =
    drush_scan_directory(
      $platform_root, // $dir
      "/.*/",         // $mask
      $exclude_paths, // $nomask
      0,              // $callback = 0
      FALSE,          // $recurse_max_depth
      'filename',     // $key
      0,              // $min_depth
      TRUE);          // $include_dot_files

  $profile_directory =
    _pantheon_autodeploy_convert_path_to_file_object($profile_path);

  $files_to_copy[$profile_directory->filename] = $profile_directory;

  return $files_to_copy;
}

/**
 * Escapes the given string for use in a Perl-compatible regular expression.
 *
 * @param string  $string
 *   The value that needs to be escaped.
 *
 * @return string
 *   The regex-safe version of the string.
 */
function _pantheon_autodeploy_regex_escape($string) {
  return preg_quote($string, '/');
}

/**
 * Converts the provided value to a boolean.
 *
 * <p>This treats the following values as equivalents for true values:</p>
 * - <code>1</code>
 * - <code>'1'</code>
 * - <code>'true'</code>
 * - <code>'yes'</code>
 * - <code>TRUE</code>
 *
 * @param mixed $value
 *   The value to convert to a boolean.
 *
 * @return bool
 *   The boolean value of the value.
 */
function _pantheon_autodeploy_convert_value_to_boolean($value) {
  $lowercase_value = strtolower($value);

  return (($value === 1) ||
          ($value === '1') ||
          ($lowercase_value === 'true') ||
          ($lowercase_value === 'yes') ||
          ($value === TRUE));
}
