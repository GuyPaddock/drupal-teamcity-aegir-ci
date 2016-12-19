<?php
/**
 * @file
 *   Code for the Distro Version-er extension for the Drupal Shell (Drush).
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
require_once('distro_versioner.constants.inc');

require_once('includes/DrushException.php');
require_once('includes/pantheon_autodeploy.info_utils.inc');

//==============================================================================
// Drush Interface Methods
//==============================================================================
/**
 * Implements hook_drush_command().
 */
function distro_versioner_drush_command() {
  $items['version-distro'] = array(
    'bootstrap'   => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    'description' => 'Updates the version in the .info files of all custom modules, features, and themes included under an installation profile or parent module to match the top-level version.',
    'aliases' => array('vdst'),
    'arguments' => array(
      'target' => 'The machine name of the installation profile or module to target.',
    ),
    'options' => array(
      PAAU_OPTION_REPLACE_STRING => 'The string to search for when filling-in versions in child modules, features, and themes. (The default is `PROFILE_VERSION`).',
    ),
    'examples' => array(
      'drush vdst sample' =>
        dt('Updates the `version` in the `.info` files of all custom modules and features in the `sample` installation profile.'),
    ),
  );

  return $items;
}

/**
 * The <code>version-distro</code> (<code>vdst</code>) command.
 *
 * Updates the version in the .info files of all custom modules, features, and
 * themes included under an installation profile or parent module to match the
 * top-level version.
 *
 * @param string $target
 *   The machine name of the installation profile or module to target.
 *
 * @throws DrushException
 *   If the deployment fails.
 */
function drush_distro_versioner_version_distro($target = NULL) {
  try {
    distro_versioner_apply_parent_module_version($target);
  }
  catch (DrushException $ex) {
    drush_set_error($ex->getDrushErrorCode(), $ex->getMessage());
  }
  catch (InvalidArgumentException $ex) {
    drush_set_error('VERSIONER_VALID_OPTION_REQUIRED', $ex->getMessage());
  }
}

//==============================================================================
// Public API Methods
//==============================================================================
/**
 * Applies the version of the specified installation profile, module, or theme
 * to all modules or themes under it.
 *
 * @param string $module_name
 *   The machine name of the module to target.
 *
 * @throws DrushException
 *   If the parent module is missing a version, or sub-modules cannot be
 *   updated.
 */
function distro_versioner_apply_parent_module_version($module_name) {
  $target_object = _distro_versioner_find_target_object($module_name);

  if (empty($target_object['data']['version'])) {
    throw new DrushException(
      'VERSIONER_PARENT_VERSION_REQUIRED',
      dt('The parent object `@target_name` does not have a version specified in its `.info` file.',
        array('@target_name' => $module_name)));
  }

  $parent_version = $target_object['data']['version'];

  $target_info_file     = $target_object['file'];
  $target_info_filepath = $target_info_file->filename;
  $containing_folder    = dirname($target_info_filepath);

  // Automatically excludes `contrib` modules and `libraries`.
  $affected_files =
    _pantheon_autodeploy_find_all_info_under_root(
      $containing_folder,
      array('contrib', 'libraries'));

  if (empty($affected_files)) {
    drush_log(
      dt('Nothing to do -- no `.info` files were found to update under `@target_name`.',
        array('@target_name' => $module_name)),
      'warning');
  }
  else {
    $search_string =
      drush_get_option(PAAU_OPTION_REPLACE_STRING, 'PROFILE_VERSION');

    $search_regex =
      sprintf("/%s/", _pantheon_autodeploy_regex_escape($search_string));

    foreach ($affected_files as $info_file) {
      $file_path = $info_file->filename;

      if ($file_path != $target_info_filepath) {
        drush_log(
          dt('Setting version in `@file_path`.',
            array('@file_path' => $file_path)));

        _pantheon_autodeploy_regex_replace_info(
          $file_path, $search_regex, $parent_version);
      }
    }

    drush_log(
      dt('Finished replacing version placeholders with `@version` in all modules and themes under `@target_name`.',
        array(
          '@version'     => $parent_version,
          '@target_name' => $module_name,
        )),
      'success');
  }
}

//==============================================================================
// Utility Methods
//==============================================================================
/**
 * Finds the installation profile, module, or theme having the specified machine
 * name.
 *
 * @param string $target
 *   The machine name of the installation profile or module to target.
 *
 * @return array
 *   An associative array that contains the following keys:
 *     - file: A <code>stdClass</code> object structured according to the
 *       conventions defined by <code>drush_scan_directory()</code>.
 *     - data: The data parsed from the info file for the target, in the format
 *       returned by <code>drush_drupal_parse_info_file()</code>.
 */
function _distro_versioner_find_target_object($target) {
  if (empty($target)) {
    throw new InvalidArgumentException(
      dt('You need to provide the name of the installation profile or module that needs to be versioned.'));
  }

  $info_files = _pantheon_autodeploy_find_info_by_module_name($target);

  if (empty($info_files)) {
    throw new InvalidArgumentException(
      dt('Could not find an installation profile, module, or theme with the specified name: @name',
        array('@name' => $target)));
  }
  elseif (count($info_files) > 1) {
    throw new InvalidArgumentException(
      dt('There are multiple objects named `@name` (perhaps this is a typo?): [@object_paths]',
        array(
          '@name'         => $target,
          '@object_paths' => implode(', ', array_keys($info_files)),
        )));
  }

  return current($info_files);
}
