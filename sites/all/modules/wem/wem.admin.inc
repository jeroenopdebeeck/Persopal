<?php
/**
 * @file
 * Create the Admin page for the WEM module.
 */

/**
 * Callback for the wem admin list page.
 */
function wem_admin_form($form, &$form_state) {

  $form['general_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('General settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  // Only show this button if segments enabled.
  if (db_table_exists('cache_wem_segment')) {
    $form['general_settings']['rebuild_cache'] = array(
      '#type' => 'button',
      '#value' => t('Rebuild Segment Cache'),
      '#ajax' => array(
        'method' => 'replace',
        'effect' => 'fade',
        'callback' => 'wem_segment_cache_rebuild_ajax',
        'wrapper' => 'edit-rebuild-cache',
      ),
    );
  }

  $form['general_settings']['token'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('wem_token_value'),
    '#title' => t('WEM token'),
    '#description' => t('Value passed to token to validate wem events.'),
  );

  $form['general_settings']['debug_mode'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('wem_debug_mode'),
    '#title' => t('Debug mode'),
    '#description' => t('Will display tracked events on the screen'),
  );

  // content_type selection list.
  $form['tabs'] = array(
    '#type' => 'vertical_tabs',
  );
  $form['tabs']['content_types'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tracked node types'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $types = node_type_get_names();
  $checked = array();
  $options = array();
  foreach ($types as $type => $title) {
    if (variable_get('wem_track_content_type_' . $type, FALSE)) {
      $checked[] = $type;
    }
    $options[$type] = $title;
  }
  $form['tabs']['content_types']['content_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t("Tracked node types"),
    '#description' => t('User activity will be tracked when viewing these node types'),
    '#default_value' => $checked,
    '#options' => $options,
  );

  $form['tabs']['user_roles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tracked user roles'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $options = array();
  $checked = array();
  $roles = user_roles();
  foreach ($roles as $rid => $title) {
    $machine = wem_machine_name($title);
    if (variable_get('wem_track_user_role_' . $machine, FALSE)) {
      $checked[] = $title;
    }
    $options[$title] = $title;
  }
  $form['tabs']['user_roles']['user_roles'] = array(
    '#type' => 'checkboxes',
    '#title' => t("Tracked user roles"),
    '#description' => t('These user roles will be tracked by the module'),
    '#default_value' => $checked,
    '#options' => $options,
  );

  $form['tabs']['dont_track'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page tracking'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['tabs']['dont_track']['dont_track'] = array(
    '#type' => 'textarea',
    '#title' => t('Do not track these pages'),
    '#description' => t('Separate entries by new lines.<br />You can use regex here(e . g . node / 123, /^node /) See PHP ') . l(t('preg_match'), 'http://php.net/manual/en/function.preg-match.php', array('absolute' => TRUE)),
    '#default_value' => implode(PHP_EOL, wem_get_pages_dont_track()),
  );

  $form['tabs']['dont_track']['usernames'] = array(
    '#type' => 'textarea',
    '#title' => t('Usernames not to track'),
    '#description' => t('Separate entries by new lines.'),
    '#default_value' => implode(PHP_EOL, variable_get('wem_users_dont_track')),
  );

  $form['tabs']['analytics'] = array(
    '#type' => 'fieldset',
    '#title' => t('Analytics'),
    '#description' => t('Send event tracking to these other tracking programs'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // Check if module enabled.
  $ga_disabled = TRUE;
  $ga_description = t("(module not found) Download from !link.", array('!link' => l(t('here'), 'http://drupal.org/project/google_analytics')));
  if (module_exists('googleanalytics')) {
    $ga_disabled = FALSE;
    $ga_description = t('Manage <a href="!ga_url">Google Analytics</a>', array('!ga_url' => url('admin/config/system/googleanalytics')));
  }
  $form['tabs']['analytics']['ga'] = array(
    '#type' => 'fieldset',
    '#title' => t('Google Analytics'),
    '#description' => $ga_description,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#disabled' => $ga_disabled,
  );
  $form['tabs']['analytics']['ga']['google_analytics'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Google Analytics WEM integration'),
    '#default_value' => variable_get('wem_analytics_ga'),
  );

  $form['#submit'][] = 'wem_admin_form_submit';

  return system_settings_form($form);
}

/**
 * Implements hook_FORM_ID_validate().
 */
function wem_admin_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (strpos($values['dont_track'], ',') !== FALSE) {
    form_set_error('dont_track', t('Separate by new lines, not commas.'));
  }
  if (strpos($values['usernames'], ',') !== FALSE) {
    form_set_error('usernames', t('Separate by new lines, not commas.'));
  }
  if (!trim(drupal_strlen($values['token']))) {
    form_set_error('token', t('Token value required'));
  }

  // Make sure usernames are valid.
  $usernames = explode(PHP_EOL, $values['usernames']);
  foreach ($usernames as $name) {
    $u = user_load_by_name($name);
    if (!$u) {
      form_set_error('usernames', t("'@name' is not a valid username", array('@name' => $name)));
    }
  }
}

/**
 * Implements hook_FORM_ID_submit().
 */
function wem_admin_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  foreach ($values['content_types'] as $type => $val) {
    variable_set('wem_track_content_type_' . $type, $val);
  }

  foreach ($values['user_roles'] as $key => $val) {
    variable_set('wem_track_user_role_' . wem_machine_name($key), $val);
  }
  $dont_track = explode(PHP_EOL, $values['dont_track']);

  // Google Analytics.
  variable_set('wem_analytics_ga', $values['google_analytics']);
  $google_universal_analytics = FALSE;
  if ($values['google_analytics']) {
    // If the 2.x version of the GA module is enabled, we'll need to know
    //  to use GUA syntax.
    $ga_module_version = wem_utils_get_project_version('googleanalytics', 'module', 'project_major');
    if ($ga_module_version >= 2) {
      $google_universal_analytics = TRUE;
    }
  }
  variable_set('wem_analytics_ga_universal', $google_universal_analytics);
  $form['tabs']['analytics']['google_analytics'] = array();

  /* variable_set('wem_analytics_sc', $values['site_catalyst']); */
  wem_save_pages_dont_track($dont_track);
  variable_set('wem_debug_mode', $values['debug_mode']);
  variable_set('wem_token_value', $values['token']);
  variable_set('wem_users_dont_track', explode(PHP_EOL, $values['usernames']));
}

/**
 * Utility function to retrieve version info for a Drupal project.
 * @param string $name - The Drupal project name whose version you wish to
 *  retrieve.
 * @param string $type - The type of project; either "module" or "theme."
 * @param string $requested_component - The portion of the version descriptor
 *  you wish to retrieve. The following options are supported
 *  (using a project whose version is "7.58-2.x-dev" for example):
 *    - 'full': the entire version string (e.g. '7.58-2.x-dev')
 *    - 'core': the Drupal core version (e.g. 7.58)
 *    - 'core_major': the major Drupal core version (e.g. 7)
 *    - 'core_minor': the minor Drupal core version (e.g. 58)
 *    - 'project_full': the full project version (e.g. '2.x-dev')
 *    - 'project_basic': the basic project version without any modifier (e.g. '2.x')
 *    - 'project_major': the major project version (e.g. 2)
 *    - 'project_minor': the minor project version (e.g. 'x')
 *    - 'modifier': the string suffix describing the version type (e.g. 'dev')
 *    - 'all': an array containing all of the above.
 * @param bool $use_numeric_values - Whether to convert numeric strings
 *  to actual numbers. If false, those components will be returned as strings.
 * @return string|int|float|array|bool - FALSE if the requested information
 *  cannot be found. Otherwise the return value is dictated by
 *  $requested_component and $use_numeric_values as described above.
 * @author: Nathanael Dewhurst
 */
function wem_utils_get_project_version($name, $type = 'module', $requested_component = 'project_basic', $use_numeric_values = TRUE) {
  if ($type != 'module' && $type != 'theme') {
    return FALSE;
  }
  $project_info = system_get_info($type, $name);
  if (isset($project_info['version'])) {
    $matches = array();
    // Parse the various components of a version identifier.
    // @todo: handle other formats (e.g. 7.99.3, etc.)?
    if (preg_match('/^((\d+)\.(x|\d+))(-((\d+)\.(x|\d+)))?(-(.+))?$/', $project_info['version'], $matches)) {
      $version_data = array(
        'full' => $project_info['version'],
        'core' => $matches[1],
        'core_major' => $matches[2],
        'core_minor' => $matches[3],
      );
      if (!empty($matches[5])) {
        $version_data['project_basic'] = $version_data['project_full'] = $matches[5];
      }
      if (!empty($matches[6])) {
        $version_data['project_major'] = $matches[6];
      }
      if (!empty($matches[7])) {
        $version_data['project_minor'] = $matches[7];
      }
      if (!empty($matches[9])) {
        $version_data['modifier'] = $matches[9];
        if (isset($version_data['project_full'])) {
          $version_data['project_full'] .= '-' . $version_data['modifier'];
        }
      }
    }
  }
  if (isset($version_data)) {
    // Convert numeric strings to numbers unless instructed otherwise.
    if ($use_numeric_values) {
      foreach ($version_data as $key => $value) {
        if (is_numeric($value)) {
          $version_data[$key] = strpos($value, '.') >= 0 ? (float) $value : (int) $value;
        }
      }
    }
    if ($requested_component == 'all') {
      return $version_data;
    }
    elseif (isset($version_data[$requested_component])) {
      return $version_data[$requested_component];
    }
  }
  return FALSE;
}
