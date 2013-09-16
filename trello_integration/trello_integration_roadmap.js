/**
 * Loads trello roadmap statuses via ajax
 **/

(function ($) {

Drupal.behaviors.trello_integration_roadmap = { 
  attach: function (context) {
    settings = Drupal.settings.trello_integration_roadmap;
    
    /**
     * @function display_statuses
     * 
     * loop over known statuses and their gh ids, applying statuses when foudn.
     **/
    var display_statuses = function(data) {
      var id_status = {};
      for (var status in data) {
        for (var i in data[status]) {
          id_status[ data[status][i] ] = status;
        }
      }
    
      $('a.get-status').each(function() {
        var $this = $(this);
        var this_id = $this.attr('id').split('-')[2];
        if (id_status[parseInt(this_id)]) {
          $this.addClass(id_status[this_id].toLowerCase())
            .removeClass('get-status');
        }
      });
      
      $('.roadmap .get-status').removeClass('get-status'); //clear remaining throbbers
    };
    
    /**
     * @function display_closed
     * 
     * Closed tickets have their own ajax call.  Mark them as done as well.
     */
    var display_closed = function(data) {
      $('a.get-status').each(function() {
        var $this = $(this);
        var id = $this.attr('id').split('-')[2];
        
        if ($.inArray(id, data)) {
          $this.addClass('closed')
            .removeClass('get-status');
        }
      });
    }
    
    
    //use statuses if preloaded, otherwise ajax them.
    if (settings.roadmap_statuses) {
      display_statuses(settings.roadmap_statuses);
    } else {
      $.getJSON(settings.ajax_path + '/statuses', '', display_statuses);
    }
    
    //as above, but for closed issues.
    if (settings.roadmap_closed) {
      display_closed(settings.roadmap_closed);
    } else {
      $.getJSON(settings.ajax_path + '/closed/' + settings.milestone, '', display_closed);
    }
  }
};

})(jQuery);  