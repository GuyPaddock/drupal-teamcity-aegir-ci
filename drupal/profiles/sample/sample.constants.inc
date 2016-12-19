<?php
/**
 * @file
 *   Constants for the "Sample" installation profile.
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

/**
 * Machine name for the installation profile.
 *
 * @var string
 */
define('SA_PROFILE_NAME', 'sample');

/**
 * The human-friendly name of this profile.
 *
 * @var string
 */
define('SA_PROFILE_FRIENDLY_NAME', 'Sample Installation');

/**
 * Default version number that appears when developing on the profile locally.
 *
 * <p><strong>DO NOT MODIFY:</strong> This is not used for production builds,
 * nor for builds that have run through CI, since the Drush Distro Version-er
 * automatically fills-in the versions of sub-modules.</p>
 */
define('PROFILE_VERSION', '7.x-1.0-dev');
