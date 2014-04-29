/**
 * Loads trello roadmap statuses via ajax
 **/

(function ($) {

Drupal.behaviors.trello_integration_roadmap = { 
  attach: function (context) {
    var settings = Drupal.settings.trello_integration_roadmap;   
    
    //attach focus event to milestone selector
    var $select = $('select#select_milestone')
    $select.change(function() {
      option = $select.find('option:selected').html();
      if (settings.milestones[option]) {
        document.location.href = settings.roadmap_path + 'archive/' + settings.milestones[option];
      }
    });
    
    //attach show/hide events to checkboxes
    $('#roadmap-legend > form > label > input').change(function(e) {
      //If everybody is unchecked, filtering is off.  Show all the things!
      $unchecked = $('#roadmap-legend > form > label > input:not(:checked)');

      if ($unchecked.length == 6) {   
        $('ul.ticket-list li > div').show();
      } else {
        //show/hide according to the checkboxes
        $('ul.ticket-list li > div.get-status').hide(); //tickets without a status are always hidden on filter
        $sel = $('ul.ticket-list li');
        $('#roadmap-legend > form > label > input').each(function(){
          $this = $(this);
          if ($this.attr('checked')) {
            $sel.find('div.' + $this.attr('name').toLowerCase()).show();
          } else {
            $sel.find('div.' + $this.attr('name').toLowerCase()).hide();
          }
        });
      }
    });
    
    $('.roadmap-item').click(function (e) {
      var elem = $('#modal');
      if (elem.length == 0) {
        elem = $('<div id="modal"></div>').appendTo('body').dialog({
          autoOpen: false,
          modal: true,
          resizable: true,
          minWidth: 700,
          position: {
            my: 'center top',
            at: 'center top+100'
          },
          close: function () {
            location.hash = '';
          }
        });
      }
      
      elem.html($('.roadmap-popup-text', this).html()).dialog('open');
      elem.dialog('option', 'title', $('span', this).html());
      location.hash = $(this).attr('data-hash'); 
    });
    
    if (location.hash) {
      $('.roadmap-item[data-hash="'+location.hash.substr(1)+'"]').click();
    }

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
    
      $('.get-status[data-gh-id]').each(function() {
        var $this = $(this);
        var this_id = $this.attr('data-gh-id');
        if (id_status[parseInt(this_id)]) {
          $this.addClass(id_status[this_id].toLowerCase())
            .removeClass('get-status');;
        }
      });
    };
    
    /**
     * @function display_closed
     * 
     * Closed tickets have their own ajax call.  Mark them as done as well.
     */
    var display_closed = function(data) {
      $('.get-status[data-gh-id]').each(function() {
        var $this = $(this);
        var id = parseInt($this.attr('data-gh-id'));
        if ($.inArray(id, data) > -1) {
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
    
    // This is a hack to keep us from overridding the menu_link TPL just for this page
    $('#tasks li').has('a[href$="/roadmap"]').attr('title',"Issues currently being worked on.");
    $('#tasks li').has('a[href$="/roadmap/planned"]').attr('title',"These issues have been reviewed and are listed in order of priority. If you feel something needs a different level of priority, please join the conversation and let us know.");
    $('#tasks li').has('a[href$="/roadmap/archive"]').attr('title',"Issues worked on in past releases.");
    $('#tasks li').has('a[href$="/roadmap/to-review"]').attr('title',"These issues have been received but not yet reviewed.");
  }
};

})(jQuery);  


  jQuery(function() {
    jQuery( document ).tooltip({
      track: true
    });
  });

