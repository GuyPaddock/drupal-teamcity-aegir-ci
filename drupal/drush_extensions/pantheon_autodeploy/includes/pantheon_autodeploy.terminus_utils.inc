<?php
/**
 * @file
 *   Pantheon Terminus-related utility methods for the Drush Auto-deploy
 *   extension for the Drupal Shell (Drush).
 *
 *   Copyright (C) 2015-2017  Red Bottle Design, LLC.
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

/**
 * Deploys the specified site to the specified environment on Pantheon.
 *
 * @param string $site_id
 *   The UUID for the site that is to be deployed.
 * @param string $target_environment
 *   The name of the environment to which the site is to be deployed. Must be
 *   one of the valid environments for the target site.
 * @param string $message
 *   The message ("note") to leave in the Pantheon deployment log.
 *
 * @throws DrushException
 *   If an error occurs while interacting with Terminus.
 */
function _pantheon_autodeploy_pantheon_deploy_site($site_id,
                                                   $target_environment,
                                                   $message) {
  _pantheon_autodeploy_ensure_pantheon_authenticated();

  // This Terminus command works with the name of the site instead of UUID
  $site_name = _pantheon_autodeploy_pantheon_site_id_to_name($site_id);

  $deploy_command =
    'terminus env:deploy %s.%s --no-interaction --cc --note=%s';

  $success =
    drush_shell_exec(
      $deploy_command,
      $site_name,
      $target_environment,
      $message
    );

  if (!$success) {
    throw new DrushException(
      'AUTODEPLOY_SITE_DEPLOY_FAILED',
      dt('Failed to deploy Pantheon site `@site_name` (@site_id) to `@environment`: @output',
        array(
          '@site_name'    => $site_name,
          '@site_id'      => $site_id,
          '@environment'  => $target_environment,
          '@output'       => _pantheon_autodeploy_get_shell_output(),
        )));
  }
}

/**
 * Maps a Pantheon site name to a Pantheon site UUID.
 *
 * @param string $site_name
 *   The site machine name to convert to a site UUID.
 *
 * @return string
 *   The UUID of the site.
 *
 * @throws InvalidArgumentException
 *   If there is no site in the list with the given name.
 *
 * @throws DrushException
 *   If the site list cannot be obtained.
 */
function _pantheon_autodeploy_pantheon_site_name_to_id($site_name) {
  $id_to_name_map = _pantheon_autodeploy_pantheon_get_site_map();
  $name_to_id_map = array_flip($id_to_name_map);

  if (!isset($name_to_id_map[$site_name])) {
    throw new InvalidArgumentException(
      dt("There is no site with the given name: @name",
        array('@name' => $site_name)));
  }

  return $name_to_id_map[$site_name];
}

/**
 * Maps a Pantheon site UUID to a site name.
 *
 * <p>This is often required when working with Terminus, as most Terminus
 * commands do not accept a site UUID in place of the site name when
 * specifying the target site.</p>
 *
 * @param string $site_id
 *   The ID to convert to a site name.
 *
 * @return string
 *   The machine name of the site.
 *
 * @throws InvalidArgumentException
 *   If there is no site in the list with the given ID.
 *
 * @throws DrushException
 *   If the site list cannot be obtained.
 */
function _pantheon_autodeploy_pantheon_site_id_to_name($site_id) {
  $id_to_name_map = _pantheon_autodeploy_pantheon_get_site_map();

  if (!isset($id_to_name_map[$site_id])) {
    throw new InvalidArgumentException(
      dt("There is no site with the given ID: @id", array('@id' => $site_id)));
  }

  return $id_to_name_map[$site_id];
}

/**
 * Gets the map of Pantheon site IDs to site names.
 *
 * <p>For performance reasons, this list is only fetched once and then cached
 * for the duration of the command execution.</p>
 *
 * @return array
 *   An associative array, where the keys are Pantheon site IDs, and the values
 *   are site machine names.
 *
 * @throws DrushException
 *   If the site list cannot be obtained.
 */
function _pantheon_autodeploy_pantheon_get_site_map() {
  static $id_to_name_map = NULL;

  if (empty($id_to_name_map)) {
    _pantheon_autodeploy_ensure_pantheon_authenticated();

    // Name,UUID
    // We are skipping the header row.
    $site_list_command =
      'terminus site:list --format=csv --fields=name,id | tail +2';

    if (!drush_shell_exec($site_list_command)) {
      throw new DrushException(
        'AUTODEPLOY_SITE_LIST_FAILED',
        dt('Failed to get Pantheon sites list: @output',
          array(
            '@output' => _pantheon_autodeploy_get_shell_output(),
          )));
    }

    $id_to_name_map = array();

    /* @var array $site_list_lines */
    $site_list_lines = drush_shell_exec_output();

    foreach ($site_list_lines as $line) {
      list($site_name, $site_id) = explode(',', $line);

      $id_to_name_map[$site_id] = $site_name;
    }
  }

  return $id_to_name_map;
}

/**
 * Ensures we have a valid session through Terminus to Pantheon.
 *
 * <p>This only attempts authentication once per command run. It must be run
 * at least once to refresh access tokens, as they seem to expire after a few
 * hours.</p>
 *
 * @throws DrushException
 *   If authentication through Terminus fails.
 */
function _pantheon_autodeploy_ensure_pantheon_authenticated() {
  static $authenticated = FALSE;

  if (!$authenticated) {
    $authenticate_command = 'terminus auth:login';

    if (!drush_shell_exec($authenticate_command)) {
      throw new DrushException(
        'AUTODEPLOY_PANTHEON_LOGIN_FAILED',
        dt('Failed to authenticate with Pantheon: @output',
          array(
            '@output' => _pantheon_autodeploy_get_shell_output(),
          )));
    }

    $authenticated = TRUE;
  }
}

/**
 * Validates that a given string is a valid Pantheon environment name.
 *
 * @param string $environment
 *   The environment name to validate.
 *
 * @throws InvalidArgumentException
 *   If the given value is not a valid environment name.
 */
function _pantheon_autodeploy_validate_pantheon_environment($environment) {
  $allowed_environments =
    array(
      PAAU_PANTHEON_ENV_DEV,
      PAAU_PANTHEON_ENV_TEST,
      PAAU_PANTHEON_ENV_LIVE,
    );

  if (!in_array($environment, $allowed_environments)) {
    throw new InvalidArgumentException(
      dt('`@name` is not a valid Pantheon environment name. Valid environments are: [@types]',
        array(
          '@name'  => $environment,
          '@types' => implode(', ', $allowed_environments),
        )));
  }
}