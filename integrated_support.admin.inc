<?php 

function integrated_support_admin_form($form, &$state) {
	$form = array(
		'integrated_support' => array(
			'#type' => 'fieldset',
		  '#title' => t('Service status'),
		  '#collapsible' => FALSE,
		  'services' => array(
  			'#markup' => integrated_support_status(),
      ), 
		),
	  'create_webhooks' => array(
      '#type' => 'checkboxes',
      '#title' => t('(Re) Deploy remote webhooks'),
	    '#description' => t('These webbhooks can be configured remotely.  Check those that you wish to enable.')   
	   ) ,
	  'integrated_support_force_cache' => array(
	    '#type' => 'textfield',
	    '#title' => t('Force API caching'),
	    '#description' => t('Integrated Support makes frequent API calls.  This setting will force those results to be cached for a number of seconds, regardless of what the API service\'s cache control headers say.'),
	    '#default_value' => variable_get('integrated_support_force_cache', FALSE),
    )
	);
	
	$info = module_invoke_all('integrated_support_info');
	foreach ($info as $name => $def) {
	  if (is_callable($def['setup function'])) {
  		$form['create_webhooks']['#options'][$name] = $def['name'] . ' - ' . $def['description'];
	  }
	}
	
	//list of user mappings between services
	$modules = module_implements('integrated_support_info');
	$user_rows = array();
	$usermap = variable_get('integrated_support_usernames', array());
	foreach ($usermap as $accounts) {
	  $row = array();
	  foreach ($modules as $m) {
	    $row[] = (isset($accounts[$m])) ? $accounts[$m] : NULL;
	  }
	  $user_rows[] = $row;
	}
	
//	variable_set('integrated_support_usernames', array(array('desk_integration' => 'jsagotsky', 'github_integration' => 'sagotsky')));
	$table = array(
	  'header' => $modules,
	  'rows' => $user_rows,
  );
	
	$form['usermap'] = array(
	  '#prefix' => '<label>User name mappings between services.</label>  Configure via "integrated_support_usernames" variable.',
	  '#markup' => theme('table', $table),
  );
	
	$form['submit'] = array(
	  '#type' => 'submit',
	  '#value' => t('Enable'),    
  );

	return $form;
}

/**
 * Enable the web hooks  from admin form
 */
function integrated_support_admin_form_submit($form, &$state) {
  $info = module_invoke_all('integrated_support_info');
  $events = array_filter($state['values']['create_webhooks']);
  foreach ($events as $event) {
    if ($func = $info[$event]['setup function']) {
      $func();
    }
  }
  
  if (isset($state['values']['integrated_support_force_cache'])) {
    variable_set('integrated_support_force_cache', $state['values']['integrated_support_force_cache']);
  }
}

