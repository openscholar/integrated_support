<?php

/**
 * @function getsatisfaction_integration_roadmap_page
 *
 * Page shows current roadmap, where roadmap is the issues in sync between getsatisfaction and github
 */
function getsatisfaction_integration_roadmap_page() {
  $ret = array(
    '#prefix' => '<h2 class="roadmap">Roadmap</h2>',    
  );

  //admins get extra links
  if (user_access('administer site configuration')) {
    //add tickets
    $items = array(
      l('Add tickets', 'admin/config/integrated_support/getsatisfaction/tickets',array('attributes' => array('class' => array('add_new')))),
    );
    
    //refresh github milestones
    if (module_exists('github_integration')) {   
      $items[] = l('Refresh milestones', 'admin/config/integrated_support/github/refresh', array('attributes' => array()));
    }  
    
    $ret[] = array(
      '#theme' => 'item_list',
      '#items' => $items,
      '#type' => 'ul',
      '#attributes' => array(),   
    );
  };

  $results = db_select('getsatisfaction_service_map', 'gs')
    ->condition('external_service', 'github_integration')
    ->fields('gs')
    ->execute();

  $header = array(
    array('data' => ' ', 'width' => '1em'),
    array('data' => 'Topic', 'width' =>'75%'),
    'Last Update', 
    'Status'
  );

  //get list of milestones.  soonest first.  finished at the bottom
  $todo = array('state' => 'open', 'sort' => 'due_date', 'direction' => 'asc');
  $done = array('state' => 'closed', 'sort' => 'due_date', 'direction' => 'desc');
  $milestones = github_integration_milestones($todo) + github_integration_milestones($done);
  $milestone_titles = array_map(create_function('$a', 'return $a["title"];'), $milestones);
  $styles = array('idea', 'problem', 'question', 'praise', 'update', 'github');
  $data = array_fill_keys($milestone_titles, array_fill_keys($styles, array()));
  $gh_ids = array();

  //fill these milestones with their tickets
  while ($row = $results->fetchAssoc()) {
    $issue = github_integration_issue($row['external_id']);
    $topic = ($row['id']) ? getsatisfaction_integration_topic($row['id']) : 
      (object) array('style' => 'github', 'at_sfn' => $issue['html_url'], 'content' => '', 'subject' => $issue['title'], 'id' => NULL);
    $append = &$data[$issue['milestone']][$topic->style];
    $gh_ids[] = $issue['id'];

    if ($issue['milestone']) {
      $append[] = _getsatisfaction_integration_roadmap_render_row($topic, $issue);
      $append[] = array(array('data' => $topic->content, 'colspan' => count($header), 'class' => array('issue-body topic-' . $row['id'])));
    }   
  }

  //now add tagged GH issues
  $labels = variable_get('getsatisfaction_integration_github_roadmap_label', 'roadmap');
  $issues = github_integration_issues(array('labels' => $labels, 'state' => 'closed'));
  $issues = array_merge($issues, github_integration_issues(array('labels' => $labels, 'state' => 'open')));    
  foreach ($issues as $issue) {
    if ($issue['milestone'] and !in_array($issue['number'], $gh_ids)) {
      $issue['labels'] = array_map(create_function('$a', 'return $a["name"];'), $issue['labels']);
      $ms = $issue['milestone']['title'];
      $topic = (object) array('style' => 'github', 'at_sfn' => $issue['html_url'], 'content' => '', 'subject' => $issue['title'], 'id' => NULL);
      $append = &$data[$ms][$topic->style];

      $append[] = _getsatisfaction_integration_roadmap_render_row($topic, $issue);
      $append[] = array(array('data' => $topic->content, 'colspan' => count($header), 'class' => array('issue-body topic-' . $row['id'])));
    }
  }


  //flatten the nested topic styles
  foreach ($data as $ms => $styles) {
    $flattened = array();
    foreach ($styles as $style) {
      if ($style) {
        $flattened = array_merge($flattened, $style);
      }
    }
    $data[$ms] = $flattened;
  }


  //show each milestone
  $tables = array();
  foreach ($data as $milestone => $ms_data) {
    if ($ms_data) {
      $tables[] = array(
        '#theme' => 'table',
        '#prefix' => "<div class='milestone'><h2>$milestone</h2>",
        '#suffix' => '</div>',
        '#header' => $header,
        '#rows' => $ms_data,  
        '#attributes' => array(
          'class' => array('roadmap'),
          'border' => '0',
        ),
        '#attached' => array(
          'css' => array(array('type' => 'file', 'data' => drupal_get_path('module', 'getsatisfaction_integration') . '/getsatisfaction_integration_roadmap.css')),
        ),
      );
    }
  }

  $ret[] = array(
    '#prefix' => "<div class='milestonewrapper'>",
    '#suffix' => '</div>',
    'milestones' => $tables,
  );

  //include github alerts
  if (module_exists('github_integration') && variable_get('getsatisfaction_roadmap_show_github_alert', TRUE)) {
    $alerts = github_integration_alert_page();
    if (!in_array($alerts, array('No reported service alerts.', 'Alerts disabled.'))) {
      $ret[] = array('alert' => $alerts); //array('#type' => 'markup', '#markup' => $alerts);
    }
  }

  //add legend
  $ret[] = _getsatisfaction_integration_roadmap_legend();

  return $ret;
}

/**
 * @function _getsatisfaction_integration_roadmap_legend
 * 
 * Draws a legend describing elements of the roadmap
 */
function _getsatisfaction_integration_roadmap_legend() {
  $header = array(
    array('data' => 'Label', 'width' =>'8em'),
    array('data' => 'Description'),
  );

  $statuses = array(
    'Design' => t('In the design phase prior to development'),
    'Development' => t('Actively being worked on by a developer'),
    'Review' => t('Code is being reviewed prior to being sent to QA'),
    'Testing' => t('Being tested by QA personnel'),
    'Done' => t('Completed, ready for deployment'),
    'Pending' => t('Work is blocked by dependent functionality'),
  );

  $styles = array(
    'idea' => t('New feature idea'),
    'bug' => t('System bug'),
    'github' => t('Infrastructure')
  );

  $rows = array();

  foreach ($statuses as $name => $desc) {
    $class = strtolower(str_replace(' ', '_', $name));
    $rows[] = array(
      "<span class='status $class'>$name</span>",
      $desc,      
    );
  }

  foreach ($styles as $style => $desc) {
    $rows[] = array(
      "<span class='style style-$style'> &nbsp; </span>",
      $desc,  
    );
  }

  return array(
    '#theme' => 'table',
    '#prefix' => "<div class='legend'><h2>Legend</h2>",
    '#suffix' => '</div>',
    '#rows' => $rows,
    '#header' => $header,
    '#attributes' => array(
      'class' => array('roadmap'),
      'border' => '0',
    )
  );
}

/**
 * @function _getsatisfaction_integration_roadmap_render_row
 * 
 * Turns a github issue and getsatisfaction topic into a data row for the roadmap table
 */
function _getsatisfaction_integration_roadmap_render_row($topic, $issue) {
  $status = _getsatisfaction_integration_roadmap_status($topic, $issue);
  $status_class = strtolower(str_replace(' ', '_', $status));
  $status_label = "<span class='status $status_class'>$status</span>";

  $return = array('class' => array('issue-meta'), 'data'=>array(
    ($topic->id) ? '<a href="javascript:void(0);" onclick="jQuery(\'.topic-'. $topic->id . '\').toggle()">+</a>' : '',
    l($topic->subject, $topic->at_sfn, array('attributes' => array('class' => array('style style-' . $topic->style)))),
    format_interval(time() - strtotime($issue['updated_at']), 1),
    (user_access('administer site configuration')) ? l($status_label, $issue['html_url'], array('html' => TRUE)) : $status_label,
  ));

  return $return;
}

/**
 * @function _getsatisfaction_integration_issue_status
 * 
 * Determines roadmap status to use on an issue
 */
function _getsatisfaction_integration_roadmap_status($topic, $issue) {
  if (!$issue) {
    return '';
  }

  $label_status = variable_get('getsatisfaction_roadmap_status_labels', array(
    'status: pending' => 'Pending',
    'status: needs spec' => 'Design',
    'status: in dev' => 'Development',
    'status: needs review' => 'Review',
    'status: needs qa' => 'Testing',
    'status: fixed' => 'Done',  
  ));

  static $milestone_states;
  if (empty($milestone_states)) {
    $milestone_states = array();
    foreach (github_integration_milestones() as $milestone) {
      $milestone_states[$milestone['title']] = $milestone['state'];
    }
  }

  //Check if milestone is closed. 
  if (@$milestone_states[$issue['milestone']] == 'closed') {
    return 'Done';
  }

  if ($issue['state'] == 'closed') {
    return 'Done';
  }

  //Get status from github label.  Order matters.
  $status = '';
  foreach ($label_status as $label => $text) {
    if (in_array($label, $issue['labels'])) {
      $status = $text;
    }
  }

  if ($status) {
    return $status;
  }

  //Get status from GS if still undetermined.
  $status_map = array(
    'pending' => '',
    'active' => '',
    'complete' => 'Done',    
  );

  return (isset($topic->status, $status_map[$topic->status])) ? $status_map[$topic->status] : '';
}
