<?php
/**
 * @file
 *   File and directory manipulation utility methods for the Drush Auto-deploy
 *   extension for the Drupal Shell (Drush).
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
 */

/**
 * Forcibly deletes the given directory, if it exists.
 *
 * <p>The deletion is logged beforehand, to assist with debugging.</p>
 *
 * @param string $path
 *   The path to delete.
 */
function _pantheon_autodeploy_force_delete_dir($path) {
  if (is_dir($path)) {
    drush_log(dt('Deleting `@path`.', array('@path' => $path)));
    drush_delete_dir($path, TRUE);
  }
}
/**
 * Creates each of the parent directories for the specified path.
 *
 * <p>If the directories already exist, this method does nothing.</p>
 *
 * @param string $file_path
 *   The path to the file or directory for which parent folders are to be
 *   created.
 */
function _pantheon_autodeploy_mk_parent_paths($file_path) {
  $parent_path = dirname($file_path);

  if (!is_dir($parent_path)) {
    drush_log(dt('Creating path `@path`.', array('@path' => $file_path)));
    drush_mkdir($parent_path);
  }
}

/**
 * Creates a Drush-style file object out of a path.
 *
 * <p>The resulting object is compatible with the output normally returned by
 * <code>drush_scan_directory()</code>.</p>
 *
 * @param string $path
 *   The path from which to create a file object from.
 *
 * @return stdClass
 *   The resulting file object.
 */
function _pantheon_autodeploy_path_to_file_object($path) {
  $base_name = basename($path);

  $file = new stdClass();

  $file->filename = $path;
  $file->basename = $base_name;
  $file->name     = substr($base_name, 0, strrpos($base_name, '.'));

  return $file;
}

/**
 * Converts an absolute path into a path relative to the given path.
 *
 * <p>If <code>$root_path</code> does not contain <code>$path</code>, then
 * <code>$path</code> is returned without modification.</p>
 *
 * @param string $path
 *   The absolute path to convert.
 * @param $root_path
 *   The path that contains <code>$path</code>, and the path to which the
 *   returned path is to be relative.
 *
 * @return string
 *   The path represented by <code>$path</code>, rewritten to be relative to
 *   <code>$root_path</code>.
 */
function _pantheon_autodeploy_absolute_to_relative_path($path, $root_path) {
  $safe_root_path = _pantheon_autodeploy_regex_escape($root_path);
  $relative_path  = preg_replace("/^{$safe_root_path}/", "", $path);

  return $relative_path;
}
