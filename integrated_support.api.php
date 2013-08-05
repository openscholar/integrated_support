<?php

/**
 * Try to connect to an integration service.  Return whether or not that connection
 * was successful.
 */
function hook_integrated_support_status() {
  $client = service_get_client();
  $data = $client->connect();

  return ($data) ? 'Service connected' : 'Service failed';
}

/**
 * Defines a github event.
 * 
 * Key is the system name.
 * 	name - Human readable name
 *  description - Human readable description
 *  process function - When a service sends data, this function should handle it before data gets sent to hooks
 *  complete function - After all hooks have responded, gives this module one last chance to react.  Completion function takes two args - the initial payload and an array of responses from the modules that acted on it.
 *  setup function - (optional) Configures remote service to send this event
 *   
 */
function hook_integration_integrated_support_info() {
  return array(
    'github_issue' => array(
      'name' => t('Github Issue'),
      'description' => t('Github notifies Integrated Support whenever an issue is opened, closed, or commented on.'),
      'process function' => 'github_integration_github_issue_prep',
      'setup function' => 'github_integration_setup',
    )
  );
}

/**
 * After a webhook has been recieved, implement this hook to respond to it
 */
function hook_integrated_support_event($module, $payload) {

}

