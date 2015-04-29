<?php
/**
 * @file
 * Main segment class that determines what segments a user is in.
 */


/**
 * @class WEMAnalyzer
 *
 * Class that can calculate user segments, points, and cache the results.
 */
class WEMAnalyzer {

  /**
   * Rebuilds the segment cache for all users.
   */
  public function rebuildCache() {
    $users = db_select('wem', 'w')
      ->fields('w', array('uid'))
      ->groupBy('uid')
      ->execute()
      ->fetchAll();
    foreach ($users as $u) {
      $this->cache($u->uid, $this->getSegments($u->uid));
    }
  }

  /**
   * Get segments for a given user.
   *
   * @param int $uid
   *   User ID
   *
   * @return array
   *   Segments that a user has.
   */
  public function getSegments($uid) {
    $rules = $this->getRules();
    $points = $this->getPoints($uid, $rules);
    return $this->calculateSegments($points);
  }

  /**
   * Calculates the segments based on the points provided.
   */
  protected function calculateSegments($points) {
    $segments = $this->loadSegments();
    // Check if the segment matches the points thresh hold.
    $return = array();
    foreach ($segments as $name => $s) {
      if ($this->pointsYieldSegment($s, $points)) {
        $return[$s['sid']] = $name;
      }
    }
    // Allow other modules to alter any of the segmentation decisions.
    drupal_alter('wem_segmentation', $segments, $points, $return);
    return $return;
  }

  /**
   * Determines if the points earned yield a segment.
   */
  protected function pointsYieldSegment($segment, $points) {
    $params = $segment['params'];
    $eval = "if(";
    // Loop through and build a string that matches the rules.
    foreach ($params as $p) {
      $eval .= "\$points['{$p['category']}'] {$p['operator']} {$p['points']} && ";
    }
    // Remove the trailing &&.
    $eval = substr($eval, 0, strlen($eval) - 4);
    $eval .= ") return TRUE;";
    // @ignore This should not pose a security risk.
    return eval($eval);
  }

  /**
   * Loads segments from the database.
   */
  public function loadSegments() {
    $query = db_select('wem_segment', 's');
    $query->join('wem_segment_params', 'p', 's.sid = p.sid');
    $segments = $query
      ->fields('s')
      ->fields('p', array('type', 'params'))
      ->orderBy('s.created', 'DESC')
      ->execute()
      ->fetchAll();
    $return = array();
    foreach ($segments as $s) {
      $return[$s->name] = array(
        'type' => $s->type,
        'sid' => $s->sid,
        'params' => unserialize($s->params),
      );
    }
    return $return;
  }

  /**
   * Cache segments for a given user.
   */
  protected function cache($uid, $segments) {
    cache_set($uid, $segments, 'cache_wem_segment');
  }

  /**
   * Calculates points based on the rules that are passed.
   */
  protected function getPoints($uid, $rules) {
    if (empty($uid)) {
      global $user;
      $uid = $user->uid;
    }
    // Get events for this user.
    $events = wem_get_events(array('uid' => $uid));
    $points = array();
    // Foreach rule check if any points can be given to that event.
    foreach ($rules as $r) {
      foreach ($events as $e) {
        if (!isset($points[$r->event_category])) {
          $points[$r->event_category] = 0;
        }
        $points[$r->event_category] += $this->calculatePoints($e, $r);
      }
    }
    return $points;
  }

  /**
   * Calculate how many points should be issued for this event.
   */
  protected function calculatePoints($event, $rule) {
    // Given this rule, figure out if the event applies.
    if ($rule->event_name == $event->name) {
      switch ($rule->event_operator) {
        case "equal to":
          if ($event->value == $rule->event_value) {
            return $rule->event_points;
          }
          break;

        case "not equal to":
          if ($event->value != $rule->event_value) {
            return $rule->event_points;
          }
          break;

        case "less than":
          if ($event->value < $rule->event_value) {
            return $rule->event_points;
          }
          break;

        case "less than or equal to":
          if ($event->value <= $rule->event_value) {
            return $rule->event_points;
          }
          break;

        case "greater than":
          if ($event->value > $rule->event_value) {
            return $rule->event_points;
          }
          break;

        case "greater than or equal to":
          if ($event->value >= $rule->event_value) {
            return $rule->event_points;
          }
          break;

        case "containing":
          if (strpos($event->value, $rule->event_value) !== FALSE) {
            return $rule->event_points;
          }
          break;

        case "matching regex":
          if (preg_match($rule->event_value, $event->value) === 1) {
            return $rule->event_points;
          }
          break;

        default:
          // No value to compare to.
          return $rule->event_points;
      }
    }
    return 0;
  }

  /**
   * Returns an array of rules.
   */
  public function getRules() {
    return db_select('wem_segment_rules', 'r')
      ->fields('r')
      ->execute()
      ->fetchAll();
  }

}
