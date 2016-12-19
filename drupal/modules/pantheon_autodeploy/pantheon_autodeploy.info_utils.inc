<?php
/**
 * @file
 *   Drupal `.info` file manipulation utility methods for the Drush Auto-deploy
 *   extension for the Drupal Shell (Drush).
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
 */

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

  $profile_path   = dirname($profile->filename);
  $info_file_path = sprintf("%s/%s", $profile_path, $info_filename);

  return drush_drupal_parse_info_file($info_file_path);
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
 * @throws AutodeployException
 *   If the file cannot be parsed.
 */
function _pantheon_autodeploy_parse_dev_only_info($info_file_path) {
  $result    = array();
  $info_data = file_get_contents($info_file_path);

  if ($info_data === FALSE) {
    throw new AutodeployException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The specified `.info` file could not be read: @path',
        array('@path' => $info_file_path)));
  }

  $matches = array();

  if (preg_match(PAAU_DEV_ONLY_PATTERN, $info_data, $matches)) {
    $result = _drush_drupal_parse_info_file($matches[1]);

    if ($result === FALSE) {
      throw new AutodeployException(
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
 * @throws AutodeployException
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
 * @throws AutodeployException
 *   If the file cannot be parsed or saved successfully.
 */
function _pantheon_autodeploy_regex_replace_info($info_file_path, $regex, $replacement) {
  if (!file_exists($info_file_path) || !is_readable($info_file_path)) {
    throw new AutodeployException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The specified `.info` file could not be read: @path',
        array('@path' => $info_file_path)));
  }

  $info_data = file_get_contents($info_file_path);

  if ($info_data === FALSE) {
    throw new AutodeployException(
      'AUTODEPLOY_REQUIRED_FILES_MISSING',
      dt('The specified `.info` file could not be read: @path',
        array('@path' => $info_file_path)));
  }

  $result_data = preg_replace($regex, $replacement, $info_data);

  if (($result_data === NULL) ||
      (file_put_contents($info_file_path, $result_data) === FALSE)) {
    throw new AutodeployException(
      'AUTODEPLOY_REQUIRED_FILES_MALFORMED',
      dt('The desired section of the specified `.info` file could not be replaced: @path',
        array('@path' => $info_file_path)));
  }
}
