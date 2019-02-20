<?php
/**
 * @file
 *   GIT-related utility methods for the Drush Auto-deploy extension for the
 *   Drupal Shell (Drush).
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
 * Clones a GIT repository to a temporary folder and returns the local path to
 * it.
 *
 * <p>This method honors Drush's verbose output mode with verbose GIT output, if
 * the user has requested Drush to be verbose.</p>
 *
 * @param string $uri
 *   The URI to the GIT repository to clone.
 * @param string $branch [optional]
 *   The name of the branch to checkout. Defaults to the repo's default branch.
 *
 * @return string
 *   The absolute path to the temporary folder where the GIT repository
 *   was cloned.
 *
 * @throws DrushException
 *   If the checkout operation fails.
 */
function _pantheon_autodeploy_git_clone_to_temporary_directory($uri,
                                                               $branch = NULL) {
  $tmp_directory = drush_tempdir() . '/' . basename($uri);
  $clone_command =
    'git clone' .
    ' --config core.autocrlf=input'.
    ' %s %s';

  if ($branch !== NULL) {
    $clone_command .= ' --branch ' . drush_escapeshellarg($branch);
  }

  if (drush_get_context('DRUSH_VERBOSE')) {
    $clone_command .= ' --verbose --progress';
  }

  if (!drush_shell_exec($clone_command, $uri, $tmp_directory)) {
    throw new DrushException(
      'AUTODEPLOY_CHECKOUT_FAILED',
      dt('Failed to checkout: @url', array('@url' => $uri)));
  }

  return $tmp_directory;
}

/**
 * Gets the status of the working tree in the specified GIT repository.
 *
 * @param string $working_repo_path
 *   The path to the GIT repository to get status for.
 *
 * @return string
 *   A multi-line string containing the status of the worktree.
 *
 * @throws DrushException
 *   If status cannot be obtained.
 */
function _pantheon_autodeploy_git_status($working_repo_path) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $status_command = $command_prefix . 'status --porcelain';

  if (!drush_shell_exec($status_command, $working_repo_path)) {
    throw new DrushException(
      'AUTODEPLOY_STATUS_FAILED',
      dt('Failed to get GIT status in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }

  return _pantheon_autodeploy_get_shell_output();
}

/**
 * Stages all modified files in the specified GIT repository.
 *
 * @param string $working_repo_path
 *   The path to the GIT repository to stage changes for.
 *
 * @throws DrushException
 *   If changes cannot be staged.
 */
function _pantheon_autodeploy_git_add_all($working_repo_path) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $add_command    = $command_prefix . 'add -A %1$s';

  if (!drush_shell_exec($add_command, $working_repo_path)) {
    throw new DrushException(
      'AUTODEPLOY_STAGING_FAILED',
      dt('Failed to stage changes in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }
}

/**
 * Commits all outstanding changes to the specified GIT repository.
 *
 * <p>This will stage all modified files, commit them, then return the
 * resulting SHA hash of the commit.</p>
 *
 * @param string $working_repo_path
 *   The path to the GIT repository to commit.
 * @param string $message
 *   The message to use for the commit.
 * @param string $author
 *   The name of the author to use for the commit, in
 *   "A U Thor <author@example.com>" format.
 *
 * @throws DrushException
 *   If the staging or commit operations fail.
 */
function _pantheon_autodeploy_git_commit($working_repo_path, $message, $author) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';

  $commit_command     = $command_prefix . 'commit --message=%2$s --author=%3$s';
  $rev_parse_command  = $command_prefix . 'rev-parse HEAD';

  _pantheon_autodeploy_git_add_all($working_repo_path);

  if (!drush_shell_exec($commit_command, $working_repo_path, $message, $author)) {
    throw new DrushException(
      'AUTODEPLOY_COMMIT_FAILED',
      dt('Failed to commit changes in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }

  if (!drush_shell_exec($rev_parse_command, $working_repo_path)) {
    throw new DrushException(
      'AUTODEPLOY_REVID_FAILED',
      dt('Failed to obtain SHA hash of latest commit in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }

  $rev_parse_output = drush_shell_exec_output();

  return $rev_parse_output[0];
}

/**
 * Creates an annotated tag having the specified name that points to the
 * specified commit in the specified GIT repository.
 *
 * @param string $working_repo_path
 *   The path to the GIT repository.
 * @param string $commit_hash
 *   The SHA hash of the target commit
 * @param string $tag_name
 *   The name of the tag to create.
 * @param string $message
 *   The message to use for the tag.
 *
 * @throws DrushException
 *   If the staging or commit operations fail.
 */
function _pantheon_autodeploy_git_tag_commit($working_repo_path, $commit_hash,
                                             $tag_name, $message) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $tag_command    = $command_prefix . 'tag --annotate --message=%2$s %3$s %4$s';

  $command_result =
    drush_shell_exec(
      $tag_command,
      $working_repo_path,
      $message,
      $tag_name,
      $commit_hash);

  if (!$command_result) {
    throw new DrushException(
      'AUTODEPLOY_TAGGING_FAILED',
      dt('Failed to create tag @tag on latest commit in `@path`: @output',
        array(
          '@tag'    => $tag_name,
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }
}

/**
 * Pushes changes in the specified GIT repository to the specified remote.
 *
 * @param string $working_repo_path
 *   The path to the GIT repository to push.
 * @param string $remote
 *   The name of the remote to which changes will be pushed. The default is
 *   "origin", which is the most common name for the default GIT remote.
 * @param string $branch
 *   The name of the branch that is to be pushed. The default is "master",
 *   which is the most common name for the default GIT branch.
 *
 * @throws DrushException
 *   If the repository cannot be pushed.
 */
function _pantheon_autodeploy_git_push($working_repo_path, $remote = 'origin',
                                       $branch = 'master') {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $push_command   = $command_prefix . 'push --tags %2$s %3$s';

  if (!drush_shell_exec($push_command, $working_repo_path, $remote, $branch)) {
    throw new DrushException(
      'AUTODEPLOY_PUSH_FAILED',
      dt('Failed to push GIT repository in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }
}
