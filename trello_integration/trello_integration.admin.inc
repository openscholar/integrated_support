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
    
    $form['trello_integration_roadmap'] = array(
      '#type' => 'textfield',
      '#title' => t('Roadmap Board'),
      '#description' => t('URL of the trello board to use as a roadmap.'),
      '#default_value' => variable_get('trello_integration_roadmap'),
    );
    
    $form['trello_integration_intake'] = array(
      '#type' => 'textfield',
      '#title' => t('Intake Board'),
      '#description' => t('URL of the trello board to use for importing cards from external services.'),
      '#default_value' => variable_get('trello_integration_intake'),
    );
  }
    
  return system_settings_form($form);
}