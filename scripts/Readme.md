# Utility Scripts for Aegir-Based CI
This folder contains scripts to assist with maintaining an Aegir-based CI
environment.

## Build Cleanup (`build_cleanup.php`)
This script can be run to cleanup all sites + Drupal platforms that were created
longer than two weeks ago.

It accepts a path prefix (like `/var/aegir/platforms/sample_7x_b15`, for
build #15 of the `sample` installation profile, or
`/var/aegir/platforms/sample_7x` for all builds of `sample`) as its only
command-line argument. It then locates all sites and platforms older than two
weeks, and queues them up for backup and deletion through Aegir.

There are two versions of the script, based on what version of Aegir you are
using:
- `aegir-2.x/build_cleanup.php` - for the 2.x version of Aegir that is based on
   Drupal 6.x.
- `aegir-3.x/build_cleanup.php` - for the 3.x version of Aegir that is based on
   Drupal 7.x.

The script can either be used stand-alone, or triggered by Cron.

A sample Aegir crontab that can invoke the script is included in
`sample-crontab`. The sample crontab logs clean-up runs to
`/var/log/aegir/cleanup.log`, using `timestamp.awk` to prefix each log line with
a timestamp. The same crontab also uses the
[`tmpreaper`](http://manpages.ubuntu.com/manpages/wily/man8/tmpreaper.8.html)
utility to clean-up both backup copies of sites and Drush's module cache that
were created more than 30 days ago. The sample crontab assumes that all of the
scripts are installed in `/var/aegir/scripts/`.
