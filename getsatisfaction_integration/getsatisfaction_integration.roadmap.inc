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

  $items = array();
  $label_status = array('status: in dev' => 'Dev', 'status: in qa' => 'QA', 'status: needs review' => 'QA', 'status: fixed' => 'Done');

  while ($row = $results->fetchAssoc()) {
    $issue = github_integration_issue($row['external_id']);
    $topic = getsatisfaction_integration_topic($row['id']);

    $status = 'Planned';
    foreach ($label_status as $key => $val) {
      if (in_array($key, $issue['labels'])) {
        $status = $val;
      }
    }

    if ($status && in_array('roadmap', $issue['labels'])) {
      //theme function me please
      //it's what i was meant to be
      //some day, some day soon
      $item = "<A href='$topic[url]'>$row[title]</a> ";
      $item .= "[<a href='https://github.com/openscholar/integrated_support/issues/$row[external_id]'>$status</a>]";
      $item .= "<p>$topic[content]</p>";
      $items[] = $item;
    }
  }

  return theme('item_list', array('items' => $items, 'type' => 'ul'));
}
