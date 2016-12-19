<?php
/**
 * @file
 *   Contains Batch API callbacks used during installation of the "Sample"
 *   installation profile.
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
