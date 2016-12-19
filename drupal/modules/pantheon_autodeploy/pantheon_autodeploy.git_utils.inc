<?php
/**
 * @file
 *   GIT-related utility methods for the Drush Auto-deploy extension for the Drupal Shell (Drush).
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
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
 * @param boolean $checkout_head
 *   Whether or not to checkout all files in HEAD after the clone operation.
 *   The default is <code>TRUE</code>.
 *
 * @return string
 *   The absolute path to the temporary folder where the GIT repository
 *   was cloned.
 *
 * @throws AutodeployException
 *   If the checkout operation fails.
 */
function _pantheon_autodeploy_git_clone_to_temporary_directory($uri, $checkout_head = TRUE) {
  $tmp_directory = drush_tempdir() . '/' . basename($uri);
  $clone_command = 'git clone %s %s';

  if (!$checkout_head) {
    $clone_command .= ' --no-checkout';
  }

  if (drush_get_context('DRUSH_VERBOSE')) {
    $clone_command .= ' --verbose --progress';
  }

  if (!drush_shell_exec($clone_command, $uri, $tmp_directory)) {
    throw new AutodeployException(
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
 * @throws AutodeployException
 *   If status cannot be obtained.
 */
function _pantheon_autodeploy_git_status($working_repo_path) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $status_command = $command_prefix . 'status';

  if (!drush_shell_exec($status_command, $working_repo_path)) {
    throw new AutodeployException(
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
 * @throws AutodeployException
 *   If changes cannot be staged.
 */
function _pantheon_autodeploy_git_add_all($working_repo_path) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $add_command    = $command_prefix . 'add %1$s';

  if (!drush_shell_exec($add_command, $working_repo_path)) {
    throw new AutodeployException(
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
 *   "A U Thor <author@example.com> format".
 *
 * @throws AutodeployException
 *   If the staging or commit operations fail.
 */
function _pantheon_autodeploy_git_commit($working_repo_path, $message, $author) {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';

  $commit_command     = $command_prefix . 'commit --message=%2$s --author=%3$s';
  $rev_parse_command  = $command_prefix . 'rev-parse HEAD';

  _pantheon_autodeploy_git_add_all($working_repo_path);

  if (!drush_shell_exec($commit_command, $working_repo_path, $message, $author)) {
    throw new AutodeployException(
      'AUTODEPLOY_COMMIT_FAILED',
      dt('Failed to commit changes in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }

  if (!drush_shell_exec($rev_parse_command, $working_repo_path)) {
    throw new AutodeployException(
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
 * @throws AutodeployException
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
    throw new AutodeployException(
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
 * @throws AutodeployException
 *   If the repository cannot be pushed.
 */
function _pantheon_autodeploy_git_push($working_repo_path, $remote = 'origin',
                                       $branch = 'master') {
  $command_prefix = PAAU_GIT_WITH_REPO_PATTERN . ' ';
  $push_command   = $command_prefix . 'push %2$s %3$s';

  if (!drush_shell_exec($push_command, $working_repo_path, $remote, $branch)) {
    throw new AutodeployException(
      'AUTODEPLOY_PUSH_FAILED',
      dt('Failed to push GIT repository in `@path`: @output',
        array(
          '@path'   => $working_repo_path,
          '@output' => _pantheon_autodeploy_get_shell_output(),
        )));
  }
}

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
