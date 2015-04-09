; Drush Makefile for fetching the version of Drupal core used by the
; "Sample" installation profile.
;
; © 2015 Red Bottle Design, LLC. All rights reserved.
;
; @author Guy Paddock (guy.paddock@redbottledesign.com)

api = 2
core = 7.x
projects[drupal][version] = 7.35

; Patches for Core
projects[drupal][patch][] = "https://www.drupal.org/files/issues/fix-shortcut-set-save-1175700-24.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/shortcut_no_limit_D7-682000-70.patch"