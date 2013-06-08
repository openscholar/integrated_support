<?php

/**
 * @function desk_update_test_issue
 * 
 * Page callback for confirming that we can update custom fields
 */
function desk_update_test_issue() {  
  $ret = desk_update_case('4', '123', 'my milestone', 'Open');
  return "\n\n<br><Br>" . var_export($ret, TRUE);
}

/**
 * @function desk_update_case
 * 
 * Updates the desk issue specified with $case_id with the other properties.
 */
function desk_update_case($case_id, $github_issue_id, $github_milestone = NULL, $github_status = NULL) {
  $desk = desk_get_client();
  $github_status = _desk_validate_github_status($github_status);
  //$state = (isset($github_status) && in_array(ucfirst($github_status), array('Open', 'Closed'))) ? ucfirst($github_status) : '--'; //must be one of these three values.
  
  $update = array('id' => $case_id);
  foreach (array('github_issue_id', 'github_milestone', 'github_status') as $var) {
    if ($$var) {
      $update['custom_fields'][$var] = $$var;
    }
  }
  
    

  
  
  $ret = $desk->api('case')->call('update', $update);
  if (isset($ret->errors) && $ret->errors) {
    error_log($ret->mesage . var_export($ret->errors, TRUE) . var_export($update, TRUE));
  }
  return $ret;
}

/**
 * @function desk_get_client
 * 
 * Returns client object for querying desk
 */
function desk_get_client() {
  static $desk_client;
  if (!empty($desk_client)) {
    return $desk_client;
  }

  $conf = conf();
  require_once('lib/tapir/tapir.php');

  $desk = new Tapir('desk');
  $desk->setParameters(array('subdomain' => 'jsagotsky'));
  $desk->useOAuth($conf['desk_consumer_key'], $conf['desk_consumer_secret'], $conf['desk_token'], $conf['desk_token_secret']);

  return $desk;
}

/**
 * @function desk_status
 *
 * Checks that desk can auth
 */
function desk_status() {
  $desk = desk_get_client();
  $search = $desk->api('case')->call('search', array('case_id' => 4));
  return (isset($search->total_entries)) ? 'Logged into desk.com' : 'Error logging in to desk.';
}


/**
 * @function desk_create_github_issue
 *
 * Callback function for creating a github issue from a desk.com api post
 */
function desk_create_github_issue() {
  if (!($data = json_post('data'))) {
    error_log('No json data in post.  $_POST:' . var_export($_POST, TRUE));
  }
  
  if (!isset($data->case_id)) {
    return;
  }

  $conf = conf();

  //create github issue
  $body = array(
    'source' => 'Desk.com',
    'link' => $conf['desk_url'] . '/agent/case/' . $data->case_id, 
    'text' => $data->case_body,
    'os' => $data->os,
    'os_version' => $data->os_version,
    'browser' => $data->os_browser,
    'browser_version' => $data->os_browser,
  );
  
  $issue = array(
    'title' => ($data->case_subject) ? $data->case_subject : 'Issue from desk.com',
    'assignee' => (isset($conf['user_map'][$data->case_user])) ? $conf['user_map'][$data->case_user] : NULL,
    'labels' => 'desk',
    'body' => github_template_body($body), //implode("\n", $issue_body),
  );

  $issue_ret = github_create_issue($issue);
  
  //send response back to desk
  $desk_ret = desk_update_case($data->case_id, $issue_ret['number'], $issue_ret['milestone'], $issue_ret['state']); 
//   //take $i and send id or number back to desk.com
//   $update = array('id' => $data->case_id, 'custom_fields' => $gh_properties);
//   $desk = desk_get_client();
//   $desk_out = $desk->api('case')->call('update', $update);
   
//   error_log('Payload: ' . json_encode($update));
  error_log('GH Response: ' . var_export($issue_ret, TRUE) . "\n\n");
  error_log('Desk Response: ' . var_export($desk_ret, TRUE));
}

function desk_preview_github() {
  //should include gh id so we can avoid dupes
  
  if (!isset($_REQUEST['payload'])) {
    print_r($_REQUEST);
    return 'no payload';
  }
  
  if (($json = json_decode($_REQUEST['payload'])) == FALSE) {
    print_r($_REQUEST['payload']);
    return "\n\ncould not decode json: " . json_last_error();
  }
  if (empty($json->case_id)) {
    print_r($_REQUEST); //might be invalid json
    return 'Can\'t send to github until case is created and has id';
  }

  if ($json->case_custom_github_issue_id) {
    return 'Case already has a github id.';
  }



  $conf = conf();
  
  $body = array(
    'source' => 'Desk.com',
    'link' => $conf['desk_url'] . '/agent/case/' . $json->case_id,
    'text' => $json->case_body,
    'os' => $json->os,
    'os_version' => $json->os_version,
    'browser' => $json->os_browser,
    'browser_version' => $json->os_browser,
  );
  
  $issue = array(
    'title' => ($json->case_subject) ? $json->case_subject : 'Issue from desk.com',
    'assignee' => (isset($conf['user_map'][$json->case_user])) ? $conf['user_map'][$json->case_user] : NULL,
    'labels' => 'desk',
    'body' => github_template_body($body), //implode("\n", $issue_body),
  );
  

  
  $issue_ret = github_create_issue($issue);
  
  //send response back to desk
  $desk_ret = desk_update_case($data->case_id, $issue_ret['number'], $issue_ret['milestone'], $issue_ret['state']); 
  
  print_r($issue_ret);
  print_r($desk_ret);

}


/*
 * @function _desk_validate_github_status($github_status);
 * 
 * Make sure github_status is one of the three valid options.
 */
function _desk_validate_github_status($github_status) {
  if (strtolower($github_status) == 'open') {
    return 'Open';
  } elseif (strtolower($github_status) == 'closed') {
    return 'Closed';
  }
  
  return '--';
}
