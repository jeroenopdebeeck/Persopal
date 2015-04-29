<?php
/**
 * @file
 * Hooks provided by the WEM Segments module.
 */

/**
 * Allow modules to revise the list of segments to which a user is added, after
 *  all core segmentation calculations have been performed. In this way, modules
 *  can introduce any custom logic not currently supported by the base WEM
 *  Segments process, completely overriding the standard segmentation logic
 *  if necessary.
 *
 * @param array $all_segments
 *  Associative array containing all WEM segments that exist in the current
 *  system. This array is keyed by segment names, and the corresponding values
 *  are arrays of segment info.
 * @param array $user_points
 *  Associative array keyed by point category, with integer values containing
 *  the number of points the user has accumulated in each category. (This array
 *  includes all point categories regardless of whether the user has
 *  accumulated points in all categories. For categories in which the user has
 *  no points, the value will be 0).
 * @param array $user_segments
 *  Array containing all segments to which the user belongs, as determined by
 *  the standard WEM Segments logic. This array is keyed by integer segment
 *  IDs, and the corresponding values are segment names (as strings).
 */
function hook_wem_segmentation_alter(&$all_segments, &$user_points, &$user_segments) {
  
  // Relax the criteria for a particular segment based on other factors.
  $segment_name = 'Stressed Users';
  if (!in_array($segment_name, $user_segments) && today_is_monday()) {
    $segment_data = $all_segments[$segment_name];
    $segment_id = $segment_data['sid'];
    $user_segments[$segment_id] = $segment_name;
  }

  // Apply more stringent criteria for a particular segment.
  if (count($user_segments) > 1 && in_array('My Exclusive Segment', $user_segments)) {
    $user_qualifies = my_secret_algorithm($user_points);
    if (!$user_qualifies) {
      $segment_data = $all_segments['My Exclusive Segment'];
      $segment_id = $segment_data['sid'];
      unset($user_segments[$segment_id]);
    }
  }
}
