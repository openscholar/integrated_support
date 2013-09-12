/**
 * Loads trello roadmap statuses via ajax
 **/

(function ($) {

Drupal.behaviors.trello_integration_roadmap = { 
  attach: function (context) {
    settings = Drupal.settings.trello_integration_roadmap;
    
    //loop over known statuses and their gh ids, applying statuses when foudn.
    var display_statuses = function(data) {
      for (var status in data) {
        for (var id in data[status]) {
          $('.roadmap span.get-status.status-gh-'+data[status][id])
            .addClass(status.toLowerCase())
            .removeClass('get-status')
            .html(status)
        }
      }
      
      $('.roadmap .get-status').removeClass('get-status'); //clear remaining throbbers
    };
    
    //use preloaded statuses or fetch and then display them
    if (settings.roadmap_statuses) {
      display_statuses(settings.roadmap_statuses);
    } else {
      //add throbber
      $.getJSON(settings.ajax_path, '', display_statuses);
    }
  }
};

})(jQuery);  