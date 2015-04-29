(function ($) {

  Drupal.wem_report = {}

  /**
   * Enhances the admin form on the reporting page.
   */
  Drupal.wem_report.adminFormEnhance = function (ctx) {
    // Check for view data links on page.
    if ($('.page-admin-reports-wem .wem-report-data', ctx).length > 0) {
      $('.wem-report-data', ctx).click(function (e) {
        e.preventDefault();
        alert(JSON.stringify($(this).data('wem-report-data')));
      });
    }
  }

  Drupal.behaviors.wem_report = {
    attach: function (context) {
      Drupal.wem_report.adminFormEnhance(context);
    }
  }

}(jQuery));
