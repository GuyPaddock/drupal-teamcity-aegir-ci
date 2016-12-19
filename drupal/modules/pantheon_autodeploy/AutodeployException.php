<?php
/**
 * @file
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
 * Parent type for exceptions that occur when Auto-deploy is performing an
 * operation.
 */
class AutodeployException extends Exception {
  private $drushErrorCode;

  /**
   * Constructor for <code>AutodeployException</code>.
   *
   * @param string $code
   *   The Drush-friendly error code.
   * @param string $message
   *   The human-friendly error message.
   * @param Exception|null $previous
   *   The previous exception used for the exception chaining.
   */
  public function __construct($drushErrorCode, $message, Exception $previous = null) {
    parent::__construct($message, 0, $previous);

    $this->drushErrorCode = $drushErrorCode;
  }

  /**
   * Gets the error code that can be passed to <code>drush_set_error()</code>.
   *
   * @return string
   *   The Drush error code for this exception.
   */
  public function getDrushErrorCode() {
    return $this->drushErrorCode;
  }
}
