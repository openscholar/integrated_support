<?php


function github_get_client($auth = TRUE) {
  static $gh_client;
  if (!empty($gh_client)) {
    return $gh_client;
  }

  $conf = conf();
  $cache = new Github\HttpClient\CachedHttpClient(array('cache_dir' => __DIR__ . '/.github-api-cache'));
  $gh_client = new Github\Client($cache);

  if ($auth) {
    try {
      $gh_client->authenticate($conf['github_auth_token'], '', Github\Client::AUTH_HTTP_PASSWORD);
    } catch (Exception $e) {
      print 'error';
      $gh_client = NULL;
    }
  }

  return $gh_client;
}

/**
 * @function github_hook_comments
 *
 * Recieves github's webhook for comments being added to an issue
 */
function github_hook_issue() {
  //get info from github payload
  $body = file_get_contents('php://input');
  $json = json_decode($body);
  //error_log(var_export($json, TRUE));
  

  if ($json->action != 'created') {
    return;
  }

  $id = $json->issue->number;
  $milestone = $json->issue->milestone->title;
  $state = $json->issue->state;

  //get related desk issues
  $desk = desk_get_client();
  $results = $desk->api('case')->call('search', array('case_custom_github_issue_id' => $id));
  
  $desk_log = array();
  //send changes to desk.com
  if ($results->total_entries) {
    //this will need another loop if we ever go over 50 items.  desk makes it easy with the next link, but I'm not sure other apis will do that
    foreach($results->_embedded->entries as $entry) {
      if (($milestone != $entry->custom_fields->github_milestone) || ($state != $entry->custom_fields->github_status)) {
        $case_id = end(explode('/', $entry->_links->self->href));      
        $desk_log[] = desk_update_case($case_id, $id, $milestone, $state);
      }
    }
  }
  
  error_log('recieved github comment or state change');
  //error_log(var_export($json, TRUE) . "\n\ndesk call results:\n");
  error_log(var_export($desk_log, TRUE));
}

/**
 * @function github_hook_issue
 *
 * Recieves github's webhook for issues being created or deleted
 */
// function github_hook_issue() {
//   //this is actually the same as the comment hook, but I wanted to keep the urls separate in case we ever want to handle them differently.
//   github_hook_comment();
// }

/**
 * @function github_status
 *
 * Checks that github can auth
 */
function github_status() {
  $gh = github_get_client();
  $me = $gh->api('current_user')->show();
  return ($me) ? 'Logged into github as: ' . $me['login'] : 'Could not auth github';
}


function github_create_issue($issue) {
  $gh = github_get_client();
  $conf = conf();
  if (!isset($issue['title'])) {
    throw new Exception('Can\'t create a github issue with no title.');
  }
  
  return $gh->api('issue')->create($conf['github_repo_owner'], $conf['github_repo_repository'], $issue);
}

/**
 * @function github_template_body
 * 
 * Builds markdown for a github body
 * 
 * Available options:
 *  source
 *  link
 *  text
 *  os
 *  os_version
 *  browser 
 *  browser_version
 */
function github_template_body($opts = array()) {
  $body = array();
  
  //link
  if (isset($opts['source'])) {
    $body[] = (isset($opts['link'])) ? "**From $opts[source]:** $opts[link]" : "**From $opts[source]**";
  }
  
  //body - quote original
  if (is_string($opts['text'])) {
    $body[] = '```' . $opts['text'] . '```';
  } elseif (is_array($opts['text'])) {
    $body[] = '> ' . implode("\n>> ", $opts['text']);
  }
  
  //os
  if (isset($opts['os'], $opts['os_version'])) {
    $body[] = 'OS: ' . $opts['os'] . ' ' . $opts['os_version'];
  }
  
  //browser  
  if (isset($opts['browser'], $opts['browser_version'])) {
    $body[] = 'Browser: ' . $opts['browser'] . ' ' . $opts['browser_version'];
  }
  
  return implode("\n\n", $body);
}