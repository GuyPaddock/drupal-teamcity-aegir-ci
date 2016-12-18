; Drush Makefile for fetching the version of Drupal core used by the
; "Sample" installation profile.
;
; © 2015 Red Bottle Design, LLC. All rights reserved.
;
; @author Guy Paddock (guy.paddock@redbottledesign.com)

api = 2
core = 7.x

projects[drupal][download][type] = git
projects[drupal][download][url] = "https://github.com/pantheon-systems/drops-7"
projects[drupal][download][tag] = "7.53"

; Patches for Core
projects[drupal][patch][] = "https://www.drupal.org/files/issues/fix-shortcut-set-save-1175700-24.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/shortcut_no_limit_D7-682000-70.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/1210178-15-undefined-index-bundles_0.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/1349080-231-d7-move-access-to-join-condition_rework-placeholders.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/form-single-submit-1705618-85.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/1093420-22.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/ignore_front_end_vendor-2329453-111.patch"
