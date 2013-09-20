/**
 * Loads trello roadmap statuses via ajax
 **/

(function ($) {

Drupal.behaviors.trello_integration_roadmap = { 
  attach: function (context) {
    settings = Drupal.settings.trello_integration_roadmap;   
    
    //set up events for the status filters
    $('.status-filter').click(function(){
      $this = $(this)
      offset = $this.offset();
      $roadmap = $('#roadmap-legend');
      $roadmap
        .fadeToggle(125)
        .offset({top:offset.top+16, left:offset.left - $roadmap.width() + $this.width()});
      
      //also save current settings somewhere so we can cancel out of this godforsaken popup
    });

    //status filter buttons.  close window and update settings or restore old filter settings
    $('.legend-buttons > span').each(function(){
      $(this).click(function(){
        $roadmap = $('#roadmap-legend');
        $milestone = $('.milestone-wrapper');
        if ($(this).attr('id') == 'legend-cancel') {
          //restore previous settings and bail out
          if (settings.filter_state) {
            for (var i in settings.filter_state.off) {
              $roadmap.find('input[name="' + settings.filter_state.off[i] + '"]').attr('checked', false)
            }
            for (var i in settings.filter_state.on) {
              $roadmap.find('input[name="' + settings.filter_state.on[i] + '"]').attr('checked', true)
            }
          } else {
            //all off
            $roadmap.find('input').each(function() {
              $(this).attr('checked', false);
            })
          }
          
          
        } else if ($(this).attr('id') == 'legend-save') {
          //get on/off settings
          off = $.map($roadmap.find('input:not(:checked)'), function(elem) {
            return $(elem).attr('name')
          });
          on = $.map($roadmap.find('input:checked'), function(elem) {
            return $(elem).attr('name')
          });
          
          //hide off, show on
          if (on.length) {
            for (var status in off) {
              $milestone.find('ul.ticket-list > li > .' + off[status].toLowerCase()).hide()
            }
            for (var status in on) {
              $milestone.find('ul.ticket-list > li > .' + on[status].toLowerCase()).show()
            }
          } else {
            $milestone.find('ul.ticket-list > li > a').show();
          }
          
          //save state
          settings.filter_state = {'off': off, 'on': on};
          
          //change link
          status = (on.length > 0) ? 'Status Filter [' + on.join(', ') + ']' : 'Status Filter';
          $('.status-filter').html(status);
        }
        
        $roadmap.fadeOut(50);
      });
    });
    
    
    //attach focus event to milestone selector
    $select = $('select#select_milestone')
    $select.change(function() {
      option = $select.find('option:selected').html();
      if (settings.milestones[option]) {
        document.location.href = settings.roadmap_path + settings.milestones[option];
      }
    });
    
    //attach show/hide events to checkboxes
//    $('#roadmap-legend > form > label > input').change(function(e) {
//      
//      //If this is the first item disabled, make it the only one enabled as though you entered a taxonomy term list.
//      $unchecked = $('#roadmap-legend > form > label > input:not(:checked)');
//      if ($unchecked.length == 1 && $unchecked.attr('name') == e.target.name) {
//        $('#roadmap-legend > form > label > input:checked').each(function(){
//          //uncheck the remain ones
//          $(this).click();
//          sel = 'ul.milestone li > a.' + $(this).attr('name').toLowerCase();
//          $(sel).hide();
//        })
//        
//        $unchecked.click(); //renable the clicked one.
//      }
//      
//      sel = 'ul.milestone li > a.' + e.target.name.toLowerCase();
//      if (e.target.checked) {
//        $(sel).show();
//      } else {
//        $(sel).hide();
//      }
//    })

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
          $this.addClass('done')
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