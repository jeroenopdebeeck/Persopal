/**
 * Drupal.wem object.
 *
 * This file takes care of the front-end page view tracking.
 * You can create custom events easily, and send them to the server using the
 * Drupal.wem.track() function.
 *
 *   Drupal.wem.track({name:"user_click", value:"blue_button"});
 *
 * You can make any HTML element have tracking on it for different events
 * by using the wem_data_attribute() function. This file will automatically
 * add those events to the HTML element.
 */
(function ($) {

  Drupal.wem = {
    token: "",
    uid: ""
  }

  /**
   * Send event data to the server.
   * Will send data to analytic programs if enabled.
   *
   * @param event
   *   Event object should consist of an event name, value, and data.
   */
  Drupal.wem.track = function (event) {
    Drupal.wem.queue.add(event);
    // Send the event to Google Analytics. We won't track page_view though.
    if (Drupal.wem.ga() && (event.name != 'page_view')) {
      if (Drupal.wem.ga_universal()) {
        ga('send', 'event', event.name, event.value, event.data);
      }
      else {
        _gaq.push(['_trackEvent', event.name, event.value, event.data]);
      }
    }
    return true;
  }

  /**
   * Returns the wem token value.
   */
  Drupal.wem.getToken = function () {
    if (token = $.Storage.get('wem_token')) {
      Drupal.wem.token = token;
      return;
    }
    $.ajax({
      url: Drupal.settings.wem.paths.api + '/get_token',
      success: function (token) {
        $.Storage.set('wem_token', token);
        Drupal.wem.token = token;
      }
    });
  }

  /**
   * Returns the wem uid.
   */
  Drupal.wem.getUID = function () {
    // Check if the cookie is set.
    if (uid = $.Storage.get('wem_uid')) {
      Drupal.wem.uid = uid;
      return;
    }
    $.ajax({
      url: Drupal.settings.wem.paths.api + '/get_uid',
      success: function (uid) {
        $.Storage.set('wem_uid', uid);
        Drupal.wem.uid = uid;
      }
    });
  }

  /**
   * Returns true if Google analytics is enabled
   */
  Drupal.wem.ga = function () {
    return Drupal.settings.wem.analytics.ga.enabled;
  }

  /**
   * Returns true if Google Universal Analytics is enabled
   */
  Drupal.wem.ga_universal = function () {
    return Drupal.settings.wem.analytics.ga.universal;
  }

  /**
   * Returns true if Site Catalyst is enabled
   */
  Drupal.wem.sc = function () {
    return Drupal.settings.wem.analytics_sc;
  }

  /**
   * Binds events to the element.
   */
  Drupal.wem.bindEvent = function (el, event, jsevent) {
    // Hover is a dirty event, replace with mouseover.
    if (jsevent == 'hover') {
      jsevent = 'mouseover';
    }
    if (jsevent == 'ready') {
      jsevent = 'onload';
    }
    // Append the js_event to the event name.
    event.name = event.name + "_" + jsevent;
    $(el).bind(jsevent, function (e) {
      Drupal.wem.track(event);
      if (event.bypass) {
        Drupal.wem.queue.run();
      }
    });
  }

  /**
   * Looks for elements with data-wem attributes and applies
   * those events to the DOM.
   */
  Drupal.wem.attachDataEvents = function () {
    var elements = $('[data-wem]');
    // Foreach element we need to get information.
    elements.each(function (i, el) {
      // Convert the string to JSON, replace slashes with quotes.
      var data = JSON.parse($(el).data('wem').replace(/\\/g, '"'));
      for (i = 0; i < data.js_events.length; i++) {
        Drupal.wem.bindEvent(el, {name: data.name, value: data.value, data: data.data}, data.js_events[i]);
      }
    });
  }

  /**
   * Queue Object
   * The queue is a stored array of events that need to be sent to the server.
   * Events are stored here to minimize load on the server.
   * The queue should be 'run' on a regular interval.
   */
  Drupal.wem.queue = {
    // The queue array. Contains events.
    queue: [],
    // Adds an event to the queue.
    add: function (event) {
      var timestamp = Drupal.wem.timestamp();
      this.queue.push([event.name, event.value, event.data, timestamp]);
      $.Storage.set('wem_queue', JSON.stringify(this.queue));
    },
    // Sends the queue to the back-end.
    run: function () {
      if (this.queue.length > 0) {
        $.ajax({
          url: Drupal.settings.wem.paths.api + '/' + Drupal.wem.token + '/' + Drupal.wem.uid,
          data: "events=" + JSON.stringify(this.queue),
          type: "POST",
          timeout: 5000,
          dataType: "json",
          success: function (data, textStatus, jqXHR) {
            Drupal.wem.queue.empty();
          },
          error: function (jqXHR, textStatus) {
            // Api will throw error if token/uid invalid, or no events tracked.
            Drupal.wem.resetInfo();
          }
        });
      }
    },
    // Empties the queue.
    empty: function () {
      this.queue = [];
      $.Storage.set('wem_queue', JSON.stringify(this.queue));
    },
    // Fetches the queue from storage.
    init: function () {
      var q;
      if (q = $.parseJSON($.Storage.get('wem_queue'))) {
        this.queue = q;
      }
    }
  }

  /**
   * Simply returns a Unix timestamp.
   */
  Drupal.wem.timestamp = function () {
    return Math.round((new Date()).getTime() / 1000);
  }

  /**
   * Checks to see if the user just logged in/out and reset the uid/tokens accordingly.
   */
  Drupal.wem.checkUserSession = function (tokenInvalid) {
    if ($('body').hasClass('not-logged-in') &&
      $.Storage.get('wem_uid') &&
      $.Storage.get('wem_uid').length !== 16) {
      // Need to refresh token/uid.
      Drupal.wem.resetInfo();
      return;
    }
    if (tokenInvalid) {
      Drupal.wem.resetInfo();
      return;
    }
  }

  /**
   * Reset the token and UID and get it from the server.
   */
  Drupal.wem.resetInfo = function () {
    $.Storage.remove('wem_uid');
    $.Storage.remove('wem_token');
    Drupal.wem.getToken();
    Drupal.wem.getUID();
  }

  /**
   * On page load we need to:
   * Check the user session to see if someone is logging in/out
   * Get a token
   * Get the user ID
   * Initialize the queue
   * Start the queue timer
   * Attach data events for elements with the data-wem attribute
   * Track a page view.
   */
  $(document).ready(function () {
    // Need to check for user login/logout.
    Drupal.wem.checkUserSession();
    Drupal.wem.getToken();
    Drupal.wem.getUID();
    Drupal.wem.queue.init();
    // We run the queue every 10 seconds. This minimizes the number of
    // requests we need to make to the server.
    setInterval('Drupal.wem.queue.run()', 5000);
    // Attach data events.
    Drupal.wem.attachDataEvents();
    // Track page view.
    Drupal.wem.track({name: "page_view", value: document.URL});
  });

}(jQuery));
