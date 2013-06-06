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
  $update = array('id' => $case_id);
  foreach (array('github_issue_id', 'github_milestone', 'github_status') as $var) {
    if ($$var) {
      $update['custom_fields'][$var] = $$var;
    }
  }
  
  $ret = $desk->api('case')->call('update', $update);
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