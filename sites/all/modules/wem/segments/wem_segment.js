(function ($) {

  Drupal.wem_segment = {}

  /**
   * Adds admin options to the node form and settings page.
   */
  Drupal.behaviors.wem_segment = {
    attach: function (context) {
      Drupal.wem_segment.enhanceAdminForm();
      Drupal.wem_segment.enhanceNodeForm(context);
      if (typeof Drupal.settings.wem.segment === 'undefined') {
        return false;
      }
      if (Drupal.settings.wem.segment.nodeFormCopy == "WEM Enabled") {
        Drupal.wem_segment.showAdminOptions();
      }
      else {
        Drupal.wem_segment.hideAdminOptions();
      }
      // Set GUA custom dimension with current visitor's segments, if possible.
      if (Drupal.wem.ga_universal() && Drupal.settings.wem.analytics.ga.dimensions.segment) {
        var segments = Drupal.settings.wem.segment.user_segments;
        // Concatenate segment names into a pipe-delimited string.
        var segment_list = '';
        $.each(segments, function(name, value) {
          if (!isNaN(name)) {
            segment_list += value + ' | ';
          }
        });
        // Remove the trailing delimiter chars.
        segment_list = segment_list.substr(0, segment_list.length - 3);
        // If there are any user segments, report them to GA using the
        //  specified dimension.
        if (segment_list.length > 0) {
          var dimension_number = Drupal.settings.wem.analytics.ga.dimensions.segment;
          if (typeof(ga) !== 'undefined') {
            ga('set', 'dimension' + dimension_number, segment_list);
          }
        }
      }
    }
  }

  /**
   * Adds settings to the node form.
   */
  Drupal.wem_segment.enhanceNodeForm = function (context) {
    if ($('.page-node-add fieldset#edit-segments, .page-node-edit fieldset#edit-segments').length > 0) {
      $('fieldset#edit-segments', context).drupalSetSummary(function (context) {
        var pt = $('input', context).val();
        return Drupal.t(Drupal.settings.wem.segment.nodeFormCopy);
      });
      $('#edit-segments-enable', context).bind('click', function (e) {
        if ($(this).is(":checked")) {
          Drupal.wem_segment.showAdminOptions();
        }
        else {
          Drupal.wem_segment.hideAdminOptions();
        }
      });
    }
  }

  /**
   * Hides admin options on the node form.
   */
  Drupal.wem_segment.hideAdminOptions = function () {
    $('#edit-segments-content').hide();
  }

  /**
   * Shows admin options on the node form.
   */
  Drupal.wem_segment.showAdminOptions = function () {
    $('#edit-segments-content').show();
  }

  /**
   * Enhances the admin form for creating rules.
   */
  Drupal.wem_segment.enhanceAdminForm = function () {
    // User is on the rules page.
    if ($('#wem-segment-admin-rule-form').length > 0) {
      $('#edit-event-category').change(function () {
        // If user selects 'custom' option, show textbox.
        if ($(this).val() == 'create_new_category') {
          $('#edit-event-category').hide();
          $('#edit-event-category-custom').show().focus();
        }
      });
      $('#edit-event-category-custom').blur(function () {
        // If textbox is empty on blur, show the select and hide the textbox.
        if ($(this).val() == '') {
          $('#edit-event-category-custom').hide();
          $('#edit-event-category').show().focus();
          $('#edit-event-category').val(0);
        }
      })
    }
  }

}(jQuery));
