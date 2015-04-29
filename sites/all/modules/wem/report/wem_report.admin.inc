<?php
/**
 * @file
 * Reporting admin form.
 */

/**
 * Implements hook_FORM_ID().
 */
function wem_report_admin_form($form, &$form_state) {
  $form = array();

  $q = drupal_get_query_parameters();
  if (isset($q['event']) || isset($q['user']) || isset($q['value'])) {
    $_SESSION['wem_table_filter'] = $q;
  }
  else {
    if (!isset($_SESSION['wem_table_filter'])) {
      $_SESSION['wem_table_filter'] = FALSE;
    }
  }
  $filter = $_SESSION['wem_table_filter'];

  // Show a count of all events.
  $events = wem_get_events();
  $form['events'] = array(
    '#type' => 'fieldset',
    '#title' => t('Events'),
    '#collapsible' => TRUE,
  );
  $form['events']['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filters'),
    '#collapsible' => TRUE,
  );
  $form['events']['filter']['events']['event_select'] = array(
    '#type' => 'select',
    '#title' => t('Events'),
    '#description' => t('Filter by event'),
    '#default_value' => $filter['event'],
  );
  $form['events']['filter']['events']['event_select']['#options'][] = 'no filter';
  foreach ($events as $e) {
    $num_events = wem_event_count(array('name' => $e->name));
    $form['events']['filter']['events']['event_select']['#options'][$e->name] = $e->name;
  }

  $form['events']['filter']['users']['user_textbox'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Filter by username'),
    '#default_value' => $filter['user'],
  );

  $form['events']['filter']['value']['value_textbox'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#description' => t('Filter by value'),
    '#default_value' => $filter['value'],
  );

  $form['events']['filter']['buttons'] = array(
    '#type' => 'container',
  );

  $form['events']['filter']['buttons']['filter'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );

  if (!empty($filter)) {
    $form['events']['filter']['buttons']['reset_filter'] = array(
      '#type' => 'submit',
      '#submit' => array('wem_report_reset_filter'),
      '#value' => t('Reset'),
    );
  }

  // We want to show a table of the past events that have occurred.
  $vars = array(
    'header' => array(
      array('data' => 'name', 'field' => 'e.name'),
      array('data' => 'value', 'field' => 'e.value'),
      array('data' => 'data'),
      array('data' => 'username'),
      array('data' => 'time', 'sort' => 'desc', 'field' => 'e.timestamp'),
    ),
  );

  $vars['rows'] = wem_report_get_rows($vars['header']);
  $form['events']['events_table'] = array(
    '#type' => 'fieldset',
    '#title' => 'Recent Events',
  );

  $form['events']['events_table']['table'] = array(
    '#collapsible' => TRUE,
    '#markup' => theme('table', $vars),
  );

  $form['Users'] = array(
    '#type' => 'fieldset',
    '#title' => 'Users',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  return $form;
}

/**
 * Resets the table filter. Called by the "Reset" button.
 */
function wem_report_reset_filter() {
  unset($_SESSION['wem_table_filter']);
}

/**
 * Submit handler for the admin form.
 */
function wem_report_admin_form_submit($form, &$form_state) {
  $v = $form_state['values'];
  $filter['event'] = $v['event_select'];
  $filter['user'] = $v['user_textbox'];
  $filter['value'] = $v['value_textbox'];
  if ($filter) {
    $_SESSION['wem_table_filter'] = $filter;
  }
  else {
    unset($_SESSION['wem_table_filter']);
  }
}

/**
 * Returns a set of rows to be shown in the admin reporting table.
 */
function wem_report_get_rows($header) {
  $filter = $_SESSION['wem_table_filter'];
  $query = db_select('wem', 'e')
    ->fields('e')
    ->extend('PagerDefault')
    ->extend('TableSort');
  if (!empty($filter['event'])) {
    $query->condition('name', $filter['event']);
  }
  if (!empty($filter['user'])) {
    $query->condition('uid', user_load_by_name($filter['user'])->uid);
  }
  if (!empty($filter['value'])) {
    $query->condition('value', $filter['value']);
  }
  if (!empty($filter['user'])) {
    // Show all events for this user.
    $limit = 999;
  }
  else {
    $limit = 25;
  }
  $events = $query->orderByHeader($header)
    ->limit($limit)
    ->execute()
    ->fetchAll();
  $rows = array();
  foreach ($events as $e) {
    $u = user_load($e->uid);
    // For some reason, $u will not be FALSE sometimes when loading anon username.
    if (empty($u) || $u->uid == 0) {
      // Anonymous user, just show the anon uid.
      $name = $e->uid;
    }
    else {
      // Registered user, show their username.
      $name = $u->name;
    }
    $rows[] = array(
      'data' => array(
        $e->name,
        $e->value,
        l(t("view data"), current_path(), array(
          'fragment' => 'view_data',
          'attributes' => array(
            'class' => array('wem-report-data'),
            'data-wem-report-data' => $e->data,
          ),
        )),
        $name,
        format_date($e->timestamp),
      ),
    );
    unset($u);
    unset($name);
  }
  return $rows;
}
