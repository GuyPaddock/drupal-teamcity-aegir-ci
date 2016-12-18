#!/usr/local/bin/drush @hostmaster php-script
<?php
/**
 * @file
 * TeamCity Drupal CI build clean-up script.
 *
 * Add this script to Aegir's crontab to automate clean-up of the sites and
 * platforms from stale CI builds.
 *
 * The script must be passed a "platform prefix" as its only argument. The
 * platform prefix is simply the first part of the path to builds that
 * you want to clean-up automatically.
 *
 * For example:
 * /var/aegir/platforms/sample_7x
 *
 * The above would clean-up builds like "/var/aegir/platforms/sample_7x_b1",
 * "/var/aegir/platforms/sample_7x_b2", and so on that are older than 7 days.
 *
 * To clean-up all CI builds, just pass the platform root folder path:
 * /var/aegir/platforms
 *
 * This script has safeguards to prevent removing the "hostmaster" platform,
 * but it's not foolproof. In general, avoid passing this script the path
 * to the hostmaster platform for best results.
 *
 * This script will only remove builds owned by the "teamcity" Aegir client.
 * To prevent a build from being cleaned-up, simply assign it to a different
 * client and it will be ignored. (This is the best practice if a developer
 * needs to keep a build around longer than normal.)
 *
 * This script cleans-up platforms owned by the TeamCity client once they
 * no longer have any sites. As such, platforms will not be deleted
 * immediately, but usually will be deleted on the next run after the sites
 * on the platform have been marked deleted.
 *
 * In case you need to resurrect a deleted site, it is worth noting that sites
 * this script purges will typically still be backed-up to Aegir's back-up
 * folder (/var/aegir/backups). This script does not clean-up any files
 * written there, though you can combine this script with a tmpreaper
 * cronjob if you wish.
 *
 * @author Guy Paddock (guy.paddock@redbottledesign.com)
 */

// Client name that owns CI builds
define('CLIENT_NAME', 'teamcity');

// Delete empty auto-generated platforms older than a certain period
define('ONE_DAY', 24 * 60 * 60);
define('ONE_WEEK', 7 * ONE_DAY);
define('TWO_DAYS', 2 * ONE_DAY);

define('CUTOFF_DATE', ONE_WEEK);

// Provide the platform location via the command line
$platform_prefix = drush_shift();

if (empty($platform_prefix)) {
  fwrite(STDERR, "Must specify platform prefix as first argument.\n");
  fwrite(STDERR, "Usage: ${argv[0]} </var/aegir/platforms/PREFIX>\n\n");
  exit(1);
}

echo "Checking for sites to cleanup...\n";
cleanup_sites($platform_prefix);

echo "Checking for platforms to clean-up...\n";
cleanup_platforms($platform_prefix);

echo "Build clean-up complete.\n\n";

function cleanup_sites($publish_path_prefix) {
  $sql =
    "SELECT
       site_node.nid         AS `site_nid`,
       site.status           AS `site_status`,
       platform_node.created AS `created_at`,
       platform.status       AS `platform_status`,
       platform.publish_path AS `platform_path`

     FROM {hosting_site} AS site

     INNER JOIN {node} AS site_node
     ON site.nid = site_node.nid

     INNER JOIN {hosting_platform} AS platform
     ON site.platform = platform.nid

     INNER JOIN {node} AS platform_node
     ON platform.nid = platform_node.nid

     INNER JOIN {hosting_client} AS client
     ON site.client = client.nid

     WHERE client.uname = :client_name
       AND (platform_node.created < :created_at)
       AND (site.status IN (:site_statuses))
       AND (platform.status IN (:platform_statuses))
       AND (platform.publish_path NOT LIKE '%%hostmaster%%')
       AND (platform.publish_path LIKE :publish_path)";

  $active_site_statuses     = array(HOSTING_SITE_DISABLED, HOSTING_SITE_ENABLED);
  $active_platform_statuses = array(HOSTING_PLATFORM_ENABLED);

  $query_params = array(
    ':client_name'       => CLIENT_NAME,
    ':created_at'        => REQUEST_TIME - CUTOFF_DATE,
    ':site_statuses'     => $active_site_statuses,
    ':platform_statuses' => $active_platform_statuses,
    ':publish_path'      => "{$publish_path_prefix}%",
  );

  $rows = db_query($sql, $query_params);

  foreach ($rows as $row) {
    $site_nid           = $row->site_nid;
    $platform_path      = $row->platform_path;
    $created_timestamp  = date(DATE_W3C, $row->created_at);

    drush_log(
      dt(
        'Queuing stale site @site_nid (`@platform_path` created on `@created_at`) for deletion.',
        array(
          '@site_nid'       => $site_nid,
          '@platform_path'  => $platform_path,
          '@created_at'     => $created_timestamp,
        )),
        'ok');

    hosting_add_task($site_nid, 'delete');
  }
}

function cleanup_platforms($publish_path_prefix) {
  $sql =
    "SELECT platform_node.nid      AS `platform_nid`,
            platform_node.created  AS `created_at`,
            platform.status        AS `platform_status`,
            platform.publish_path  AS `platform_path`

     FROM {hosting_platform} AS platform

     INNER JOIN {node} AS platform_node
     ON platform.nid = platform_node.nid

     LEFT JOIN 
     (
       SELECT site.platform AS `platform_nid`,
              COUNT(1)      AS count
       FROM {hosting_site} AS site
       GROUP BY platform_nid
     ) AS site
     ON platform.nid = site.platform_nid

     WHERE ((site.count = 0) OR (site.count IS NULL))
       AND (platform_node.created < :created_at)
       AND (platform.publish_path NOT LIKE '%%hostmaster%%')
       AND (platform.publish_path LIKE :publish_path);";

  $query_params = array(
    ':created_at'   => REQUEST_TIME - CUTOFF_DATE,
    ':publish_path' => "{$publish_path_prefix}%",
  );

  $rows = db_query($sql, $query_params);

  foreach ($rows as $row) {
    $platform_nid       = $row->platform_nid;
    $platform_status    = $row->platform_status;
    $platform_path      = $row->platform_path;
    $created_timestamp  = date(DATE_W3C, $row->created_at);

    if ($platform_status == HOSTING_PLATFORM_ENABLED) {
      drush_log(
        dt(
          'Queuing empty platform @platform_nid (`@platform_path` created on `@created_at`) for deletion.',
          array(
            '@platform_nid'   => $platform_nid,
            '@platform_path'  => $platform_path,
            '@created_at'     => $created_timestamp,
          )),
        'ok');

      hosting_add_task($platform_nid, 'delete');
    }

    else if ($platform_status == HOSTING_PLATFORM_DELETED) {
      drush_log(
        dt(
          'Purging deleted platform @platform_nid (`@platform_path` created on `@created_at`).',
          array(
            '@platform_nid'   => $platform_nid,
            '@platform_path'  => $platform_path,
            '@created_at'     => $created_timestamp,
          )),
        'ok');

      // FIXME: node_delete() is not working.
      node_delete($platform_nid);
      @rmdir($publish_path);
    }
  }
}
