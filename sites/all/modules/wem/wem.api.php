<?php
/**
 * @file
 * Hooks provided by the WEM module.
 */

/**
 * Allow modules to declare events that they track.
 * Any module that tracks events using wem_event_track() should implement
 *  this hook.
 * @return array
 *  Array keyed by event machine names, where corresponding values are arrays
 *    with the following elements:
 *      'title' (string) - the human-readable name of the event (required).
 *      'description' (string) - a brief explanation of the event type
 *        (when it occurs, how it is tracked, etc.).
 */
function hook_wem_event_info() {
  $events = array(
    'my_event' => array(
      'title' => t('My Special Event'),
      'description' => t('This event occurs whenever something special happens.'),
    ),
  );

  return $events;
}

/**
 * Allows you to react on an event being tracked.
 *
 * @param array $fields
 *   Array of event fields.
 */
function hook_wem_event_track($fields) {

  if ($fields['name'] == "my_event") {
    // Do something custom here.
  }

}

/**
 * Allows you to alter the fields before they are added to the database.
 *
 * @param array $fields
 *   Array of event fields.
 */
function hook_wem_event_track_alter(&$fields) {

  if ($fields['name'] == "some_event" && $fields['value'] == "") {
    // Add a custom value.
    $fields['value'] = request_uri();
  }

}
