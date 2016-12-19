<?php
/**
 * @file
 *   Shell-related utility methods for the Drush Auto-deploy extension for the
 *   Drupal Shell (Drush).
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
 */

/**
 * Gets the output from the last command in a single, multi-line string.
 *
 * @return string
 *   The ouput from the last shell command.
 */
function _pantheon_autodeploy_get_shell_output() {
  $last_output = drush_shell_exec_output();

  return implode(PHP_EOL, $last_output);
}
