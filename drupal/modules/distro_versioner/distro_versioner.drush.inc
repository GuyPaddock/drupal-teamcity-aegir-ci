<?php
/**
 * Implements hook_drush_command().
 */
function distro_versioner_drush_command() {
  $items['version-distro'] = array(
    'description' => 'Updates the version in the .info files of all custom modules and features included in an installation profile to match the profile version.',
    'aliases'     => array('vdst'),
    'options'     => array(
      'profile'   => 'The installation profile to target. The default is the `exclusive` profile in the current platform.',
    ),
  );

  return $items;
}