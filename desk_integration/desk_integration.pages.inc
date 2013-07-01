<?php

/**
 * @function desk_integration_mass_message_form
 * 
 * Form for messaging all desk users that meet certain criteria
 */
function desk_integration_mass_message_form() {
  $form['destination'] = array(
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#title' => t('Reply to all Desk tickets that'),
    'milestone' => array(
      '#type' => 'textfield',
      '#title' => t('Milestone'),    
    ),
    'state' => array(
      '#type' => 'select',
      '#options' => array('Open' => 'Open', 'Closed' => 'Closed'),
      '#default_value' => 'Closed',
      '#title' => t('State'),
    ),     
  );
  
  //override milestones with known milestones from github client
  if (is_callable('github_integration_client')) {
    $gh = github_integration_client();
    $get_milestones = array('state' => 'open', 'order' => 'asc');
    $response = $gh->api('issue')->milestones()->all(variable_get('github_repo_owner'), variable_get('github_repo_repository'), $get_milestones);
    $milestones = array_map(create_function('$a', 'return $a["title"];'), $response);
    
    $form['destination']['milestone'] = array(
      '#type' => 'select',
      '#options' => array_combine($milestones, $milestones),
      '#title' => t('Milestone'),
      '#default_value' => current($milestones),    
    );
  }
  
  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),  
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Mass Message',
  );
  
  $form['#submit'][] = 'desk_integration_mass_message_form_submit';
  return $form;
}

function desk_integration_mass_message_form_submit($form, $state) {
  if (($milestone = $state['values']['milestone']) && ($status = $state['values']['state']) && strlen($state['values']['message'])) {
    $desk = desk_integration_client();
    $cases = $desk->api('case')->call('search', array('case_custom_github_status' => $status, 'case_custom_github_milestone' => $milestone));
    if ($cases->total_entries) {
      foreach($cases->_embedded->entries as $entry) {
        $id = @array_pop(explode("/", $entry->_links->self->href));
        $sent = $desk->api('case')->call('create_reply', array('id' => $id, 'direction' => 'out', 'body' => $state['values']['message']));
        $message[(isset($sent->errors)) ? 'errors' : 'success'][] = (isset($sent->errors)) ? "#$id $sent->message" : $id;
      }
    }
  }

  drupal_set_message(t('@num messages sent', array('@num' => count($message['success']))));
  if ($message['errors']) {
    drupal_set_message(t('@num messages failed: @msg', array('@num' => count($message['errors']), '@msg' => implode(", \n",$message['errors']))), 'error');
  }
}