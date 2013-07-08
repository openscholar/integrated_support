<?php 

function desk_integration_admin() {
  $form = array(
    'desk_subdomain' => array(
      '#type' => 'textfield',
      '#title' => t('Desk Subdomain'),
      '#description' => t('http://<strong>subdomain</strong>.desk.com'),
      '#required' => TRUE,
      '#default_value' => variable_get('desk_subdomain'),
    ),
    'desk_auth' => array(
      '#type' => 'fieldset',
      '#title' => t('Desk Authorization'),
      '#description' => t('Log into your Desk.com.  In the admin panel, click settings, then API.  Click Add API application. Copy the Key and Secret, then click Your Access Token to get the Token and Token Secret.'),
      'desk_consumer_key' => array(
        '#type' => 'textfield',
        '#title' => t('Key'),
        '#description' => t(''),
        '#required' => TRUE,
        '#default_value' => variable_get('desk_consumer_key'),
      ),
      'desk_consumer_secret' => array(
        '#type' => 'textfield',
        '#title' => t('Secret'),
        '#description' => t(''),
        '#required' => TRUE,
        '#default_value' => variable_get('desk_consumer_secret'),
      ),
      'desk_auth_token' => array(
        '#type' => 'textfield',
        '#title' => t('Token'),
        '#description' => t(''),
        '#required' => TRUE,
        '#default_value' => variable_get('desk_auth_token'),
      ),
      'desk_auth_secret' => array(
        '#type' => 'textfield',
        '#title' => t('Token secret'),
        '#description' => t(''),
        '#required' => TRUE,
        '#default_value' => variable_get('desk_auth_secret'),
      ),
    ),

    'desk_notifications' => array(
      '#type' => 'fieldset',
      '#title' => t('Notifications'),

      'desk_notify_milestone' => array(
        '#type' => 'Checkbox',
        '#title' => t('Milestone'),
        '#description' => t('Notify users when their cases\' milestone changes.'),
        '#default_value' => variable_get('desk_notify_milestone', TRUE),
      ),

      'desk_notify_closed' => array(
        '#type' => 'Checkbox',
        '#title' => t('Case Closed'),
        '#description' => t('Notify users when their cases are closed.'),
        '#default_value' => variable_get('desk_notify_closed', TRUE),
      ),
    )
  );


  return system_settings_form($form);
}
