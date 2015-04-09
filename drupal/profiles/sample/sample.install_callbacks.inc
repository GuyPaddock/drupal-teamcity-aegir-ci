<?php
/**
 * @file
 *  <p>Contains Batch API callbacks used during installation of the
 *  "Sample" installation profile.</p>
 *
 *  <p>© 2015 Red Bottle Design, LLC. All rights reserved.</p>
 *
 * @author  Guy Paddock (guy.paddock@redbottledesign.com)
 */

/**
 * BatchAPI callback.
 *
 * @see sample_import_content()
 */
function _sample_import($operation, $type, &$context) {
  watchdog(
    SA_PROFILE_NAME,
    'Importing content @operation @type',
    array(
      '@operation' => $operation,
      '@type' => $operation),
    WATCHDOG_DEBUG);
 
  $context['message'] = t('@operation', array('@operation' => $type));
  
  $migration = Migration::getInstance($operation);
  $migration->processImport();
}