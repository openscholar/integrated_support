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

  $header = array(' ', 'Topic', 'Owner', 'Last Update', 'Milestone', 'Status');
  $data = array();

  while ($row = $results->fetchAssoc()) {
    $issue = github_integration_issue($row['external_id']);
    $topic = getsatisfaction_integration_topic($row['id']);
    $status = _getsatisfaction_integration_roadmap_status($topic, $issue);
    
    if ($status) { // && in_array('roadmap', $issue['labels'])) {
      $data[] = array('class' => array('issue-meta'), 'data'=>array(
        '<a href="javascript:void(0);" onclick="jQuery(\'.topic-'. $row['id']. '\').toggle()">+</a>',
        l($row['title'], $topic->at_sfn),
        $issue['user'],
        format_interval(time() - strtotime($issue['updated_at']), 1),
        ($issue['milestone']) ? $issue['milestone'] : t('N/A'),
        l($status, $issue['html_url'], array('attributes' => array('class' => array('status', $status)))),
      ));
      
      $data[] = array(array('data' => $topic->content, 'colspan' => count($header), 'class' => array('issue-body topic-' . $row['id'])));
    }
  }

  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $data,  
    '#attributes' => array(
      'id' => 'roadmap',
      'border' => '0',
    ),
    '#attached' => array(
      'css' => array(array('type' => 'file', 'data' => drupal_get_path('module', 'getsatisfaction_integration') . '/getsatisfaction_integration_roadmap.css')),
    ),
  );
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