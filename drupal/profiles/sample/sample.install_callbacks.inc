<?php
/**
 * @file
 *  Contains Batch API callbacks used during installation of the "Sample"
 *  installation profile.
 *
 *  © 2015 Red Bottle Design, LLC. All rights reserved.
 *
 * @author  Guy Paddock (guy.paddock@redbottledesign.com)
 */

/**
 * BatchAPI content import callback.
 *
 * @see sample_import_content()
 */
function _sample_import($operation, $type, &$context) {
  $migration = Migration::getInstance($operation);
  $totalCount = $migration->sourceCount();

  $message = 'Importing @operation content (@count)...';
  $variables = array(
    '@operation' => $operation,
    '@count' => format_plural($totalCount, '1 record', '@count records'),
  );

  watchdog(
    SA_PROFILE_NAME,
    $message,
    $variables,
    WATCHDOG_DEBUG);

  $context['message'] = t($message, $variables);

  /* @var Migration $migration */
  $result = $migration->processImport();

  // Allow resuming when incomplete.
  if ($result == MigrationBase::RESULT_INCOMPLETE) {
    $processedCount = $migration->processedCount();

    $context['finished'] = $processedCount / $totalCount;

    $context['message']
      = t('Importing @operation content - @current of @total...',
        array(
          '@operation' => $operation,
          '@current'
            => format_plural(
                $migration->processedCount(),
                '1 record',
                '@count records'),
            '@total'
            => format_plural(
                $migration->sourceCount(),
                '1 record',
                '@count records'),
        ));
  }
  else {
    $context['finished'] = 1;
  }
}