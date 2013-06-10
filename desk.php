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
  //get request data for each of the template vars
  $data = array();
  require_once('liquid.inc');
  foreach (array_keys(desk_liquid_vars()) as $var) {
    $data[$var] = $_REQUEST[$var];
  }
  
  //Don't create a ticket from a case with no id.  There's no way to update the original case with the gh_id.  
  if (empty($data['case_id'])) {
    return 'Case has no id.  Send after its saved so GH can reference it.';
  }
  
  //don't create tickets from cases that are already assigned to an id
  if ($data['case_custom_github_issue_id']) {
    return 'Case already has a github id.';
  }
  
  //build a github ticket from the desk payload
  $conf = conf();
  $body = array(
    'source' => 'Desk.com',
    'link' => $conf['desk_url'] . '/agent/case/' . $data['case_id'],
    'text' => $data['case_body'],
    'os' => $data['os'],
    'os_version' => $data['os_version'],
    'browser' => $data['os_browser'],
    'browser_version' => $data['os_browser'],
  );
  
  $issue = array(
    'title' => ($data['case_subject']) ? $data['case_subject'] : 'Issue from desk.com',
    'assignee' => (isset($conf['user_map'][$data['case_user_name']])) ? $conf['user_map'][$data['case_user_name']] : NULL,
    'labels' => 'desk',
    'body' => github_template_body($body), //implode("\n", $issue_body),
  );
  
  //create the ticket
  $github_ret = github_create_issue($issue);
  if (!$github_ret['number']) {
    return 'Github response:' . serialize($github_ret);
  }
  
  //send response back to desk
  $desk_ret = desk_update_case($data['case_id'], $github_ret['number'], $github_ret['milestone'], $github_ret['state']); 
  
  
  //redirect to github or show some errors.
  if ($desk_ret && $desk_ret->custom_fields->github_issue_id) {
    return html_redirect($github_ret['url']);
  } else {
    $out[] = 'Error updating desk case.';
    $out[] = '';
    $out[] = 'Original request:';
    $out[] = serialize($_REQUEST);
    $out[] = '';
    $out[] = 'Github response:';
    $out[] = serialize($github_ret);
    $out[] = '';
    $out[] = 'Desk update response:';
    $out[] = serialize($desk_ret);
    
    return implode("\n", $out);
  }
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
