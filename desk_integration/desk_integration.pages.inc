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

function desk_integration_mass_message_form_submit($form, &$state) {
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


/**
 * @function desk_integration_service_rating
 *
 * Collects ratings from links sent out with desk cases.
 * Should verify that the included timestamp match's desk's timestamp.
 */
function desk_integration_service_rating($form, &$state, $case_id, $hidden_token, $rating) {
  //hidden token is custoemr id.
  $ratings = array('Excellent', 'Good', 'Fair', 'Poor'); //variable?

  //comment is provided, skip ahead and append to that to the db row we already have.
  if (isset($_REQUEST['comments'])) {
    _desk_integration_service_rating_submit($case_id, $rating, $_REQUEST['comments']);
    return array('message' => array('#type' => 'markup', '#markup' => t('Thank you for your comments, you may close this page.')));
  }

  if (!in_array($rating, $ratings)) {
    return array('error' => array('#type' => 'markup', '#markup' => t('Invalid rating.  Please use one of @ratings', array('@rating' => implode(', ', $ratings)))));
  }

  //bail out if this has already been rated.
  $rated = db_select('desk_service_reviews')
    ->condition('id', $case_id)
    ->fields('desk_service_reviews', array('id'))
    ->range(0,1)
    ->execute();

  if ($rated->rowCount()) {
    //drupal_set_message(t('This case has already been rated.'), 'error');
    return array('error' => array('#type' => 'markup', '#markup' => t('This case has already been rated.')));
  }

  //get desk's closed date.  compare that with what was sent.
  $desk = desk_integration_client();
  $case = $desk->api('case')->call('show', array('id' => $case_id));

  if (!$case || @$case->message == 'Resource Not Found') {
    return array('error' => array('#type' => 'markup', '#markup' => t('This case does not exist.')));
  }

  list (,,,,$customer) = explode('/', $case->_links->customer->href);
  if ($hidden_token != $customer) { //closed at doesn't really exist.  use updated_at.
    return array('error' => array('#type' => 'markup', '#markup' => t('Could not authenticate source of request.')));
  }

  $record = array('id' => $case_id, 'rating' => $rating);
  drupal_write_record('desk_service_reviews', $record);

  $form = array(
    'comments' => array(
      '#type' => 'textarea',
      '#title' => t('Further Comments'),
      '#prefix' => '<p>Thank you for your feedback!  If you have any additional comments, feel free to leave them here.</p>',
    ),
    'case_id' => array(
      '#type' => 'hidden',
      '#value' => $case_id,
    ),
    'rating' => array(
      '#type' => 'hidden',
      '#value' => $rating,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Comment'),
    ),
    '#submit' => array('desk_integration_service_rating_submit'),
  );

  return $form;
}

/**
 * @function _desk_integration_service_rating_comment
 *
 * Adds a comment to a review.
 */
function _desk_integration_service_rating_submit($id, $rating, $comments) {
  $updated = db_update('desk_service_reviews')
    ->condition('id', $id)
    ->condition('rating', $rating)
    ->isNull('comments')
    ->fields(array('comments' => check_plain($comments)))
    ->execute();
}

/**
 * @function desk_integration_bug_report
 * 
 * Bug report form.  Sends a bug to both GS and GH, and links them.
 */
function desk_integration_bug_report($form, &$state, $case_id, $case_user, $case_agent) {
  //this functionality relies on all three integrations being enabled.
  if (!module_exists('github_integration') || !module_exists('getsatisfaction_integration')) {
    return array();
  }
  
  //check that id is a case, and it has right user and agents.
  if (($case = _desk_integration_bug_report_validate_case($case_id, $case_user, $case_agent)) == FALSE) {
    return array();
  }

  //get github options
  $client = github_integration_client();
  $none = array(-1 =>  '-- Select --');
  
  $ms_order = array('state' => 'open', 'sort' => 'due_date', 'direction' => 'asc');
  $milestones = $none + array_map(create_function('$a', 'return $a["title"];'), github_integration_milestones($ms_order));
  
  $users = $client->api('repo')->collaborators()->all(variable_get('github_repo_owner'), variable_get('github_repo_repository'));
  $users = ($users) ? $none + array_map(create_function('$a', 'return $a["login"];'), $users) : array();
  
  $labels = $client->api('repo')->labels()->all(variable_get('github_repo_owner'), variable_get('github_repo_repository'));
  $labels = ($labels) ? array_map(create_function('$a', 'return $a["name"];'), $labels) : array();
  
  //build the form
  $form = array(
    'title' => array(
      '#title' => t('Title'),
      '#description' => t('Ticket title.'),
      '#type' => 'textfield',
      '#default_value' => $case->subject,
      '#required' => TRUE,
    ),
      
    'body' => array(
      '#title' => t('Body'),
      '#description' => t('Main text to be sent to Github and GetSatisfaction.  Now is your chance to filter out any private data.'),
      '#type' => 'textarea',
      '#rows' => 15,
      '#default_value' => $case->message->body,
      '#required' => TRUE,
    ),
        
    'destination' => array(
      '#title' => t('Destination'),
      '#description' => t('To which services should this ticket be sent?  Selecting both services will also create a link between them.'),
      '#type' => 'checkboxes',
      '#options' => array('github_integration' => t('Github'), 'getsatisfaction_integration' => t('GetSatisfaction')),
      '#default_value' => array('github_integration', 'getsatisfaction_integration'),   
    ),
      
    'github' => array(
      '#type' => 'fieldset',
      '#title' => t('Github'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'milestone' => array(
        '#title' => t('Github Milestone'),
        '#type' => 'select',
        '#options' => drupal_map_assoc($milestones),    
      ),
      'assignee' => array(
        '#title' => t('Github Assignee'),
        '#type' => 'select',
        '#options' => drupal_map_assoc($users),
      ),
      'labels' => array(
        '#title' => t('Github Labels'),
        '#type' => 'checkboxes',
        '#options' => drupal_map_assoc($labels),
        '#default_value' => array('desk'),
      ),
    ),
    
    'getsatisfaction' => array(
      '#type' => 'fieldset',
      '#title' => t('GetSatisfaction'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,  
      'style' => array(
        '#title' => t('Style'),
        '#type' => 'select',
        '#options' => drupal_map_assoc(array('question', 'problem' ,'praise', 'idea', 'update')),
        '#default_option' => 'problem',
      ),
    ),
    
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Create Tickets'),
      '#submit' => array('desk_integration_bug_report_submit'),
    )    
  );
  
  return $form;
}

/**
 * @function desk_integration_bug_report_submit
 * 
 * Submit function for desk_integration_bug_report.  Creates github and getsatisfaction
 * tickets, links them.
 */
function desk_integration_bug_report_submit($form, &$state) {
  $values = $state['values'];
  list($case_id, $customer, $user) = $state['build_info']['args'];
  $issue = $topic = NULL;
  
  //send to github
  if ($values['destination']['github_integration']) {
    $issue = array(
      'title' => $values['title'],
      'body' => preg_replace('/^/', '> ', $values['body']), //blockquote the entire body
      'assignee' => ($values['assignee'] != '-- Select --') ? $values['assignee'] : NULL,
      'labels' => implode(',', array_filter($values['labels'])),
      'milestone' => ($values['milestone'] != '-- Select --') ? $values['milestone'] : NULL, 
    );
    
    $issue = github_create_issue(array_filter($issue));
    if ($issue) {
      desk_integration_update_cases($case_id, $issue['number'], $issue['milestone']['title']);
    }
  }
  
  //send to getsatisfaction
  if ($values['destination']['getsatisfaction_integration']) {
    $gs_client = getsatisfaction_integration_client();
    $topic = array(
      'subject' => $values['title'],
      'additional_detail' => $values['body'],
      'style' => $values['style'],
    );
    $topic = $gs_client->api('topic')->call('create', $topic);
  }
  
  //gs <-> gh linkage
  if (isset($issue, $issue['number'], $topic, $topic->at_sfn)) {
    $state = array('values' => array(
      'external_service' => 'github_integration',
      'external_id' => $issue['number'],
      'getsatisfaction_url' => $topic->at_sfn,
    ));
    
    require_once(drupal_get_path('module', 'getsatisfaction_integration') . '/getsatisfaction_integration.admin.inc');
    drupal_form_submit('getsatisfaction_integration_admin_tickets', $state);
  }
}

/**
 * @function _desk_integration_bug_report_validate_case()
 * 
 * Validates that a case's customer and agent match the user and agent ids we were given.  Returns case object if they do.
 */
function _desk_integration_bug_report_validate_case($case_id, $case_customer, $case_assigned_user) {
  $desk = desk_integration_client();
  $case = $desk->api('case')->call('show', array('id' => $case_id));
  list(,,,,$customer) = explode('/', $case->_links->customer->href);
  list(,,,,$assigned_user) = explode('/', $case->_links->assigned_user->href);
  
  if (!$case) {
    drupal_set_message(t('Error loading case "@id"', array('@id' => $case_id)), 'error');
    return FALSE;
  }
  
  if ($assigned_user != $case_assigned_user || $customer != $case_customer) {
    drupal_set_message(t('Error authenticating case "@id"', array('@id' => $case_id)), 'error');
    return FALSE;
  }
  
  if ($case->custom_fields->github_issue_id) {
    drupal_set_message(t('This case already has a github issue assigned to it.'), 'error');
    return FALSE;
  }

  $case->message = $desk->api('case')->call('message', array('id' => $case_id)); 
  return $case;
}