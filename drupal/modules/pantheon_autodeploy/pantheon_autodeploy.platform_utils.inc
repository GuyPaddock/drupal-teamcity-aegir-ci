<?php
/**
 * @file
 *   Drupal platform utility methods for the Drush Auto-deploy extension for the
 *   Drupal Shell (Drush).
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
 */

/**
 * Gets the local filesystem path to the Drupal platform.
 *
 * @return string
 *   The local path to the platform.
 *
 * @throws AutodeployException
 *   If the platform path cannot be identified, for any reason.
 */
function pantheon_autodeploy_get_platform_root() {
  static $drupal_root = NULL;

  if (empty($drupal_root)) {
    $drupal_root = drush_get_context('DRUSH_SELECTED_DRUPAL_ROOT');
  }

  if (empty($drupal_root)) {
    throw new AutodeployException(
      'DRUSH_SOURCE_NOT_EXISTS', dt('Cannot locate the source platform.'));
  }

  return $drupal_root;
}

/**
 * Gets the declared version of the specified installation profile.
 *
 * <p>For performance reasons, the version of each profile is only fetched once
 * and then cached for the duration of the command execution.</p>
 *
 * @param string $profile_name
 *   The machine name of the installation profile.
 *
 * @return string
 *   The version string for the installation profile.
 *
 * @throws AutodeployException
 *   If the version cannot be determined, for any reason.
 */
function pantheon_autodeploy_get_profile_version($profile_name) {
  static $profile_versions = array();

  if (!isset($profile_versions[$profile_name])) {
    $platform_root = pantheon_autodeploy_get_platform_root();

    $profile_info_path =
      sprintf('%s/profiles/%2$s/%2$s.info', $platform_root, $profile_name);

    if (!file_exists($profile_info_path) || !is_readable($profile_info_path)) {
      throw new AutodeployException(
        'AUTODEPLOY_REQUIRED_FILES_MISSING',
        dt('The specified `.info` file could not be read: @path',
          array('@path' => $profile_info_path)));
    }

    $profile_info = drush_drupal_parse_info_file($profile_info_path);

    if (empty($profile_info['version'])) {
      throw new AutodeployException(
        'AUTODEPLOY_VERSION_MISSING',
        dt('No version has been declared in the profile `.info` file: @path',
          array('@path' => $profile_info_path)));
    }

    $profile_versions[$profile_name] = $profile_info['version'];
  }

  return $profile_versions[$profile_name];
}

/**
 * Identifies which installation profile in the current platform is marked
 * `exclusive`, if any.
 *
 * If none of the installation profiles are exclusive, <code>NULL</code> is
 * returned. If multiple installation profiles are exclusive, only the first
 * one encountered is returned.
 *
 * @return NULL|stdClass
 *   Either the first exclusive profile encountered in the current
 *   platform; or, <code>NULL</code> if no profile is exclusive.
 */
function _pantheon_autodeploy_locate_exclusive_install_profile() {
  $result   = NULL;
  $profiles = _pantheon_autodeploy_get_install_profiles();

  foreach ($profiles as $profile) {
    $info_file_data = pantheon_autodeploy_get_install_profile_info($profile);

    if (!empty($info_file_data['exclusive'])) {
      $result = $profile;
      break;
    }
  }

  return $result;
}

/**
 * Gets all of the installation profiles in the current platform.
 *
 * This function caches the results for performance, so that multiple calls are
 * not expensive.
 *
 * @return stdClass[]
 *   An associative array of objects, each representing a single installation
 *   profile in the current Drupal platform. Each key is the machine name of the
 *   installation profile.
 */
function _pantheon_autodeploy_get_install_profiles() {
  static $profile_list = NULL;

  if (empty($profile_list)) {
    $drupal_root  = pantheon_autodeploy_get_platform_root();
    $profile_list = drush_find_profiles($drupal_root);
  }

  return $profile_list;
}
