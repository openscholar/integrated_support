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
    $issue = _getsatisfaction_integration_github_issue($row['external_id']);
    $topic = _getsatisfaction_integration_gs_topic($row['id']);

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

/**
 * @function _getsatisfaction_integration_github_issue
 *
 * Fetch a github issue by id.
 */
function _getsatisfaction_integration_github_issue($id) {
  //why no cache?  github lib does it for us.
  $gh = github_integration_client(FALSE); //authed req is better.  100x the rate limit.  when it works again, remove this false
  $issue = $gh->api('issue')->show(variable_get('github_repo_owner'), variable_get('github_repo_repository'), $id);
  $data = array(
    'labels' => array_map(create_function('$a', 'return $a["name"];'),$issue['labels']),
    'state' => $issue['state'],
    'milestone' => $issue['milestone']
  );

  return $data;
}

/**
 * @function _getsatisfaction_integration_gs_topic(
 *
 * Fetch a GS topic by id.
 */
function _getsatisfaction_integration_gs_topic($id) {
  $cache_key = 'getsatisfaction:topic:'.$id;
  $cache = cache_get($cache_key);

  if ($cache && time() < $cache->expire) {
    return $cache->data;
  }

  $client = getsatisfaction_integration_client();
  $topic = $client->api('topic')->call('topic', array('id' => $id));
  $data = array(
    'url' => $topic->at_sfn,
    'content' => $topic->content,
  );
  cache_set($cache_key, $data, 'cache', time() + 604800);
  return $data;
}