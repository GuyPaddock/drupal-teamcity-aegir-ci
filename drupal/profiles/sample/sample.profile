<?php
/**
 * @file
 *  Enables modules and site configuration for a "Sample" site installation.
 *
 *  © 2015 Red Bottle Design, LLC. All rights reserved.
 *
 * @author  Guy Paddock (guy.paddock@redbottledesign.com)
 */
require_once('sample.constants.inc');

if (!function_exists('profiler_v2')) {
  require_once('libraries/profiler/profiler.inc');
}

profiler_v2(SA_PROFILE_NAME);

/**
 * Implements hook_profiler_form_FORM_ID_alter() for install_configure_form().
 *
 * Allows the profile to alter the site configuration form.
 */
function sample_profiler_form_install_configure_form_alter(&$form, $form_state) {
  // Pre-populate the site name
  $form['site_information']['site_name']['#default_value']
    = SA_PROFILE_FRIENDLY_NAME;
}