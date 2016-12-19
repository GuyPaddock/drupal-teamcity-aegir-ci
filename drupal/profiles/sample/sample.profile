<?php
/**
 * @file
 *   Enables modules and site configuration for a "Sample" site installation.
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
