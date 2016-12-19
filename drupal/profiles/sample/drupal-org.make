; Drush Makefile for fetching all modules used by the "Sample"
; installation profile.
;
; This file triggers Drush to download the contrib modules and patches
; that the profile requires. This file is automatically discovered by
; Drush after it has downloaded Drupal as part of "build-sample.make".
;
; © 2015 Red Bottle Design, LLC. All rights reserved.
;
; @author Guy Paddock (guy.paddock@redbottledesign.com)

; Core version
; ------------
; Each makefile should begin by declaring the core version of Drupal that all
; projects should be compatible with.
core = 7.x

; API version
; ------------
; Every makefile needs to declare its Drush Make API version. This version of
; drush make uses API version `2`.
api = 2

; Modules
; --------

;===============================================================================
; Modules for Content Import/Export
;===============================================================================
projects[migrate][type] = "module"
projects[migrate][version] = "2.7"
projects[migrate][subdir] = "contrib"

projects[migrate_extras][type] = "module"
projects[migrate_extras][version] = "2.5"
projects[migrate_extras][subdir] = "contrib"
projects[migrate_extras][patch][] = "https://www.drupal.org/files/migrate_extras-2.5-add_support_for_uuid-1870886-4-do-not-test.patch"

projects[uuid_menu_links][type] = "module"
projects[uuid_menu_links][subdir] = "contrib"
projects[uuid_menu_links][download][type] = git
projects[uuid_menu_links][download][url] = "http://git.drupal.org/sandbox/aspilicious/1494440.git"
projects[uuid_menu_links][download][revision] = 7fceaf8e5d46c2ab2bf36b97b2bc89a2b1d21d17
projects[uuid_menu_links][download][branch] = "7.x-1.x"

;===============================================================================
; Modules for Development
;===============================================================================
; <developmentOnly>
projects[devel][type] = "module"
projects[devel][version] = "1.5"
projects[devel][subdir] = "contrib"

; Dev is needed because there is no stable release
projects[devel_themer][type] = "module"
projects[devel_themer][subdir] = "contrib"
projects[devel_themer][download][type] = git
projects[devel_themer][download][revision] = cf347e10353260c45783ea0ac0ad9986f438ed8d
projects[devel_themer][download][branch] = "7.x-1.x"

projects[forminspect][type] = "module"
projects[forminspect][version] = "1.1"
projects[forminspect][subdir] = "contrib"

projects[simplehtmldom][type] = "module"
projects[simplehtmldom][version] = "1.12"
projects[simplehtmldom][subdir] = "contrib"
; </developmentOnly>

;===============================================================================
; Modules for E-Commerce
;===============================================================================

;===============================================================================
; Modules for API
;===============================================================================
projects[date][type] = "module"
projects[date][version] = "2.9"
projects[date][subdir] = "contrib"

projects[entity][type] = "module"
projects[entity][version] = "1.8"
projects[entity][subdir] = "contrib"

projects[libraries][type] = "module"
projects[libraries][version] = "2.3"
projects[libraries][subdir] = "contrib"

projects[token][type] = "module"
projects[token][version] = "1.6"
projects[token][subdir] = "contrib"

;===============================================================================
; Modules for Layout and Site Structure
;===============================================================================
projects[context][type] = "module"
projects[context][version] = "3.6"
projects[context][subdir] = "contrib"

projects[context_omega][type] = "module"
projects[context_omega][version] = "1.1"
projects[context_omega][subdir] = "contrib"

projects[ctools][type] = "module"
projects[ctools][version] = "1.7"
projects[ctools][subdir] = "contrib"

projects[ds][type] = "module"
projects[ds][version] = "2.7"
projects[ds][subdir] = "contrib"

projects[entityreference][type] = "module"
projects[entityreference][version] = "1.1"
projects[entityreference][subdir] = "contrib"

projects[eva][type] = "module"
projects[eva][version] = "1.2"
projects[eva][subdir] = "contrib"

projects[features][type] = "module"
projects[features][version] = "2.4"
projects[features][subdir] = "contrib"

projects[features_extra][type] = "module"
projects[features_extra][version] = "1.0-beta1"
projects[features_extra][subdir] = "contrib"
projects[features_extra][patch][] = "https://www.drupal.org/files/issues/features_extra-add-date-format-support-1279928-49.patch"

projects[field_group][type] = "module"
projects[field_group][version] = "1.4"
projects[field_group][subdir] = "contrib"

projects[views][type] = "module"
projects[views][version] = "3.10"
projects[views][subdir] = "contrib"

projects[rules][type] = "module"
projects[rules][version] = "2.9"
projects[rules][subdir] = "contrib"

projects[strongarm][type] = "module"
projects[strongarm][version] = "2.0"
projects[strongarm][subdir] = "contrib"

projects[uuid][type] = "module"
projects[uuid][version] = "1.0-alpha6"
projects[uuid][subdir] = "contrib"

projects[uuid_features][type] = "module"
projects[uuid_features][version] = "1.0-alpha4"
projects[uuid_features][subdir] = "contrib"

;===============================================================================
; Modules for Search and SEO
;===============================================================================
projects[search_api][type] = "module"
projects[search_api][version] = "1.14"
projects[search_api][subdir] = "contrib"

projects[search_api_autocomplete][type] = "module"
projects[search_api_autocomplete][version] = "1.1"
projects[search_api_autocomplete][subdir] = "contrib"

projects[search_api_db][type] = "module"
projects[search_api_db][version] = "1.4"
projects[search_api_db][subdir] = "contrib"

projects[xmlsitemap][type] = "module"
projects[xmlsitemap][version] = "2.2"
projects[xmlsitemap][subdir] = "contrib"

;===============================================================================
; Modules for Social Integration / Sharing
;===============================================================================

;===============================================================================
; Modules for Permission Management
;===============================================================================
; Dev is needed for DDO-923882
projects[administerusersbyrole][type] = "module"
projects[administerusersbyrole][subdir] = "contrib"
projects[administerusersbyrole][download][type] = git
projects[administerusersbyrole][download][revision] = f867152
projects[administerusersbyrole][download][branch] = 7.x-1.x

projects[masquerade][type] = "module"
projects[masquerade][subdir] = "contrib"
projects[masquerade][version] = "1.0-rc7"

projects[publishcontent][type] = "module"
projects[publishcontent][subdir] = "contrib"
projects[publishcontent][version] = "1.3"

projects[role_delegation][type] = "module"
projects[role_delegation][subdir] = "contrib"
projects[role_delegation][version] = "1.1"

projects[role_export][type] = "module"
projects[role_export][subdir] = "contrib"
projects[role_export][version] = "1.0"

projects[shortcutperrole][type] = "module"
projects[shortcutperrole][subdir] = "contrib"
projects[shortcutperrole][version] = "1.2"

projects[user_settings_access][type] = "module"
projects[user_settings_access][subdir] = "contrib"
projects[user_settings_access][version] = "1.0"
projects[user_settings_access][patch][] = "https://www.drupal.org/files/user_settings_access-apply_coding_standards-2096901-1.patch"
projects[user_settings_access][patch][] = "https://www.drupal.org/files/user_settings_access-restrict_account_fields_and_display-2054645-18.patch"

;===============================================================================
; Modules for Usability
;===============================================================================
projects[diff][type] = "module"
projects[diff][version] = "3.2"
projects[diff][subdir] = "contrib"

; Dev is needed "because Dave Reid" (i.e. never a stable release from him, EVER).
projects[file_entity][type] = "module"
projects[file_entity][subdir] = "contrib"
projects[file_entity][download][type] = git
projects[file_entity][download][revision] = 58b80010a4fd0e6944a4fe5ea8dbe6fed775523e
projects[file_entity][download][branch] = 7.x-2.x

projects[fpa][type] = "module"
projects[fpa][version] = "2.6"
projects[fpa][subdir] = "contrib"

projects[inline_entity_form][type] = "module"
projects[inline_entity_form][version] = "1.5"
projects[inline_entity_form][subdir] = "contrib"

; Dev is needed "because Dave Reid" (i.e. never a stable release from him, EVER).
projects[media][type] = "module"
projects[media][subdir] = "contrib"
projects[media][download][type] = git
projects[media][download][revision] = 63824297d512b1993d4448eb851d2856f2d5df4e
projects[media][download][branch] = 7.x-2.x

projects[module_filter][type] = "module"
projects[module_filter][version] = "2.0"
projects[module_filter][subdir] = "contrib"

projects[views_bulk_operations][type] = "module"
projects[views_bulk_operations][version] = "3.2"
projects[views_bulk_operations][subdir] = "contrib"

projects[markdown][type] = "module"
projects[markdown][version] = "1.2"
projects[markdown][subdir] = "contrib"

projects[portable_path][type] = "module"
projects[portable_path][version] = "1.2"
projects[portable_path][subdir] = "contrib"

projects[typogrify][type] = "module"
projects[typogrify][version] = "1.0-rc9"
projects[typogrify][subdir] = "contrib"

; Themes
; --------

; Libraries
; ---------
; We're pulling down dev because trying to download the library as a project
; puts it in the wrong spot.
libraries[profiler][type] = "library"
libraries[profiler][download][type] = git
libraries[profiler][download][branch] = "7.x-2.x"
libraries[profiler][download][revision] = 2ed21403fedb82df3a82f09d28f1f8b1a9bf2b67
libraries[profiler][patch][] = "https://www.drupal.org/files/issues/profiler-add-ability-to-add-roles-2145695-4.patch"
libraries[profiler][patch][] = "https://www.drupal.org/files/issues/profiler-support_non_custom_blocks-2418273-5.patch"
libraries[profiler][patch][] = "https://www.drupal.org/files/issues/profiler-site_pre_and_post_install_hooks-2418335-6.patch"
libraries[profiler][patch][] = "https://www.drupal.org/files/issues/profiler-support_custom_task_and_form_hooks-2244059-2.patch"
