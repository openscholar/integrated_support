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
