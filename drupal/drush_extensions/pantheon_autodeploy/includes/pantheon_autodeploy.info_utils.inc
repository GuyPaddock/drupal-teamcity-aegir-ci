<?php
/**
 * @file
 *   Drupal `.info` file manipulation utility methods for the Drush Auto-deploy
 *   extension for the Drupal Shell (Drush).
 *
 *   Copyright (C) 2015-2017  Red Bottle Design, LLC
 *   Copyright (c) 2019 Inveniem.
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
 * @author Guy Elsmore-Paddock (guy@inveniem.com)
 */
require_once('pantheon_autodeploy.platform_utils.inc');

/**
 * Finds all modules, features, or themes having the specified machine name.
 *
 * @param string $name
 *   The machine name of the desired module(s), feature(s), or theme(s).
 *
 * @return array
 *   A nested array of info data for all matching modules, features, or themes;
 *   or an empty array if none could be find. Each entry contains the following
 *   keys:
 *     - file: A <code>stdClass</code> object structured according to the
 *       conventions defined by <code>drush_scan_directory()</code>.
 *     - data: The data parsed from the info file for the target, in the format
 *       returned by <code>drush_drupal_parse_info_file()</code>.
 *
 * @throws DrushException
 *   If the platform path cannot be identified, for any reason.
 */
function _pantheon_autodeploy_find_info_by_module_name($name) {
  $result = array();

  $platform_root = pantheon_autodeploy_get_platform_root();

  $exclude_paths  = array('.', '..', 'tests');
  $all_info_files =
    drush_scan_directory(
      $platform_root,         // $dir
      "/^{$name}\\.info*$/",  // $mask
      $exclude_paths);        // $nomask

  foreach ($all_info_files as $path => $file) {
    $result[$path] = array(
      'file' => $file,
      'data' => drush_drupal_parse_info_file($path),
    );
  }

  return $result;
}

/**
 * Finds all `.info` files under the specified root path.
 *
 * @param string $root_path
 *   The path under which `.info` files will be located.
 * @param array $exclude_paths
 *   An array of directory or file names to exclude from the search.
 *
 * @return stdClass[]
 *   An array of file objects structured according to the conventions defined by
 *   <code>drush_scan_directory()</code>.
 */
function _pantheon_autodeploy_find_all_info_under_root($root_path, $exclude_paths = array()) {
  $exclude_paths = array_merge(array('.', '..', 'tests'), $exclude_paths);

  $all_info_files =
    drush_scan_directory(
      $root_path,         // $dir
      "/^.*\\.info*$/",   // $mask
      $exclude_paths);    // $nomask

  return $all_info_files;
}

/**
 * Parses an info file from an install profile and returns the result.
 *
 * By default, the info file targeted is the main info file for the profile
 * (for example, the main info file for a profile named "abc" would be
 * "abc.info"), but this can be overridden using the <code>$info_file</code>
 * parameter.
 *
 * @param stdClass|string $profile
 *   Either the profile instance (as returned by
 *   <code>_pantheon_autodeploy_get_install_profiles()</code>), or the name
 *   of the installation profile.
 *
 * @param string $info_filename
 *   An optional parameter for specifying the filename of the info file within
 *   the profile to parse and return.
 *
 * @return array
 *   Either the result of parsing the specified info file in the specified
 *   installation profile; or, an empty array if the info file could not be
 *   found.
 *
 * @throws InvalidArgumentException
 *   If the specified profile does not exist.
 */
function _pantheon_autodeploy_get_install_profile_info($profile, $info_filename = NULL) {
  if (is_object($profile)) {
    $profile_object = $profile;
  }
  else {
    $profiles = _pantheon_autodeploy_get_install_profiles();

    if (isset($profiles[$profile])) {
      $profile_object = $profiles[$profile];
    }
    else {
      throw new InvalidArgumentException("Unknown profile: $profile");
    }
  }

  if (empty($info_filename)) {
    $info_filename = "{$profile_object->name}.info";
  }

  $profile_path   = dirname($profile_object->filename);
  $info_file_path = sprintf("%s/%s", $profile_path, $info_filename);

  return drush_drupal_parse_info_file($info_file_path);
}

/**
 * Gets the branch-specific deployment settings from the specified installation
 * profile.
 * 
 * <p>If the installation profile has no deployment settings for the specified
 * branch, or any deployment settings whatsoever, an empty array is
 * returned.</p>
 * 
 * @param string $profile_name
 *   The name of the installation profile for which settings are desired.
 * @param string $branch
 *   The name of the branch for which settings are desired.
 * 
 * @return array
 *   Either the array of deployment settings, or <code>NULL</code> if there are
 *   no settings for the branch. The settings can include the following keys:
 *   - target
 *     - site-id: The UUID of the target Pantheon site.
 *     - environment: The name of the Pantheon environment (dev, test, live).
 *   - tag-after-commit: Whether or not to tag commits with release numbers.
 */
function _pantheon_autodeploy_get_settings_for($profile_name, $branch) {
  $result = NULL;

  $system_defaults = array(
    'target' => array(
      'site-id'     => NULL,
      'environment' => NULL,
    ),
    'tag-after-commit' => FALSE,
  );

  $profile_info = _pantheon_autodeploy_get_install_profile_info($profile_name);

  if (!empty($profile_info['autodeploy']['branches'])) {
    $branches   = $profile_info['autodeploy']['branches'];
    $have_match = FALSE;

    foreach ($branches as $branch_pattern => $settings) {
      $branch_regex = "!^{$branch_pattern}$!";

      if (preg_match($branch_regex, $branch)) {
        $result     = $settings;
        $have_match = TRUE;
        break;
      }
    }

    if ($have_match) {
      if (!empty($profile_info['autodeploy']['defaults'])) {
        $profile_defaults = $profile_info['autodeploy']['defaults'];
      }
      else {
        $profile_defaults = array();
      }

      $result =
        array_replace_recursive($system_defaults, $profile_defaults, $result);

      drush_log(
        dt('Using branch settings: !settings',
          array('!settings' => var_export($result, TRUE))));
    }
  }

  return $result;
}

/**
 * Parses the portion of the specified `.info` file that has been demarcated
 * by "developmentOnly" tags.
 *
 * @param string $info_file_path
 *   The path to the info file to read.
 *
 * @return array
 *   The data in the development-only section, as a nested array structure in
 *   the same format as returned by <code>drush_drupal_parse_info_file()</code>
 *   or <code>drupal_parse_info_file()</code>.
 *
 * @throws DrushException
 *   If the file cannot be parsed.
 */
function _pantheon_autodeploy_parse_dev_only_info($info_file_path) {
  $result    = array();
  $info_data = file_get_contents($info_file_path);

  if ($info_data === FALSE) {
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The specified `.info` file could not be read: @path',
        array('@path' => $info_file_path)));
  }

  $matches = array();

  if (preg_match(PAAU_DEV_ONLY_PATTERN, $info_data, $matches)) {
    $info_extract = $matches[1];

    // Workaround dev-only passages/sections of the info file that contain ONLY
    // comment lines.
    //
    // See: https://github.com/drush-ops/drush/issues/4005
    $info_extract .= PHP_EOL . '_placeholder=1';

    $result = _drush_drupal_parse_info_file($info_extract);

    if ($result === FALSE) {
      throw new DrushException(
        'AUTODEPLOY_REQUIRED_FILES_MALFORMED',
        dt('The dev-only section of the specified `.info` file could not be parsed: @path',
          array('@path' => $info_file_path)));
    }
  }

  return $result;
}

/**
 * Removes the portion of the specified `.info` file that has been demarcated
 * by "developmentOnly" tags.
 *
 * @param string $info_file_path
 *   The path to the info file to modify.
 *
 * @throws DrushException
 *   If the file cannot be parsed or saved successfully.
 */
function _pantheon_autodeploy_remove_dev_only_info($info_file_path) {
  _pantheon_autodeploy_regex_replace_info(
    $info_file_path, PAAU_DEV_ONLY_PATTERN, "");
}

/**
 * Replaces any portion of the specified `.info` file that matches the provided
 * regular expression with the specified string replacement.
 *
 * The `.info` file is modified in-place.
 *
 * @param string $info_file_path
 *   The path to the info file to modify.
 * @param string $regex
 *   The PCRE to use for finding portions to replace in the file.
 * @param string $replacement
 *   The string to fill-in for any matching portions of the file.
 *
 * @throws DrushException
 *   If the file cannot be parsed or saved successfully.
 */
function _pantheon_autodeploy_regex_replace_info($info_file_path, $regex, $replacement) {
  if (!file_exists($info_file_path) || !is_readable($info_file_path)) {
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The specified `.info` file could not be read: @path',
        array('@path' => $info_file_path)));
  }

  $info_data = file_get_contents($info_file_path);

  if ($info_data === FALSE) {
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The specified `.info` file could not be read: @path',
        array('@path' => $info_file_path)));
  }

  $result_data = preg_replace($regex, $replacement, $info_data);

  if (($result_data === NULL) ||
      (file_put_contents($info_file_path, $result_data) === FALSE)) {
    throw new DrushException(
      'AUTODEPLOY_REQUIRED_FILES_MALFORMED',
      dt('The desired section of the specified `.info` file could not be replaced: @path',
        array('@path' => $info_file_path)));
  }
}
