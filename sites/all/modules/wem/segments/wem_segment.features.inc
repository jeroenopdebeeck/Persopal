<?php
/**
 * @file
 * Contains the necessary Features hooks and functionality for segment output.
 */

/**
 * Implements hook_features_export().
 */
function wem_segment_segments_features_export($data, &$export, $module_name) {
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['wem'] = 'wem';
  $export['dependencies']['wem_segment'] = 'wem_segment';
  foreach ($data as $component) {
    $export['features']['wem_segment_segments'][$component] = $component;
  }
  return array();
}

/**
 * Implements hook_features_export().
 */
function wem_segment_rules_features_export($data, &$export, $module_name) {
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['wem'] = 'wem';
  $export['dependencies']['wem_segment'] = 'wem_segment';
  foreach ($data as $component) {
    $export['features']['wem_segment_rules'][$component] = $component;
  }
  return array();
}

/**
 * Implements hook_features_export_options().
 */
function wem_segment_segments_features_export_options() {
  $options = array();
  $wem = new WEMAnalyzer();
  $segments = $wem->loadSegments();
  foreach ((array)$segments as $name => $s) {
    $options[$name] = $name;
  }
  return $options;
}

/**
 * Implements hook_features_export_options().
 */
function wem_segment_rules_features_export_options() {
  $options = array();
  $wem = new WEMAnalyzer();
  $rules = $wem->getRules();
  foreach ((array)$rules as $r) {
    $options[$r->hash] = t("Event {$r->event_name} {$r->event_operator} {$r->event_value} category {$r->event_category} gets {$r->event_points} points");
  }
  return $options;
}

/**
 * Implements hook_features_export_render().
 */
function wem_segment_rules_features_export_render($module, $data) {
  $output = array();
  foreach ((array)$data as $hash) {
    // Load the rule by it's hash.
    $rule = db_select('wem_segment_rules', 'r')
      ->fields('r')
      ->condition('hash', $hash)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
    unset($rule['rid']);
    $output['rules'][$hash] = $rule;
  }
  $array = features_var_export($output);
  $code = "return $array;";
  return array('wem_segment_import_rules' => $code);
}

/**
 * Implements hook_features_export_render().
 */
function wem_segment_segments_features_export_render($module, $data) {
  $wem = new WEMAnalyzer();
  $segments = $wem->loadSegments();
  $output = array();
  foreach ((array)$data as $name) {
    if (isset($segments[$name])) {
      // This segment needs to be exported.
      $s = $segments[$name];
      unset($s['sid']);
      $output['segments'][$name] = $s;
    }
  }
  $array = features_var_export($output);
  $code = "return $array;";
  return array('wem_segment_import_segments' => $code);
}

/**
 * Implements hook_features_revert().
 */
function wem_segment_segments_features_revert($module) {
  wem_segment_segments_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild().
 */
function wem_segment_segments_features_rebuild($module) {
  if ($defaults = features_get_default('wem_segment_segments', $module)) {
    foreach ($defaults['segments'] as $name => $s) {
      wem_segment_save_segment($name, $s['type'], $s['params']);
    }
  }
}

/**
 * Implements hook_features_revert().
 */
function wem_segment_rules_features_revert($module) {
  wem_segment_rules_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild().
 */
function wem_segment_rules_features_rebuild($module) {
  if ($defaults = features_get_default('wem_segment_rules', $module)) {
    foreach ($defaults['rules'] as $hash => $r) {
      wem_segment_save_rule($r['event_name'], $r['event_operator'], $r['event_value'], $r['event_category'], $r['event_points']);
    }
  }
}