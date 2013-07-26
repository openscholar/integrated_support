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

  //include github alerts
  if (module_exists('github_integration') && variable_get('getsatisfaction_roadmap_show_github_alert', TRUE)) {
    $alerts = github_integration_alert_page();
    if (!in_array($alerts, array('No reported service alerts.', 'Alerts disabled.'))) {
      $ret[] = array('alert' => $alerts); //array('#type' => 'markup', '#markup' => $alerts);
    }
  }
  
  //add legend
  $ret[] = _getsatisfaction_integration_roadmap_legend();
  
  if (user_access('administer site configuration')) {
    $ret[] = array(
      '#text' => 'Add tickets',
      '#path' => 'admin/config/integrated_support/getsatisfaction/tickets',
      '#theme' => 'link',
      '#options' => array('attributes' => array('class' => array('add_new'))),
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
  $styles = array('idea', 'problem', 'question', 'praise', 'update');
  $data = array_fill_keys($milestone_titles, array_fill_keys($styles, array()));
  
  
  //fill these milestones with their tickets
  while ($row = $results->fetchAssoc()) {
    $issue = github_integration_issue($row['external_id']);
    $topic = getsatisfaction_integration_topic($row['id']);
    $status = _getsatisfaction_integration_roadmap_status($topic, $issue); 
    $status_class = strtolower(str_replace(' ', '_', $status));
    $status_label = "<span class='status $status_class'>$status</span>"; 
    
    $append = &$data[$issue['milestone']][$topic->style];
    if (!$issue['milestone']) {
      continue;
    }
    
    $append[] = array('class' => array('issue-meta'), 'data'=>array(
      '<a href="javascript:void(0);" onclick="jQuery(\'.topic-'. $row['id']. '\').toggle()">+</a>',
      l($row['title'], $topic->at_sfn, array('attributes' => array('class' => array('style-' . $topic->style)))),
      format_interval(time() - strtotime($issue['updated_at']), 1),
      (user_access('administer site configuration')) ? l($status_label, $issue['html_url'], array('html' => TRUE)) : $status_label,
    ));
    
    $append[] = array(array('data' => $topic->content, 'colspan' => count($header), 'class' => array('issue-body topic-' . $row['id'])));   
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
  foreach ($data as $milestone => $ms_data) {
    if ($ms_data) {
      $ret[] = array(
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
      "<span class='style-$style'> &nbsp; </span>",
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
  if ($issue['milestone'] && isset($milestone_states[$issue['milestone']]) && $milestone_states[$issue['milestone']] == 'closed') {
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
    //'active' => 'In Progress', //retired.
    'complete' => 'Done',    
  );
  
  return (isset($status_map[$topic->status])) ? $status_map[$topic->status] : '';
}