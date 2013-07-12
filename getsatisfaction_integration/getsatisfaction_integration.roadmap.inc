<?php

/**
 * @function getsatisfaction_integration_roadmap_page
 *
 * Page shows current roadmap, where roadmap is the issues in sync between getsatisfaction and github
 */
function getsatisfaction_integration_roadmap_page() {
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
  $data = array_fill_keys($milestone_titles, array());
  
  //fill these milestones with their tickets
  while ($row = $results->fetchAssoc()) {
    $issue = github_integration_issue($row['external_id']);
    $topic = getsatisfaction_integration_topic($row['id']);
    $status = _getsatisfaction_integration_roadmap_status($topic, $issue);
    if (!$issue['milestone']) {
      continue;
    }
    
    if ($status) { // && in_array('roadmap', $issue['labels'])) {
      $data[$issue['milestone']][] = array('class' => array('issue-meta'), 'data'=>array(
        '<a href="javascript:void(0);" onclick="jQuery(\'.topic-'. $row['id']. '\').toggle()">+</a>',
        l($row['title'], $topic->at_sfn),
        format_interval(time() - strtotime($issue['updated_at']), 1),
        "<span class='$status status'>$status</span>",
      ));
      
      $data[$issue['milestone']][] = array(array('data' => $topic->content, 'colspan' => count($header), 'class' => array('issue-body topic-' . $row['id'])));
    }
  }

  array_filter($data);
  $ret = array();
  foreach ($data as $milestone => $ms_data) {
    $ret[] = array(
      '#theme' => 'table',
      '#prefix' => "<h2>$milestone</h2>",
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
  
  return $ret;
}

/**
 * @function _getsatisfaction_integration_issue_status
 * 
 * Determines roadmap status to use on an issue
 * @param unknown_type $issue
 */
function _getsatisfaction_integration_roadmap_status($topic, $issue) {
  $gh_client = github_integration_client();
  
  static $milestone_states;
  if (empty($milestone_states)) {
    $milestone_states = array();
    foreach (github_integration_milestones() as $milestone) {
      $milestone_states[$milestone['title']] = $milestone['state'];
    }
  }
  
  //Check if milestone is closed. 
  if ($issue['milestone'] && isset($milestone_states[$issue['milestone']]) && $milestone_states[$issue['milestone']] == 'closed') {
    return 'Deployed';
  }
  
  //Get status from GS
  $status_map = array(
    'pending' => 'Planned',
    'active' => 'Dev',
    'complete' => 'Coded',    
  );
  
  return (isset($status_map[$topic->status])) ? $status_map[$topic->status] : 'Planned';
}