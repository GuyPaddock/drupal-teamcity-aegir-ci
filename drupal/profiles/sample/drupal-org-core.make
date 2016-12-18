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
projects[drupal][patch][] = "https://www.drupal.org/files/issues/image_style_deliver_shouldnt_use_module_invoke_all-6.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/options_drupal7-1919338-58.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/php_5_5_imagerotate-2215369-32.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/1349080-231-d7-move-access-to-join-condition_rework-placeholders.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/D7_improve_theme_registry-2339447-46.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/form-single-submit-1705618-85.patch"
projects[drupal][patch][] = "https://www.drupal.org/files/issues/drupal-user_menu_link_alter_expects_module-1719280-6-7.x.patch"
