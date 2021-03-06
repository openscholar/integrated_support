<?php 

function github_integration_admin() {
  //get all our labels
  $owner = variable_get('github_repo_owner');
  $repo = variable_get('github_repo_repository');
  $labels = array();

  if ($owner && $repo) {
    $client = github_integration_client(FALSE);
    $response = $client->api('repo')->labels()->all($owner, $repo);
    $labels = ($response) ? array_map(create_function('$a', 'return $a["name"];'), $response) : array();
  }

  $form = array(
    'github_auth_token' => array(
      '#type' => 'textfield',
      '#title' => t('Authorization token'),
      '#description' => t('Token allows access to a github account.  To create one, log into Github, click Account Settings, then Applications, and click Create new Token.'),
      '#required' => TRUE,
      '#default_value' => variable_get('github_auth_token'),
    ),
    'github_repo_owner' => array(
      '#type' => 'textfield',
      '#title' => t('Repository owner'),
      '#description' => t('https://github.com/<strong>openscholar</strong>/integrated_support'),
      '#required' => TRUE,
      '#default_value' => variable_get('github_repo_owner'),
    ),
    'github_repo_repository' => array(
      '#type' => 'textfield',
      '#title' => t('Repository name'),
      '#description' => t('https://github.com/openscholar/<strong>integrated_support</strong>'),
      '#required' => TRUE,
      '#default_value' => variable_get('github_repo_repository'),
    ),
    'github_secret' => array(
      '#type' => 'textfield',
      '#title' => t('Secret'),
      '#description' => t('Sign Github hooks with a shared secret.  The default one was automatically created at random for your convenience.'),
      '#required' => TRUE,
      '#default_value' => variable_get('github_secret', drupal_hash_base64(drupal_random_bytes(256))),
    ),
    'github_cache_dir' => array(
      '#type' => 'textfield',
      '#title' => t('Cache'),
      '#description' => t('Path to use for caching github queries (default: /tmp).'),
      '#required' => TRUE,
      '#default_value' => variable_get('github_cache_dir', '/tmp'),
    ),
    'github_alert' => array(
      '#type' => 'fieldset',
      '#collapsed' => TRUE,
      '#collapsible' => TRUE,
      '#title' => t('Alert Status page'),
      'github_alert_labels' => array(
        '#type' => 'checkboxes',
        '#title' => t('Alert Labels'),
        '#description' => t('Github tickets that have all these labels will show up on /alerts.  Selecting none will disable /alerts.'),
        '#default_value' => variable_get('github_alert_labels', array()),
        '#options' => array_combine($labels, $labels),
      ),
    )
  );

  return system_settings_form($form);
}


/**
 * @function github_integration_milestone_refresh()
 *
 * Github milestone changes aren't pushed automatically.  This does a pull to refresh our
 * stored milestones and does notifications if needed.
 */
function github_integration_milestone_refresh() {
  $count = 0;
  //getsatisfaction
  if (module_exists('getsatisfaction_integration') && db_field_exists('getsatisfaction_service_map', 'milestone')) {

    $results = db_select('getsatisfaction_service_map', 'gs')
      ->condition('external_service', 'github_integration')
      ->fields('gs')
      ->execute();
  
    while ($row = $results->fetchObject()) {
      $issue = github_integration_issue($row->external_id);
      $done = _getsatisfaction_integration_update_topic_milestone($row->id, $issue['milestone'], $row->external_id);
      $count += (int) $done;
    }
  }
  
  //desk
  if (module_exists('desk_integration')) {
    $desk = desk_integration_client();
    
    $page = 1;
    while ($page) {
      $search = $desk->api('case')->call('search', array('page' => $page, 'case_custom_github_issue_id' => '>0'));
      
      foreach ($search->_embedded->entries as $case) {
        $issue = github_integration_issue($case->custom_fields->github_issue_id); //lets hope this really is cached...
        list(,,,,$id) = explode('/', $case->_links->self->href);

        if (($case->custom_fields->github_milestone != $issue['milestone']) && ($case->status != 'closed')) {
          $desk->api('case')->call('update', array('id' => $id, 'custom_fields' => array('github_milestone' => $issue['milestone'])));
          $count++;
        }
      }

      $page = (isset($replies->_links->self->next)) ? $page + 1 : FALSE;
    }
  }
  
  return t('Updated @count items.', array('@count' => $count));
}
