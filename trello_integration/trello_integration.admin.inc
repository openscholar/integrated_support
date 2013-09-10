<?php 

/**
 * @function trello_integration_admin
 * 
 * Trello's admin form
 */
function trello_integration_admin() {
  $key = variable_get('trello_integration_key');
  $secret = variable_get('trello_integration_secret');
  $token = variable_get('trello_integration_token');
  
  //authorization part of the form
  $form = array(
    'auth' => array(
      '#type' => 'fieldset',
      '#title' => 'Trello Authorization',
      '#collapsible' => TRUE,
      '#description' => t('Get your Application Key and Secret from !url, then generate and save an Authorization Token to use Trello.', 
        array('!url' => l('https://trello.com/1/appKey/generate', 'https://trello.com/1/appKey/generate'))
      ),
      
      'trello_integration_key' => array(
        '#type' => 'textfield',
        '#title' => t('Trello Application Key'),
        '#description' => t('Get yours at https://trello.com/1/appKey/generate'),
        '#required' => TRUE,
        '#default_value' => $key,
      ),
      'trello_integration_secret' => array(
        '#type' => 'textfield',
        '#title' => t('Trello Application Secret'),
        '#description' => t('Get yours at https://trello.com/1/appKey/generate'),
        '#required' => TRUE,
        '#default_value' => $secret,
      ),
    ),
  );
  
  if ($key && $secret) {
    $form['auth']['trello_integration_token'] = array(
      '#type' => 'textfield',
      '#title' => t('Trello Authorization Token'),
      '#description' => t('Generate yours !here', array('!here' => l('here', "https://trello.com/1/authorize?key=${key}&name=OSTrello&expiration=never&response_type=token&scope=read,write"))),
      '#required' => TRUE,
      '#default_value' => $token,   
    );
  } else {
    $form['trello_integration_token'] = array(
      '#markup' => '<div class="warning messages">Once you save your key and secret, you will have to generate an access token as well.</div>',
    );
  }
  
  //when auth is valid, lets collapse that part
  if ($token) {
    $form['auth']['#title'] = 'Trello Authorized';
    $form['auth']['#collapsed'] = TRUE;  

    $form['trello_integration_intake'] = array(
      '#type' => 'textfield',
      '#title' => t('Intake Board'),
      '#description' => t('URL of the trello board to use for importing cards from external services.'),
      '#default_value' => variable_get('trello_integration_intake'),
    );
    
    $form['trello_integration_roadmap'] = array(
      '#type' => 'textfield',
      '#title' => t('Roadmap Board'),
      '#description' => t('URL of the trello board to use as a roadmap.'),
      '#default_value' => variable_get('trello_integration_roadmap'),
    );
    
    $form['trello_integration_push_labels'] = array(
      '#type' => 'checkbox',
      '#title' => t('Push labels now.'),
      '#default_value' => FALSE,
      '#description' => t('Trello\'s API only allows me to set a label by color, not by title.  Selecting this checkbox and clicking save will push the labels defined by the `trello_integration_colors` variable so they can be applied correctly.'),
    );
    
    $form['#submit'][] = 'trello_integration_admin_submit';
  }
    
  return system_settings_form($form);
}

function trello_integration_admin_submit($form, &$state, $a=NULL) {
  if ($state['values']['trello_integration_push_labels']) {
    $trello = trello_integration_client();
    $roadmap = variable_get('trello_integration_roadmap');
    list(,,,,$board) = explode('/', $roadmap);
    
    foreach (variable_get('trello_integration_colors') as $label => $color) {
      $trello->boards($board)->labelNames($color)->update($label);
      drupal_set_message(t('Set @color to @label', array('@color' => $color, '@label' => $label)));
    }
  }
}