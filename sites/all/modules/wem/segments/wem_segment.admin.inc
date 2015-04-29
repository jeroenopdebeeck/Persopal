<?php
/**
 * @file
 * Create forms for the segment and rules admin form.
 */

/**
 * Admin form for segment table.
 */
function wem_segment_admin_rule_form($form, &$form_state) {
  // Check actions to see if we need to do anything.
  $q = drupal_get_query_parameters();
  if (isset($q['action'])) {
    switch ($q['action']) {
      case "delete":
        if (isset($q['rid'])) {
          wem_segment_rule_delete($q['rid']);
        }
        break;

      case "clone":
        $rule = wem_segment_load_rule($q['rid']);
        wem_segment_save_rule($rule['event_name'], $rule['event_operator'], $rule['event_value'], $rule['event_category'], $rule['event_points']);
        drupal_set_message(t('Rule cloned.'));
        break;

      case "edit":
        $default = wem_segment_load_rule($q['rid']);
        break;

    }
  }

  $fieldset_title = t('Add segment rule');
  if (!empty($default)) {
    $fieldset_title = t('Edit segment rule');
  }
  $form['save_rule'] = array(
    '#type' => 'fieldset',
    '#title' => check_plain($fieldset_title),
    '#description' => t('These rules will determine how many point users get based on their actions. View !link.', array('!link' => l(t('help and examples'), 'admin/help/wem_segment/rules'))),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['save_rule']['event_edit_rid'] = array(
    '#type' => 'hidden',
  );
  $form['save_rule']['event_name'] = array(
    '#prefix' => t('If event'),
    '#type' => 'select',
    '#options' => wem_segment_event_names(),
  );
  $form['save_rule']['event_operator'] = array(
    '#type' => 'select',
    '#prefix' => t('has value'),
    '#options' => array(
      '' => '-- operator --',
      'equal to' => '==',
      'not equal to' => '!==',
      'less than' => '<',
      'less than or equal to' => '<=',
      'greater than' => '>',
      'greater than or equal to' => '>=',
      'containing' => 'containing',
      'matching regex' => 'matching regex',
    ),
  );
  $form['save_rule']['event_value'] = array(
    '#type' => 'textfield',
  );
  $categories = wem_segment_get_categories();
  $categories['create_new_category'] = t('create new...');
  $form['save_rule']['event_category'] = array(
    '#prefix' => t('then category'),
    '#type' => 'select',
    '#empty_option' => t('-- category --'),
    '#options' => $categories,
  );
  $form['save_rule']['event_category_custom'] = array(
    '#type' => 'textfield',
  );
  $form['save_rule']['event_points'] = array(
    '#prefix' => t('gets'),
    '#type' => 'textfield',
    '#suffix' => t('points'),
  );
  $form['save_rule']['event_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save rule'),
  );
  if (!empty($default)) {
    // Set default values for form items.
    $form['save_rule']['event_edit_rid']['#default_value'] = $default['rid'];
    $form['save_rule']['event_name']['#default_value'] = $default['event_name'];
    $form['save_rule']['event_operator']['#default_value'] = $default['event_operator'];
    $form['save_rule']['event_value']['#default_value'] = $default['event_value'];
    $form['save_rule']['event_category']['#default_value'] = $default['event_category'];
    $form['save_rule']['event_points']['#default_value'] = $default['event_points'];
  }

  $form['rules'] = array(
    '#type' => 'fieldset',
    '#title' => t('Segment rules'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $vars = array(
    'header' => array(
      array('data' => 'id', 'width' => '50'),
      array('data' => 'Event'),
      array('data' => 'Operator', 'width' => '100'),
      array('data' => 'Value'),
      array('data' => 'Category'),
      array('data' => 'Points', 'width' => '100'),
      array('data' => 'Created', 'width' => '200'),
      array('data' => 'Actions', 'width' => '150'),
    ),
  );
  if (isset($default['rid'])) {
    $rid = $default['rid'];
  }
  else {
    $rid = FALSE;
  }
  $vars['rows'] = wem_segment_get_rows_rules($rid);
  if (count($vars['rows']) < 1) {
    $form['rules']['table'] = array(
      '#markup' => t("You haven't created any segment rules yet."),
    );
  }
  else {
    $form['rules']['table'] = array(
      '#markup' => theme('table', $vars),
    );
  }
  return $form;
}

/**
 * Admin form to add user segments.
 */
function wem_segment_admin_add_segment_form($form, &$form_state) {

  $form['create_segment'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add segment'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['create_segment']['segment_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
  );

  $form['create_segment']['segment_categories'] = array(
    '#type' => 'container',
    '#title' => t('Categories'),
  );

  // Set segment type to 'static'. We may eventually have more segment types.
  $form['create_segment']['segment_type'] = array(
    '#type' => 'hidden',
    '#default_value' => 'static',
  );

  $form['create_segment']['segment_categories']['segment_type_static'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Matches these rules'),
    '#description' => t('People will be put into this segment only if they match ALL of the rules you set.'),
  );

  $static_rule = array(
    '#type' => 'container',
  );

  $static_rule['static_category'] = array(
    '#prefix' => t('Category'),
    '#type' => 'select',
    '#default_value' => NULL,
    '#empty_option' => t('-- category --'),
    '#options' => wem_segment_get_categories(),
  );

  $static_rule['static_operator'] = array(
    '#prefix' => t('has total points'),
    '#type' => 'select',
    '#default_value' => NULL,
    '#empty_option' => t('-- operator --'),
    '#options' => array(
      '>' => '>',
      '>=' => '>=',
      '<' => '<',
      '<=' => '<=',
      '==' => '==',
      '!=' => '!=',
    ),
  );

  $static_rule['static_points'] = array(
    '#type' => 'textfield',
    '#default_value' => NULL,
    '#attributes' => array(
      'size' => 10,
    ),
  );

  // Created 6 static rules an admin can customize.
  for ($i = 0; $i <= 6; $i++) {
    $form['create_segment']['segment_categories']['segment_type_static']['static_rule'][$i] = $static_rule;
  }

  $form['create_segment']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save segment'),
  );

  return $form;
}

/**
 * View segments form.
 */
function wem_segment_admin_view_segments_form($form, &$form_state) {

  $q = drupal_get_query_parameters();
  if (isset($q['action'])) {
    switch ($q['action']) {
      case "delete":
        if (isset($q['sid'])) {
          wem_segment_segment_delete($q['sid']);
        }
        break;
    }
  }

  $form['segments'] = array(
    '#type' => 'fieldset',
    '#title' => t('Segments'),
  );

  $vars = array(
    'header' => array(
      array('data' => 'Name'),
      array('data' => 'Params'),
      array('data' => 'Created'),
      array('data' => 'Actions'),
    ),
  );
  $vars['rows'] = wem_segment_get_rows_segments();
  if (count($vars['rows']) < 1) {
    $form['segments']['table'] = array(
      '#markup' => t("You haven't created any segments yet."),
    );
  }
  else {
    $form['segments']['table'] = array(
      '#markup' => theme('table', $vars),
    );
  }

  return $form;
}

/**
 * Builds the rows for the rules table.
 */
function wem_segment_get_rows_rules($rid = NULL) {
  $rows = array();
  $rules = db_select('wem_segment_rules', 'e')
    ->fields('e')
    ->orderBy('created', 'DESC')
    ->execute()
    ->fetchAll();
  foreach ($rules as $r) {
    $r->created = date('m/d/y - h:i', $r->created);
    if ($r->rid == $rid) {
      $rows[$r->rid]['class'] = array('active');
    }
    $rule = (array)$r;
    unset($rule['hash']);
    $rows[$r->rid]['data'] = $rule;
    $rows[$r->rid]['data'][] = wem_segment_rules_table_actions($r->rid);
  }
  return $rows;
}

/**
 * Builds the rows for the segment table.
 */
function wem_segment_get_rows_segments() {
  $rows = array();
  $query = db_select('wem_segment', 's');
  $query->join('wem_segment_params', 'p', 's.sid = p.sid');
  $segments = $query
    ->fields('s')
    ->fields('p', array('type', 'params'))
    ->orderBy('s.created', 'DESC')
    ->execute()
    ->fetchAll();
  foreach ($segments as $s) {
    $s->created = date('m/d/y - h:i', $s->created);
    $output = "";
    if ($s->type == 'static') {
      foreach (unserialize($s->params) as $param) {
        $output .= "<b>{$param['category']}</b> score {$param['operator']} {$param['points']} points AND<br />";
      }
      $output = drupal_substr($output, 0, drupal_strlen($output) - 10);
    }
    elseif ($s->type == 'comparison') {
      $params = unserialize($s->params);
      $categories = $params['categories'];
      $output = "Comparing " . implode(', ', $categories);
    }
    $rows[$s->sid]['data'] = array(
      'name' => $s->name,
      'params' => $output,
      'created' => $s->created,
      'actions' => wem_segment_segments_table_actions($s->sid),
    );
  }
  return $rows;
}

/**
 * Builds the action links in the segments table.
 */
function wem_segment_segments_table_actions($sid = 0) {
  return l(t('delete'), current_path(), array(
    'query' => array(
      'action' => 'delete',
      'sid' => $sid,
    ),
  ));
  // @todo: allow existing segments to be edited! (NRD 2014-03-12).
}

/**
 * Builds the action links in the rules table.
 */
function wem_segment_rules_table_actions($rid = 0) {
  return l(t('edit'), current_path(), array(
    'query' => array(
      'action' => 'edit',
      'rid' => $rid,
    ),
  )) . ' | ' .
  l(t('clone'), current_path(), array(
    'query' => array(
      'action' => 'clone',
      'rid' => $rid,
    ),
  )) . ' | ' .
  l(t('delete'), current_path(), array(
    'query' => array(
      'action' => 'delete',
      'rid' => $rid,
    ),
  ));
}

/**
 * Implements hook_FORM_ID_validate().
 */
function wem_segment_admin_add_segment_form_validate($form, &$form_state) {
  $v = $form_state['values'];

  if (empty($v['segment_name'])) {
    form_set_error('segment_name', t('Please enter a segment name.'));
  }

  $static_rules = $v['segment_type_static']['static_rule'];
  $submitted = array();
  foreach ($static_rules as $rule) {
    foreach ($rule as $val) {
      if ($val !== '0' && empty($val)) {
        // Do not include this in the submitted array.
        continue 2;
      }
    }
    // If the loop gets here, they have filled out all three sections
    // to make a complete rule.
    $submitted[] = $rule;
  }
  if (count($submitted) < 1) {
    form_set_error('', t('A valid rule MUST contain a category, operator, and points value.'));
  }

}

/**
 * Implements hook_FORM_ID_submit().
 */
function wem_segment_admin_add_segment_form_submit($form, &$form_state) {
  $v = $form_state['values'];

  $params = array();

  // Build array for static params.
  $static_rules = $v['segment_type_static']['static_rule'];
  foreach ($static_rules as $rule) {
    foreach ($rule as $val) {
      if ($val !== '0' && empty($val)) {
        // Do not include this in the submitted array.
        continue 2;
      }
    }
    // If the loop gets here, they have filled out all three sections
    // to make a complete rule.
    $params[] = array(
      'category' => $rule['static_category'],
      'operator' => $rule['static_operator'],
      'points' => $rule['static_points'],
    );
  }

  wem_segment_save_segment($v['segment_name'], $v['segment_type'], $params);
  drupal_set_message(t("Added segment ':segment' successfully.", array(":segment" => $v['segment_name'])), "status");
}

/**
 * Implements hook_FORM_ID_validate().
 */
function wem_segment_admin_rule_form_validate($form, &$form_state) {
  $v = $form_state['values'];

  if (!$v['event_name']) {
    form_set_error('event_name', t('Please specify an event name.'));
  }
  if (!$v['event_category'] && !$v['event_category_custom']) {
    form_set_error('event_category', t('Please select a category.'));
  }
  if (!$v['event_operator'] && $v['event_value']) {
    form_set_error('event_value', t('Please specify an operator.'));
  }
  if (!$v['event_points']) {
    form_set_error('event_points', t('Please specify a point value.'));
  }
  if ($v['event_points'] < 1) {
    form_set_error('event_points', t('Please enter a point value greater than 0.'));
  }
}

/**
 * Implements hook_FORM_ID_submit().
 */
function wem_segment_admin_rule_form_submit($form, &$form_state) {
  $v = $form_state['values'];
  if (!empty($v['event_category_custom'])) {
    $category = $v['event_category_custom'];
  }
  else {
    $category = $v['event_category'];
  }
  if ($v['event_edit_rid']) {
    // User is editing, need to alter the rule.
    $result = wem_segment_edit_rule($v['event_edit_rid'], $v['event_name'], $v['event_operator'], $v['event_value'], $category, $v['event_points']);
    if ($result) {
      drupal_set_message(t("Rule edited successfully."));
    }
    else {
      drupal_set_message(t("Could not edit the rule."), 'error');
    }
  }
  else {
    // User is saving, need to create the rule.
    $result = wem_segment_save_rule($v['event_name'], $v['event_operator'], $v['event_value'], $category, $v['event_points']);
    if ($result) {
      drupal_set_message(t("Rule created successfully."));
    }
    else {
      drupal_set_message(t("Could not create the rule."), 'error');
    }
  }
}

/**
 * Returns form array for the reports page.
 */
function wem_segment_alter_report_form() {
  $form['segments'] = array(
    '#type' => 'fieldset',
    '#title' => 'Segments',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  return $form;
}

/**
 * Adds segment data to the report form.
 */
function wem_segment_form_alter(&$form, &$form_state, $form_id) {
  if ($form_id == 'wem_report_admin_form') {
    $form[] = wem_segment_alter_report_form();
  }
  if (!empty($form['#node_edit_form']) && user_access('wem node segments')) {
    $form['segments'] = array(
      '#type' => 'fieldset',
      '#group' => 'additional_settings',
      '#title' => t('Segments'),
      '#tree' => TRUE,
    );
    // Load node segments if they exist set default values.
    $node_segments = wem_segment_get_node_segments($form['nid']['#value']);
    $form['segments']['enable'] = array(
      '#type' => 'checkbox',
      '#title' => 'Enable WEM segmentation',
    );
    if ($node_segments) {
      $form['segments']['enable']['#default_value'] = 1;
    }
    $form['segments']['content'] = array(
      '#type' => 'container',
      '#tree' => TRUE,
    );
    $form['segments']['content']['header'] = array(
      '#markup' => '<p>' . t('Control how this content is displayed to different user segments.') . '</p>',
    );

    $wem = new WEMAnalyzer();
    $segments = $wem->loadSegments();
    foreach ($segments as $name => $s) {
      $form['segments']['content'][$name] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($name),
        '#collapsed' => TRUE,
        '#collapsible' => TRUE,
      );
      $form['segments']['content'][$name]['has_segment'] = array(
        '#type' => 'select',
        '#title' => "If a user has this segment",
        '#options' => array(
          '' => 'No Preference',
          'show' => 'Show this content',
          'hide' => 'Hide this content',
        ),
      );
      if (!empty($node_segments[$name]['has_segment'])) {
        $form['segments']['content'][$name]['has_segment']['#default_value'] = $node_segments[$name]['has_segment'];
      }
      $form['segments']['content'][$name]['no_segment'] = array(
        '#type' => 'select',
        '#title' => "If a user does not have this segment",
        '#options' => array(
          '' => 'No Preference',
          'show' => 'Show this content',
          'hide' => 'Hide this content',
        ),
      );
      if (!empty($node_segments[$name]['no_segment'])) {
        $form['segments']['content'][$name]['no_segment']['#default_value'] = $node_segments[$name]['no_segment'];
      }
    }
  }
  // Alter the main WEM admin form to add segment-related settings.
  elseif ($form_id == 'wem_admin_form') {
    // Provide support for sending segment data to Google Analytics using
    //  custom dimensions.
    $form['tabs']['analytics']['ga']['dimensions'] = array(
      '#type' => 'fieldset',
      '#title' => t('Custom Dimensions'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['tabs']['analytics']['ga']['dimensions']['wem_analytics_ga_dimensions_segment'] = array(
      '#type' => 'textfield',
      '#title' => t('User segments dimension'),
      '#description' => t('The unique ID number of the custom dimension you use to capture a visitor\'s WEM segment(s). E.g. if you created a dimension called "WEM Segments" and it is stored as "dimension1," enter the number "1" in this field.'),
      '#maxlength' => 3,
      '#default_value' => variable_get('wem_analytics_ga_dimensions_segment'),
    );
    $form['#validate'][] = 'wem_segment_wem_admin_form_validate';
  }
}

/**
 * Custom validation handler for wem admin form.
 */
function wem_segment_wem_admin_form_validate($form, &$form_state) {
  if (isset($form_state['values']['wem_analytics_ga_dimensions_segment'])) {
    if (!is_numeric($form_state['values']['wem_analytics_ga_dimensions_segment'])) {
      form_set_error('wem_analytics_ga_dimensions_segment', t('Please enter a simple number in this field'));
    }
  }
}

/**
 * Returns an array of unique event names.
 */
function wem_segment_event_names() {
  $events = module_invoke_all('wem_event_info');
  $select = array();
  $select[] = '-- event --';
  foreach ($events as $machine_name => $event_data) {
    $select[$machine_name] = $event_data['title'];
  }
  return $select;
}

/**
 * Implements hook_help().
 */
function wem_segment_help($path, $arg) {
  $help_text = arg(3);
  switch ($help_text) {
    case "rules":
      return t('Help for rules.');

    case "segments":
      return t('Help for segments.');

  }
}

/**
 * Creates a rule in the database.
 */
function wem_segment_save_rule($name, $op, $value, $cat, $points) {
  // Create a hash based on column data.
  $hash = sha1(implode('_', func_get_args()));
  return db_insert('wem_segment_rules')
    ->fields(array(
      'hash' => $hash,
      'event_name' => $name,
      'event_operator' => $op,
      'event_value' => $value,
      'event_category' => $cat,
      'event_points' => $points,
      'created' => REQUEST_TIME,
    ))
    ->execute();
}

/**
 * Edits a rule in the database.
 */
function wem_segment_edit_rule($rid, $name, $op, $value, $cat, $points) {
  // Create a hash based on column data.
  $hash = sha1(implode('_', func_get_args()));
  return db_update('wem_segment_rules')
    ->fields(array(
      'hash' => $hash,
      'event_name' => $name,
      'event_operator' => $op,
      'event_value' => $value,
      'event_category' => $cat,
      'event_points' => $points,
      'created' => REQUEST_TIME,
    ))
    ->condition('rid', $rid)
    ->execute();
}

/**
 * Returns the categories that have been created.
 */
function wem_segment_get_categories() {
  $rules = db_select('wem_segment_rules', 'r')
    ->fields('r')
    ->groupBy('event_category')
    ->execute()
    ->fetchAll();
  $categories = array();
  foreach ($rules as $r) {
    if (!empty($r->event_category)) {
      $categories[$r->event_category] = $r->event_category;
    }
  }
  return $categories;
}

/**
 * Delete a segment based on it's sid.
 */
function wem_segment_segment_delete($sid = 0) {
  if (!$sid) {
    drupal_set_message(t('No sid specified.'), 'error');
    return FALSE;
  }
  $result = db_delete('wem_segment')
    ->condition('sid', $sid)
    ->execute();
  if ($result) {
    db_delete('wem_segment_params')
      ->condition('sid', $sid)
      ->execute();
    drupal_set_message(t('Deleted segment ":sid".', array(":sid" => $sid)));
  }
  else {
    drupal_set_message(t('Could not delete segment ":sid".', array(":sid" => $sid)), 'error');
  }
}

/**
 * Delete a segments rule based on it's rid.
 */
function wem_segment_rule_delete($rid = 0) {
  if (!$rid) {
    drupal_set_message(t('No rid specified.'), 'error');
    return FALSE;
  }
  $result = db_delete('wem_segment_rules')
    ->condition('rid', $rid)
    ->execute();
  if ($result) {
    drupal_set_message(t('Deleted segment rule ":rid".', array(":rid" => $rid)));
  }
  else {
    drupal_set_message(t('Could not delete segment rule ":rid".', array(":rid" => $rid)), 'error');
  }
  return $result;
}

/**
 * Loads a rule based on it's rid.
 */
function wem_segment_load_rule($rid) {
  return db_select('wem_segment_rules', 'r')
    ->fields('r')
    ->condition('rid', $rid)
    ->execute()
    ->fetchAssoc();
}

/**
 * Saves a user segment to the database.
 */
function wem_segment_save_segment($name, $type = "", $params = array()) {
  // Create the segment.
  $sid = db_insert('wem_segment')
    ->fields(array('name' => $name, 'created' => REQUEST_TIME))
    ->execute();
  // Add the params.
  if (!empty($sid) && !empty($type) && !empty($params)) {
    return db_insert('wem_segment_params')
      ->fields(array(
        'sid' => $sid,
        'type' => $type,
        'params' => serialize($params),
      ))
      ->execute();
  }
}
