<?php 

/**
 * @file github_integration.pages.inc
 * 
 * Assorted pages and other one time functions that needn't be loaded on every bootstrap
 */

/**
 * @function github_integration_highscore_page()
 * 
 * Displays stats from different users across releases.
 * 
 * Problems
 *   1. API only returns contributors.  Users with open tickets are not contributors.
 *   2. Paging.  API only returns first 30 issues in a query.  No idea what the totals are.
 */
function github_integration_highscore_page() {
  //tickets created: resolved, unresolved.
  //tickets owned: resolved, unresolved.
  $milestones = github_integration_milestones(array('state' => 'closed', 'sort' => 'due_date', 'direction' => 'desc'));
//  shuffle($milestones);
  $milestone = current($milestones);
  $params = array('milestone' => $milestone['number'], 'per_page' => 100);
unset($params['milestone']);
  //get all users
  $client = github_integration_client();
  $users = $client->api('repo')->collaborators()->all(variable_get('github_repo_owner'), variable_get('github_repo_repository'));
  $users = ($users) ? array_map(create_function('$a', 'return $a["login"];'), $users) : array();
  $users = array_merge($users, array('brianap', 'gcaro', 'jjweiner', 'ebiewener', 'kstuart863', 'mfowkes', 'FordKelly', 'kentondoyle', 'ekiczek', 'Ferdi'));
  sort($users); 
  
  $headers = array(
    'User',
    'Closed issues I reported',
    'Open issues I reported',
    'Closed issues I own',
    'Open issues I own',  
      
  );
  
  /**
trophies

loudest: most reported issues.
diplomat: best reported ratio
   */
  
  $stats = array();
  foreach ($users as $user) {
    $stats[$user] = array(
      'name' => $user,
      'created_resolved' => _github_integration_count_issues($params + array('state' => 'closed', 'creator' => $user)),
      'created_unresolved' => _github_integration_count_issues($params + array('state' => 'open', 'creator' => $user)),
      'assigned_resolved' => _github_integration_count_issues($params + array('state' => 'closed', 'assignee' => $user)),
      'assigned_unresolved' => _github_integration_count_issues($params + array('state' => 'open', 'assignee' => $user)),
    );
  }
  //turnaround time?  shortest created -> closed time.
  $achievements = array(
    t('Loudmouth - Most tickets filed.') => array_map(create_function('$a', 'return $a["created_resolved"] + $a["created_unresolved"];'), $stats),
    t('Diplomat - Best success rate on tickets filed.') => array_map(create_function('$a', '$c = $a["created_resolved"] + $a["created_unresolved"]; return ($c) ? round($a["created_resolved"] / $c, 3) : 0;'), $stats),
    t('Git-R-Done - Best closure rate on owned tickets.') => array_map(create_function('$a', '$c = $a["assigned_resolved"] + $a["assigned_unresolved"]; return ($c) ? round($a["assigned_resolved"] / $c, 3) : 0;'), $stats),
    t('Slacker - Most open, unresolved tickets.') => array_map(create_function('$a', 'return $a["assigned_unresolved"];'), $stats),
    t('Poor Social Skills- Worst success rate on tickets filed.') => array_map(create_function('$a', '$c = $a["created_resolved"] + $a["created_unresolved"]; return ($c) ? round($a["created_unresolved"] / $c, 3) : 0;'), $stats),
  );
  dpm($achievements);
  
  $top = 5;
  $highscores = array();
  foreach ($achievements as $title => $scores) {
    asort($scores, SORT_NUMERIC);
    $scores = array_reverse($scores, TRUE);
    $winners = array();
    foreach ($scores as $name => $score) {
      $winners[] = count($winners) ? "$name ($score)" : "<strong>$name ($score)</strong>";
      if (count($winners) >= $top) {
        break;
      }
    }
    
    $highscores[] = "<h2>$title</h2>" . implode(', ', $winners);  
  }
  
  $table = array(
    '#theme' => 'table',
    '#header' => $headers,
    '#rows' => $stats,    
    '#prefix' => '<h2>Github High Scores for the ' . $milestone['title'] . ' release</h2>',   
  );
  
  return array(
    'highscores' => array(
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#theme' => 'item_list',
      '#items' => $highscores,
      '#type' => 'ul',
      '#attributes' => array('class' => array('highscores')),   
      '#attached' => array(
        'css' => array(array('type' => 'file', 'data' => drupal_get_path('module', 'github_integration') . '/github_integration.css'))
      ),
    ),
    'table' => $table,    
  );
}

/**
 * @function _github_integration_list_issues()
 * 
 * Wrapper for issue->all.  Fetches all issues matching a set of parameters.
 */
function _github_integration_count_issues($parameters) {
  $cache_key = 'github_integration_issue_count::' . http_build_query($parameters);
  $cached = cache_get($cache_key);
  if ($cached && $cached->expire > time()) {
    return $cached->data;
  }

  $issues = github_integration_issues($parameters);  
  $count = count($issues);  
  cache_set($cache_key, $count, 'cache', 60*60*24);
  return $count;
}
