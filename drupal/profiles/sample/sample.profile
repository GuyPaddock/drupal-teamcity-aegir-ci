<?php
/**
 * @file
 *  <p>Enables modules and site configuration for a "Sample" site
 *  installation.</p>
 *
 *  <p>© 2015 Red Bottle Design, LLC. All rights reserved.</p>
 *
 * @author  Guy Paddock (guy.paddock@redbottledesign.com)
 */
require_once('sample.constants.inc');

if (!function_exists('profiler_v2')) {
  require_once('libraries/profiler/profiler.inc');
}

profiler_v2(SA_PROFILE_NAME);

/**
 * <p>Implements <code>hook_profiler_form_FORM_ID_alter()</code> for
 * <code>install_configure_form()</code>.</p>
 *
 * <p>Allows the profile to alter the site configuration form.</p>
 */
function sample_profiler_form_install_configure_form_alter(&$form, $form_state) {
  // Pre-populate the site name
  $form['site_information']['site_name']['#default_value'] = 'Sample';
}