<?php 

/**
 * @function getsatisfaction_integration_admin()
 *
 * Admin page for getsatisfaction integration service
 */
function getsatisfaction_integration_admin() {	
  $form = array(
    'getsatisfaction_company' => array(
      '#type' => 'textfield',
      '#title' => t('GetSatisfaction Company'),
      '#description' => t('http://getsatisfaction.com/<strong>company</strong>'),
      '#required' => TRUE,
      '#default_value' => variable_get('getsatisfaction_company'),
    ),

    'getsatisfaction_tickets' => array(
      '#theme' => 'link',
      '#text' => 'Sync GetSatisfaction cases with other services',
      '#path' => 'admin/config/integrated_support/getsatisfaction/tickets/',
      '#options' => array('attributes' => array(), 'html' => NULL),
    ),

    'getsatisfaction_auth' => array(
      '#type' => 'fieldset',
      '#title' => t('getsatisfaction Authorization'),
      '#description' => t('Please supply your GetSatisfaction username and password.  @TODO Replace this with oauth.'),
      'getsatisfaction_username' => array(
        '#type' => 'textfield',
        '#title' => t('Username'),
        '#required' => TRUE,
        '#default_value' => variable_get('getsatisfaction_username'),
      ),
      'getsatisfaction_password' => array(
        '#type' => 'textfield',
        '#title' => t('Password'),
        '#required' => TRUE,
        '#default_value' => variable_get('getsatisfaction_password'),
      ),
    ),
  );

  if (module_exists('github_integration')) {
    $form += array(
      'getsatisfaction_integration_cron' => array(
        '#type' => 'checkbox',
        '#title' => 'Create github issues from pending topics',
        '#default_value' => variable_get('getsatisfaction_integration_cron', FALSE),
        '#description' => t('When a topic is marked pending, if it isn\'t linked to a github issue, this cron job will create an issue and link it automatically.'),
      ),
    );
  }

  return system_settings_form($form);
}

/**
 * @function getsatisfaction_integration_admin_tickets
 *
 * Admin page for mapping GS tickets to other services
 */
function getsatisfaction_integration_admin_tickets() {
  drupal_add_library('system', 'drupal.ajax');

  //get current rows for table
  $results = db_select('getsatisfaction_service_map', 'gs')
    ->fields('gs')
    ->execute();

  $table_rows = array();

  while ($row = $results->fetchAssoc()) {
    $tr = array();
    $path = "admin/config/integrated_support/getsatisfaction/tickets/delete/nojs/$row[id]/$row[external_service]/$row[external_id]";
    $link = l('Del', $path, array('attributes' => array('class' => array('use-ajax'))));

    array_unshift($row, $link);
    $tr['data'] = $row;
    $tr['class'] = array("$row[id]-$row[external_service]-$row[external_id]");
    $table_rows[] = $tr;
  }


  $modules = array_diff(module_implements('integrated_support_info'), array('getsatisfaction_integration'));
  $form = array(
    'service_mappings' => ($table_rows) ? array(
      '#theme' => 'table',
      '#header' => array(t('Delete'), t('ID'), t('Topic'), t('Mapped to'), t('Ext. ID'), t('Ext. title')),
      '#rows' => $table_rows,
    ) : array(),

    'add_map' => array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#title' => t('Sync tickets'),
      'getsatisfaction_url' => array(
        '#title' => t('Link to GetSatisfaction Topic'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#maxlength' => 1024,
      ),
      'external_service' => array(
        '#title' => t('Other Service'),
        '#type' => 'select',
        '#options' => array_combine($modules, $modules),
        '#required' => TRUE,
      ),
      'external_id' => array(
        '#title' => t('Ticket ID'),
        '#description' => t('2849 is the id of this github ticket: https://github.com/openscholar/openscholar/issues/<strong>2849</strong>'),
        '#type' => 'textfield',
        '#required' => TRUE,
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => 'Sync Tickets',
      ),
    ),

    '#submit' => array('getsatisfaction_integration_admin_tickets_submit'),
    '#validate' => array('getsatisfaction_integration_admin_tickets_validate'),
  );

  return $form;
}

/**
 * @function getsatisfaction_integration_admin_tickets_validate
 *
 * Validation for linking a GS case to another service
 */
function getsatisfaction_integration_admin_tickets_validate(&$form, &$state) {
  //GetSatisfaction
  if (!filter_var($state['values']['getsatisfaction_url'], FILTER_VALIDATE_URL)) {
    form_set_error('getsatisfaction_url', t('Must be a URL'));
  } else {
    $slug = @end(explode('/', $state['values']['getsatisfaction_url']));
    $slug = @current(explode('?', $slug));
    $gs_client = getsatisfaction_integration_client();
    $me = $gs_client->api('people')->call('search', array('email' => variable_get('getsatisfaction_username')));
    //$response = $gs_client->api('topic')->call('topic', array('id' => $slug));
    $topic = getsatisfaction_integration_topic($slug);

    if (!$topic) {
      //Could not connect
      form_set_error('getsatisfaction_url', t('Can\'t retrieve topic.  Is this URL correct?'));
    } elseif (FALSE && $topic->author->id != $me->id) {
      //Post isn't owned by this account.  Can only modify accounts we own.
      form_set_error('getsatisfaction_url', t('Cannot modify topic.  Please create a topic as "@user" and merge this one into it, then try again with new URL.', array('@user' => $me->canonical_name)));
    } else {
      //It's good!  Collect data.
      $state['values']['id'] = $topic->id;
      $state['values']['title'] = $topic->subject;
    }
  }

  //Other Service
  if ($item = _getsatisfaction_integration_item_by_id($state['values']['external_service'], $state['values']['external_id'])) {
    $state['values']['external_item'] = $item;
  } else {
    form_set_error('add_map', t('Could not find specified ticket in service.'));
  }
}

/**
 * @function getsatisfaction_integration_admin_tickets_submit
 *
 * Saves link between GS case and another system's ticket
 */
function getsatisfaction_integration_admin_tickets_submit(&$form, &$state) {
  $record = array(
    'id' => $state['values']['id'],
    'title' => $state['values']['title'],
    'external_service' => $state['values']['external_service'],
    'external_id' => $state['values']['external_item']['id'],
    'external_title' => $state['values']['external_item']['title'],
  );

  drupal_write_record('getsatisfaction_service_map', $record);

  //trigger an update now.  no sense in waiting.
  if ($record['external_service'] == 'github_integration') {
    getsatisfaction_integration_integrated_support_event('github_issue', $state['values']['external_item']);
  }
}

/**
 * @function getsatisfaction_integration_admin_tickets_delete
 *
 * Page callback removes a getsatisfactio service map
 */
function getsatisfaction_integration_admin_tickets_delete($ajax, $gs_id, $service, $service_id) {
  $deleted = db_delete('getsatisfaction_service_map')
    ->condition('id', $gs_id)
    ->condition('external_service', $service)
    ->condition('external_id', $service_id)
    ->execute();

  if ($ajax === 'ajax') {
    $commands[] = ajax_command_remove(".$gs_id-$service-$service_id");
    ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
    exit;
  } else {
    return ($deleted) ? t('Deleted mapping') : t('Could not delete mapping');
  }
}

/**
 * @function _getsatisfaction_integration_id_title
 *
 * Returns the title of an item on one of our services.
 */
function _getsatisfaction_integration_item_by_id($service, $id) {
  try {
    switch ($service) {
      case 'github_integration':
        return github_integration_issue($id);

      default:
        return 'N/A';
    }
  } catch (Exception $e) {
    return False;
  }

}
